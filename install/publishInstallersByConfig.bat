@ECHO ON

SETLOCAL

IF "%1"=="" GOTO USAGE

IF "%1"=="Debug" (
  set CONFIG=Debug
  set CFG_FILENAME_SUFFIX=_Debug
  set ZIP_FILENAME_SUFFIX=-debug
) ELSE IF "%1"=="Release" (
  set CONFIG=Release
  set CFG_FILENAME_SUFFIX=
  set ZIP_FILENAME_SUFFIX=
) ELSE (
  GOTO USAGE
)

CALL ..\build\setVersion.bat

IF "%BUILD_NUMBER%"=="" (
  echo Error: BUILD_NUMBER is not set!
  GOTO :EOF
)

SET S_DRIVE=\\nextlabs.com\share\data
SET S_DRIVE_POSIX=//nextlabs.com/share/data



REM
REM Copy installers
REM

md temp_%CONFIG%\%CONFIG%_32bit
IF ERRORLEVEL 1 GOTO :EOF
copy build\output_2014\%CONFIG%_32bit\Media_EXE\DiskImages\DISK1\setup.exe temp_%CONFIG%\%CONFIG%_32bit
IF ERRORLEVEL 1 GOTO :EOF
copy build\output_2014\%CONFIG%_32bit\Media_EXE\DiskImages\DISK1\Setup.ini temp_%CONFIG%\%CONFIG%_32bit
IF ERRORLEVEL 1 GOTO :EOF
copy "build\output_2014\%CONFIG%_32bit\Media_MSI\DiskImages\DISK1\NextLabs Rights Management.msi" temp_%CONFIG%\%CONFIG%_32bit
IF ERRORLEVEL 1 GOTO :EOF

md temp_%CONFIG%\%CONFIG%_64bit
IF ERRORLEVEL 1 GOTO :EOF
copy build\output_2014\%CONFIG%_64bit\Media_EXE\DiskImages\DISK1\setup.exe temp_%CONFIG%\%CONFIG%_64bit
IF ERRORLEVEL 1 GOTO :EOF
copy build\output_2014\%CONFIG%_64bit\Media_EXE\DiskImages\DISK1\Setup.ini temp_%CONFIG%\%CONFIG%_64bit
IF ERRORLEVEL 1 GOTO :EOF
copy "build\output_2014\%CONFIG%_64bit\Media_MSI\DiskImages\DISK1\NextLabs Rights Management.msi" temp_%CONFIG%\%CONFIG%_64bit
IF ERRORLEVEL 1 GOTO :EOF



REM
REM Copy tools with installers
REM

copy ..\build\bin\Win32\%CONFIG%\ClientPackageBuilder.exe temp_%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
copy ClientPackageBuilder_2014%CFG_FILENAME_SUFFIX%.cfg temp_%CONFIG%\ClientPackageBuilder.cfg
IF ERRORLEVEL 1 GOTO :EOF



REM
REM Publish ZIP files for instalers and tools
REM
SET RES_PUBLISH_TO_BODA=false
if "%PUBLISH_TO_BODA%" == "Yes" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "yes" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "YES" set RES_PUBLISH_TO_BODA=true
if "%PUBLISH_TO_BODA%" == "1" set RES_PUBLISH_TO_BODA=true
if "%RES_PUBLISH_TO_BODA%" == "true" (
IF NOT EXIST %S_DRIVE%\build\release_candidate\RightsManagementClient\%VERSION_MAJMIN% md %S_DRIVE%\build\release_candidate\RightsManagementClient\%VERSION_MAJMIN%
IF ERRORLEVEL 1 GOTO :EOF
IF NOT EXIST %S_DRIVE%\build\release_candidate\RightsManagementClient\%VERSION_MAJMIN%\%BUILD_NUMBER% md %S_DRIVE%\build\release_candidate\RightsManagementClient\%VERSION_MAJMIN%\%BUILD_NUMBER%
IF ERRORLEVEL 1 GOTO :EOF
cd temp_%CONFIG%
zip -D -r -b . -9 %S_DRIVE_POSIX%/build/release_candidate/RightsManagementClient/%VERSION_MAJMIN%/%BUILD_NUMBER%/NextLabsRightsManagement-%VERSION_MAJMIN%.%BUILD_NUMBER%%ZIP_FILENAME_SUFFIX%.zip .
IF ERRORLEVEL 1 GOTO :EOF
echo INFO: Created %S_DRIVE_POSIX%/build/release_candidate/RightsManagementClient/%VERSION_MAJMIN%/%BUILD_NUMBER%/NextLabsRightsManagement-%VERSION_MAJMIN%.%BUILD_NUMBER%%ZIP_FILENAME_SUFFIX%.zip
cd ..
)
rd /s /q temp_%CONFIG%



echo ********** Finished publishing %CONFIG% installers **********
GOTO :EOF



:USAGE
echo Usage: %0 config
echo config can be either "Debug" or "Release"
