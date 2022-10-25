@ECHO OFF

SETLOCAL

CALL setVersion.bat

CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"
devenv.com all.sln /Build "Debug|Win32"
IF ERRORLEVEL 1 GOTO END

cd bin\Win32\Debug

IF "%USE_EXTERNAL_SIGNING%" == "Yes" (
    CALL ..\..\..\signModulesByServer256Only.bat
) ELSE (
    CALL ..\..\..\signModules.bat
)

IF ERRORLEVEL 1 GOTO END

:END
ENDLOCAL
