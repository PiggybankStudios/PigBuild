@echo off

REM In pig_build examples we need to set PIG_BUILD_ROOT to a full path (not relative)
REM so that the real shell script inside pig_build/shell can know where to find pig_build
REM In your own build.bat for your project you can omit all this and the
REM pig_build/shell/build.bat will assume pig_build just exists inside a folder called "pig_build"

REM %~dp0 gives us the directory part of the path to this batch script
set PIG_BUILD_ROOT=%~dp0\..\..

call %PIG_BUILD_ROOT%\shell\build.bat %*