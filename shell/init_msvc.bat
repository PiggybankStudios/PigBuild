@echo off

REM This file does a few things. We need to run the batch file that sets up the
REM environment variables that are required for running the MSVC C/C++ compiler.

REM This is called VsDevCmd.bat but used to be called vcvarsall.bat, the location
REM of this batch file also depends on which version of Visual Studio you installed.
REM This batch file takes multiple seconds to run so we also try very hard to not
REM run it if we don't need to. To do this we can dump the environment variables to
REM a file after we run the batch file and then rather than running it for the next build
REM we just parse the dumped file and set the environment variables based off that.

REM TODO: Eventually we should try and find the compiler in a "smart" way and we should
REM.      be a little more careful about what we dump to the msvc_environment.txt file

REM Finally we also want to track and report how long the batch file takes so we can
REM separate that time from our actual build time.

if "%~1"=="" (
	echo Usage: init_msvc.bat output_file.txt
	exit /b 1
)

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_start_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)

echo Initializing MSVC compiler...

REM TODO: Report a nice error message if we don't find the compiler/VsDevCmd.bat
REM set VSCMD_DEBUG=3
REM NOTE: Uncomment or change one of these lines to match your installation of Visual Studio compiler
REM call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64
REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64 -no_logo
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 > NUL 2>&1

for /F "tokens=1-4 delims=:.," %%a in ("%time%") do (
	set /A "vsdevcmd_end_time=(((%%a*60)+1%%b %% 100)*60+1%%c %% 100)*100+1%%d %% 100"
)
set /A vsdevcmd_elapsed_hundredths=vsdevcmd_end_time-vsdevcmd_start_time
set /A vsdevcmd_elapsed_seconds_part=vsdevcmd_elapsed_hundredths/100
set /A vsdevcmd_elapsed_hundredths_part=vsdevcmd_elapsed_hundredths%%100
if %vsdevcmd_elapsed_hundredths_part% lss 10 set vsdevcmd_elapsed_hundredths_part=0%vsdevcmd_elapsed_hundredths_part%
echo VsDevCmd.bat took %vsdevcmd_elapsed_seconds_part%.%vsdevcmd_elapsed_hundredths_part% seconds

REM The environment variables we care about are probably some subset of the following:
REM   DevEnvDir, ExtensionSdkDir, EXTERNAL_INCLUDE, FSHARPINSTALLDIR, INCLUDE, LIB, LIBPATH, NETFXSDKDir
REM   Framework40Version, FrameworkDir, FrameworkDir64, FrameworkVersion, FrameworkVersion64, UCRTVersion, UniversalCRTSdkDir
REM   VCIDEInstallDir, VCINSTALLDIR, VCToolsInstallDir, VCToolsRedistDir, VCToolsVersion, VisualStudioVersion
REM   VS170COMNTOOLS, VSCMD_ARG_app_plat, VSCMD_ARG_HOST_ARCH, VSCMD_ARG_TGT_ARCH, VSCMD_VER, VSINSTALLDIR, VSSDK150INSTALL, VSSDKINSTALL
REM   WindowsLibPath, WindowsSdkBinPath, WindowsSdkDir, WindowsSDKLibVersion, WindowsSdkVerBinPath, WindowsSDKVersion, WindowsSDK_ExecutablePath_x64, WindowsSDK_ExecutablePath_x86
REM   __DOTNET_ADD_64BIT, __DOTNET_PREFERRED_BITNESS, __VSCMD_PREINIT_PATH
REM Also a bunch of things added to PATH:
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VC\VCPackages;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TestWindow;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\TeamFoundation\Team Explorer;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\bin\Roslyn;
REM   C:\Program Files (x86)\Microsoft SDKs\Windows\v10.0A\bin\NETFX 4.8 Tools\x64\;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\FSharp\Tools;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Team Tools\DiagnosticsHub\Collector;
REM   C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\\x64;
REM   C:\Program Files (x86)\Windows Kits\10\bin\\x64;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\\MSBuild\Current\Bin\amd64;
REM   C:\Windows\Microsoft.NET\Framework64\v4.0.30319;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;
REM   C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\VC\Linux\bin\ConnectionManagerExe
set > "%~1"