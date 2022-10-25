@ECHO OFF

SETLOCAL

CALL ..\build\setVersion.bat

IF "%BUILD_NUMBER%"=="" (
  echo Error: BUILD_NUMBER is not set!
  GOTO :EOF
)

SET S_DRIVE=\\nextlabs.com\share\data
SET S_DRIVE_POSIX=//nextlabs.com/share/data



IF NOT EXIST %S_DRIVE%\build\release_candidate\artifacts\Kismet\%VERSION_MAJMIN% md %S_DRIVE%\build\release_candidate\artifacts\Kismet\%VERSION_MAJMIN%
IF ERRORLEVEL 1 GOTO END
md %S_DRIVE%\build\release_candidate\artifacts\Kismet\%VERSION_MAJMIN%\%BUILD_NUMBER%
IF ERRORLEVEL 1 GOTO END

cd ..\build
zip -D -r -b . -9 %S_DRIVE_POSIX%/build/release_candidate/artifacts/Kismet/%VERSION_MAJMIN%/%BUILD_NUMBER%/kismet-%VERSION_MAJMIN%.%BUILD_NUMBER%-release-bin.zip bin
IF ERRORLEVEL 1 GOTO END
echo INFO: Created %S_DRIVE_POSIX%/build/release_candidate/artifacts/Kismet/%VERSION_MAJMIN%/%BUILD_NUMBER%/kismet-%VERSION_MAJMIN%.%BUILD_NUMBER%-release-bin.zip

cd ..
zip -D -r -b . -9 %S_DRIVE_POSIX%/build/release_candidate/artifacts/Kismet/%VERSION_MAJMIN%/%BUILD_NUMBER%/kismet-%VERSION_MAJMIN%.%BUILD_NUMBER%-release-install.zip install
IF ERRORLEVEL 1 GOTO END
echo INFO: Created %S_DRIVE_POSIX%/build/release_candidate/artifacts/Kismet/%VERSION_MAJMIN%/%BUILD_NUMBER%/kismet-%VERSION_MAJMIN%.%BUILD_NUMBER%-release-install.zip
