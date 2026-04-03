@echo off

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

if not exist build mkdir build
pushd build

if not exist builder.exe (
	CALL :BuildAndRun
) else if not exist builder_hash.txt (
	CALL :BuildAndRun
) else (
	builder.exe %*
	if "!ERRORLEVEL!"=="42" (
		echo Recompile requested
		CALL :BuildAndRun
	)
)

popd

EXIT /B 0
:BuildAndRun

CALL ..\pig_build\shell\init_msvc.bat msvc_environment.txt
echo Compiling build_script.c...
del builder.* builder_hash.txt build_script.obj > NUL 2> NUL
cl ..\build_script.c /Fe"builder.exe" /Fd"builder.pdb" /std:clatest /INCREMENTAL:NO /Od /FC /nologo /Zi /I".." /I"..\pig_build" /link Shlwapi.lib"
if "%ERRORLEVEL%"=="0" builder.exe %*

EXIT /B
