REM @ECHO OFF

SETLOCAL

del /F /Q build.log
cd build
CALL cleanall.bat

REM
REM Release build
REM
ECHO:  
ECHO Building Win32 Release .....
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"
devenv.com all.sln /Build "Release|Win32" >> ..\build.log
IF ERRORLEVEL 1 GOTO :ERROR
ECHO: 
ECHO Building x64 Release .....
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"
devenv.com all.sln /Build "Release|x64" >> ..\build.log
IF ERRORLEVEL 1 GOTO :ERROR

REM
REM Debug build
REM
ECHO:
ECHO Building Win32 Debug .....
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"
devenv.com all.sln /Build "Debug|Win32" >> ..\build.log
IF ERRORLEVEL 1 GOTO :ERROR
ECHO: 
ECHO Building x64 Debug .....
CALL "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\VsDevCmd.bat"
devenv.com all.sln /Build "Debug|x64" >> ..\build.log
IF ERRORLEVEL 1 GOTO :ERROR

echo ***************************************************************
echo *                  Build Succeed!!                            *
echo ***************************************************************
GOTO :END

:ERROR
echo ***************************************************************
echo *                  Build Failed!!!                            *
echo *         check build.log for detail                          *
echo ***************************************************************

:END
cd ..
ENDLOCAL