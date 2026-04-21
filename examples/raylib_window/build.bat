@echo off
set PIG_BUILD_ROOT=%~dp0\..\..
REM TODO: Link to curl? Or WinHTTP?
call %PIG_BUILD_ROOT%\shell\build.bat %*