@ECHO OFF

SETLOCAL

IF "%BUILD_NUMBER%"=="" (
  echo Error: BUILD_NUMBER is not set!
  GOTO :EOF
)

REM
REM Check line-end in .INF files.  Make sure that they have Windows line-end.
REM Otherwise, Visual Studio will fail to add timestamps to the .inf files
REM without any error messages.
REM 
echo Checking nxrmdrv.inf ...
build\lineendchecker\Win32\Release\lineendchecker.exe /win nxrmdrv\src\nxrmdrv\nxrmdrv.inf
IF ERRORLEVEL 1 GOTO :EOF
echo Checking nxrmflt.inf ...
build\lineendchecker\Win32\Release\lineendchecker.exe /win nxrmflt\src\nxrmflt\nxrmflt.inf
IF ERRORLEVEL 1 GOTO :EOF
echo Checking nxrmvhd.inf ...
build\lineendchecker\Win32\Release\lineendchecker.exe /win nxrmvhd\drv\src\nxrmvhd.inf
IF ERRORLEVEL 1 GOTO :EOF

REM
REM Release build
REM
cd build
CALL buildRelease32.bat
IF ERRORLEVEL 1 GOTO :EOF
CALL buildRelease64.bat
IF ERRORLEVEL 1 GOTO :EOF
cd ..\install
CALL buildInstallersByConfig Release
IF ERRORLEVEL 1 GOTO :EOF
CALL publishInstallersByConfig Release
IF ERRORLEVEL 1 GOTO :EOF
cd ..

REM
REM Debug build
REM
cd build
CALL buildDebug32.bat
IF ERRORLEVEL 1 GOTO :EOF
CALL buildDebug64.bat
IF ERRORLEVEL 1 GOTO :EOF
cd ..\install
CALL buildInstallersByConfig Debug
IF ERRORLEVEL 1 GOTO :EOF
CALL publishInstallersByConfig Debug
IF ERRORLEVEL 1 GOTO :EOF
cd ..

SET RES_PUBLISH_TO_BODA=false
if "%PUBLISH_TO_BODA%" == "Yes" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "yes" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "YES" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "1" set RES_PUBLISH_TO_BODA=true
if "%RES_PUBLISH_TO_BODA%"=="true" (
REM
REM Artifacts
REM
cd install
CALL publishArtifacts.bat
)