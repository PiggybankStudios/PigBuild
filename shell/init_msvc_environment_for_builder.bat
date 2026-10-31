@echo off

rem The goal of this script is to initialize the environment to prepare for calling the
rem Microsoft compiler that was installed with Visual Studio to allow us to compile
rem the build_script.c into a builder.exe on Windows. Usually the recommended approach
rem from Microsoft is to run a particular command prompt flavor that was installed and
rem can be found in the Start menu as things like "Developer Command Prompt for VS 2022"
rem or "x64 Native Tools Command Prompt for VS 2022" which do this environment initialization
rem for you, removing the ambiguity of where to find the compiler. The goal of Pig Build
rem is to make it as easy as possible to build a program so we want to support running
rem the build.bat in a regular un-configured Command Prompt window. To do this we have
rem to guess where your Visual Studio compiler is installed and run it's VsDevCmd.bat
rem (formerly vcvarsall.bat) and we have to pass the correct arguments to configure it
rem to compile for the host architecture (x86_64 or ARM64). If you already are in a configured
rem environment, this script will do nothing and we will assume it was configured for the
rem correct target architecture. If you don't have Visual Studio installed (or we can't find it)
rem then this script will throw an error with some info on what might be wrong and exit

REM By default blocks like IF and FOR in batch are treated as a single command and variable values are expended when the block is entered
REM Enabling "delayed expansion" allows us to use !variable! syntax inside blocks to get the value of variables on that line (and at each iteration in the case of loops)
setlocal enabledelayedexpansion

rem NOTE: This script does not handle vcvars32.bat or similar names that came before vcvarsall.bat
rem       This means the script will not work for versions before VS 2005

rem First let's check if we think the environment is already configured
if defined INCLUDE if defined LIB (
	where cl >NUL 2>&1
	if "!ERRORLEVEL!"=="0" goto :msvc_already_configured
)

call :get_time "%TIME%" START_TIME

rem +==============================+
rem | determine_host_architecture  |
rem +==============================+
:determine_host_architecture

set "HOST_ARCHITECTURE="
rem The registry value is not affected by WOW64 or x64-on-ARM64 so we check that first and only fallback if it's not set
for /f "tokens=3" %%A in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v PROCESSOR_ARCHITECTURE 2^>nul ^| find "PROCESSOR_ARCHITECTURE"') do set "HOST_ARCHITECTURE=%%A"
rem Fallback to PROCESSOR_ARCHITEW6432 or PROCESSOR_ARCHITECTURE if the registry value is not set
if not defined HOST_ARCHITECTURE if defined PROCESSOR_ARCHITEW6432 set "HOST_ARCHITECTURE=%PROCESSOR_ARCHITEW6432%"
if not defined HOST_ARCHITECTURE set "HOST_ARCHITECTURE=%PROCESSOR_ARCHITECTURE%"

set "VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS="
set "VCVARSALL_ARCHITECTURE_ARG="
set "VSWHERE_COMPONENT_ARG="
if /i "%HOST_ARCHITECTURE%"=="AMD64" (
	set "VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS=-arch=x64 -host_arch=x64"
	set "VCVARSALL_ARCHITECTURE_ARG=amd64"
	set "VSWHERE_COMPONENT_ARG=Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
) else if /i "%HOST_ARCHITECTURE%"=="ARM64" (
	set "VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS=-arch=arm64 -host_arch=arm64"
	set "VCVARSALL_ARCHITECTURE_ARG=arm64"
	set "VSWHERE_COMPONENT_ARG=Microsoft.VisualStudio.Component.VC.Tools.ARM64"
) else (
	echo Cannot determine architecture of the host machine through registry entry or PROCESSOR_ARCHITEW6432 or PROCESSOR_ARCHITECTURE. Please update init_msvc_environment_for_builder.bat to handle your environment
	exit /b 1
)
rem echo VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS: %VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS%
rem echo VCVARSALL_ARCHITECTURE_ARG: %VCVARSALL_ARCHITECTURE_ARG%
rem echo VSWHERE_COMPONENT_ARG: %VSWHERE_COMPONENT_ARG%

rem +==================================+
rem | find_vs_install_dir_with_vswhere |
rem +==================================+
:find_vs_install_dir_with_vswhere

set "VS_INSTALL_DIR="
set "VCVARSALL_PATH="
set "VSDEVCMD_PATH="

rem Next let's try and find the Visual Studio install directory using vswhere.exe (works on VS installations after VS 2017 15.2)
set "VSWHERE_PATH=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE_PATH%" (
	for /f "usebackq delims=" %%I in (`"%VSWHERE_PATH%" -latest -products * -requires %VSWHERE_COMPONENT_ARG% -property installationPath`) do set "VS_INSTALL_DIR=%%I"
	
	if not defined VS_INSTALL_DIR (
		echo WARNING: vswhere.exe reports no VS install with component %VSWHERE_COMPONENT_ARG%
		goto :guess_vs_install_dir
	) else if not exist "!VS_INSTALL_DIR!" (
		echo WARNING: vswhere.exe pointed us to a directory that doesn't exist: "!VS_INSTALL_DIR!"
		goto :guess_vs_install_dir
	) else (
		goto :guess_vsdevcmd_or_vcvarsall_path
	)
) else (
	goto :guess_vs_install_dir
)

rem +==================================+
rem | guess_vsdevcmd_or_vcvarsall_path |
rem +==================================+
:guess_vsdevcmd_or_vcvarsall_path
if exist "%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat" (
	set "VSDEVCMD_PATH=%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat"
	goto :run_vsdevcmd
) else if exist "%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat" (
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat"
	goto :run_vcvarsall
) else if exist "%VS_INSTALL_DIR%\VC\vcvarsall.bat" (
	rem NOTE: We probably will never hit this branch, because I think this relative location only exists for VS version before vswhere.exe was a thing
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
) else (
	echo WARNING: vswhere.exe tells us to look into "%VS_INSTALL_DIR%" but we could not find a VsDevCmd.bat nor a vcvarsall.bat in that directory. Please update init_msvc_environment_for_builder.bat to look at the correct location. See the "find_vs_install_dir_with_vswhere" section
	goto :guess_vs_install_dir
)

rem +==============================+
rem |     guess_vs_install_dir     |
rem +==============================+
:guess_vs_install_dir

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
	set "VS_INSTALL_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community"
	set "VSDEVCMD_PATH=%VS_INSTALL_DIR%\Common7\Tools\VsDevCmd.bat"
	goto :run_vsdevcmd
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files\Microsoft Visual Studio\2022\Community"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\Auxiliary\Build\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 14.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 12.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 11.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 10.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 9.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio 8.0\VC\vcvarsall.bat" (
	set "VS_INSTALL_DIR=C:\Program Files (x86)\Microsoft Visual Studio 8.0"
	set "VCVARSALL_PATH=%VS_INSTALL_DIR%\VC\vcvarsall.bat"
	goto :run_vcvarsall
)

goto :couldnt_find_installation

rem +==============================+
rem |        run_vcvarsall         |
rem +==============================+
:run_vcvarsall
rem Guard against the batch changing the directory on us
pushd .
echo Initializing MSVC compiler...
rem echo Running "%VCVARSALL_PATH%" %VCVARSALL_ARCHITECTURE_ARG%
call "%VCVARSALL_PATH%" %VCVARSALL_ARCHITECTURE_ARG% > NUL 2>&1
popd
goto :verify_environment

rem +==============================+
rem |         run_vsdevcmd         |
rem +==============================+
:run_vsdevcmd
rem Guard against the batch changing the directory on us
pushd .
echo Initializing MSVC compiler...
rem echo Running "%VSDEVCMD_PATH%" %VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS%
call "%VSDEVCMD_PATH%" %VSDEVCMD_ARCHITECTURE_AND_HOST_ARGS% > NUL 2>&1
popd
goto :verify_environment

rem +==============================+
rem |      verify_environment      |
rem +==============================+
:verify_environment

where cl >nul 2>&1
if "%ERRORLEVEL%"=="0" (
	goto :exit_successfully
) else (
	echo Compiler was still not available in the PATH even after finding a VS Installation with C/C++ compiler and running the configuration script. This is likely a bug in init_msvc_environment_for_builder.bat. Please update the script to handle your environment after running VsDevCmd.bat/vcvarsall.bat
	exit /b 1
)

rem +==============================+
rem |  couldnt_find_installation   |
rem +==============================+
:couldnt_find_installation
echo Couldn't find Visual Studio installation.
echo vswhere.exe is not available (would be if you installed VS 2017 or later) and we couldn't find it at any of the hardcoded locations.
echo If you have VS installed please update the "guess_vs_install_dir" section of init_msvc_environment_for_builder.bat with the location of vcvarsall.bat or VsDevCmd.bat.
echo If you have Visual Studio installed make sure you have installed the C/C++ compiler feature in the VS Installer
echo Alternatively, you can run the build.bat in a "Developer Command Prompt for VS XXXX" to skip the need for us to search for the installed compiler
exit /b 1

rem +==============================+
rem |   msvc_already_configured    |
rem +==============================+
:msvc_already_configured
echo Running in pre-configured MSVC environment
exit /b 0

rem +==============================+
rem |      exit_successfully       |
rem +==============================+
:exit_successfully

call :get_time "%TIME%" END_TIME
call :format_time_diff "%START_TIME%" "%END_TIME%" ELAPSED_TIME_STR
echo MSVC compiler Initialized (%ELAPSED_TIME_STR%)

rem set "COMPILER_PATH="
rem set "LINKER_PATH="
rem for /f "usebackq delims=" %%I in (`where cl`) do set "COMPILER_PATH=%%I"
rem for /f "usebackq delims=" %%I in (`where link`) do set "LINKER_PATH=%%I"
rem echo Using compiler at "%COMPILER_PATH%"
rem echo Using linker at "%LINKER_PATH%"

exit /b 0

rem +==============================+
rem |           get_time           |
rem +==============================+
rem call :get_time "%TIME%" OUTPUT_VAR_NAME
:get_time
for /f "tokens=1-4 delims=:., " %%a in ("%~1") do set /a "%~2=((100%%a %% 100)*3600 + (100%%b %% 100)*60 + (100%%c %% 100))*100 + (100%%d %% 100)"
exit /b 0

rem +==============================+
rem |       format_time_diff       |
rem +==============================+
rem call :format_time_diff "%START_TIME%" "%END_TIME%" OUTPUT_VAR_NAME
:format_time_diff
set /a "ELAPSED_TIME_INT=%~2-%~1"
if %ELAPSED_TIME_INT% lss 0 set /a "ELAPSED_TIME_INT+=8640000"
set /a "ELAPSED_TIME_SEC_WHOLE=ELAPSED_TIME_INT/100, ELAPSED_TIME_SEC_DECIMAL=100+ELAPSED_TIME_INT%%100"
set "%~3=%ELAPSED_TIME_SEC_WHOLE%.%ELAPSED_TIME_SEC_DECIMAL:~1%s"
exit /b 0
