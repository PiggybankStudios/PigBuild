@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

REM Find this script's directory, %0 contains the path to the script, strip it to just directory part
REM NOTE: We have to do this before we pushd into the "build" folder
set "BUILD_BAT_FOLDER=%~dp0"
REM Remove trailing backslash
if "%BUILD_BAT_FOLDER:~-1%"=="\" set "BUILD_BAT_FOLDER=%BUILD_BAT_FOLDER:~0,-1%"

REM If someone runs this script directly from the pig_build/shell/ folder,
REM assume they want to build the project two directories up.
if /I "%CD%"=="%BUILD_BAT_FOLDER%" (
	echo build.bat was run directly from pig_build\shell\ folder. Moving up to build project in two folders above...
	cd ..\..
)

REM We take the current working directory as the project that we want to build
set "PROJECT_ROOT=%CD%"
if not exist "%PROJECT_ROOT%\build_script.c" (
	echo Couldn't find build_script.c in the current working directory. pig_build requires that the build.bat is run from the project's root directory and that a build_script.c exists in that root folder
	exit /B 1
)

if not exist build mkdir build
pushd build

REM Default PIG_BUILD_ROOT to this script's parent directory
REM (i.e. assume this script lives at <pig_build>/shell/build.bat)
IF NOT DEFINED PIG_BUILD_ROOT (
	for %%I in ("%BUILD_BAT_FOLDER%\..") do set "PIG_BUILD_ROOT=%%~fI"
)

REM We need to escape backslashes in the PIG_BUILD_ROOT path in order to set the
REM PIG_BUILD_ROOT define to a C string literal with backslashes
set "PIG_BUILD_ROOT_ESCAPED=%PIG_BUILD_ROOT:\=\\%"

REM These are the flags for building the build_script.c. Flags for the target program are decided inside build_script.c
set "compiler_flags=/Fe"builder.exe""
set "compiler_flags=%compiler_flags% /std:clatest"
set "compiler_flags=%compiler_flags% /INCREMENTAL:NO /nologo /FC"
set "compiler_flags=%compiler_flags% /Fd"builder.pdb" /Od /Zi"
set "compiler_flags=%compiler_flags% /I"%PROJECT_ROOT%" /I"%PIG_BUILD_ROOT%\src""
set "compiler_flags=%compiler_flags% "/DPIG_BUILD_ROOT=\"%PIG_BUILD_ROOT_ESCAPED%\"""
set "compiler_flags=%compiler_flags% %PIG_BUILD_FLAGS%"


if not exist builder.exe (
	CALL :CompileBuilder
	if "!ERRORLEVEL!" NEQ "0" goto :Done
) else if not exist builder_hash.txt (
	CALL :CompileBuilder
	if "!ERRORLEVEL!" NEQ "0" goto :Done
)

REM Run the builder.exe and save the exit code so we can handle if it requests a rebuild
.\builder.exe %*
set "builder_exit_code=!ERRORLEVEL!"

if "!ERRORLEVEL!"=="42" (
	echo [Recompile requested]
	CALL :CompileBuilder
	if "!ERRORLEVEL!" NEQ "0" goto :Done
	.\builder.exe %*
)

:Done
popd
EXIT /B 0

REM +--------------------------------------------------------------+
REM |                        CompileBuilder                        |
REM +--------------------------------------------------------------+
:CompileBuilder

CALL %PIG_BUILD_ROOT%\shell\init_msvc.bat msvc_environment.txt
echo [Compiling build_script.c...]
del builder.* builder_hash.txt build_script.obj > NUL 2> NUL
cl %PROJECT_ROOT%\build_script.c %compiler_flags%
EXIT /B
