@Echo off
echo Connecting to the build server...
remote\psexec \\nulldev.stream.aol.com -u remotebuild -c -f .\Remote\winamp_remote.cmd %1 %2 %3
echo %errorlevel%
if errorlevel 1236 goto LOGIN_ERROR

@echo off
if %1==BETA SET BUILDTYPE=beta
if %1==FINAL SET BUILDTYPE=final
if %1==NIGHT SET BUILDTYPE=nightly
explorer "http://nulldev.stream.aol.com/builds/default.asp?path=%BUILDTYPE%"
if errorlevel 1 goto BUILD_ERROR
goto END


:LOGIN_ERROR
pause
goto END

:BUILD_ERROR
pause
goto END

:END
exit