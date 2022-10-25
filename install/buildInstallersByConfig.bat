@ECHO ON

SETLOCAL

IF "%1"=="" GOTO USAGE

IF "%1"=="Debug" (
  set CONFIG=Debug
  set ISM_FILENAME_SUFFIX=_Debug
) ELSE IF "%1"=="Release" (
  set CONFIG=Release
  set ISM_FILENAME_SUFFIX=
) ELSE (
  GOTO USAGE
)

CALL ..\build\setVersion.bat

IF "%BUILD_NUMBER%"=="" (
  set buildNumber=0
) ELSE (
  set buildNumber=%BUILD_NUMBER%
)

set version="%VERSION_MAJMIN%.%buildNumber%"

CALL c:\windows\syswow64\cscript ISAutoGUIDVersion.js 2014RightsManagementClient_x86%ISM_FILENAME_SUFFIX%.ism %version%
CALL c:\windows\syswow64\cscript ISAutoGUIDVersion.js 2014RightsManagementClient_x64%ISM_FILENAME_SUFFIX%.ism %version%

xcopy /s /k /i /y ..\build\bin\Win32\%CONFIG%\*.sys build\data\Win32\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\Win32\%CONFIG%\*.inf build\data\Win32\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\Win32\%CONFIG%\*.cat build\data\Win32\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\Win32\%CONFIG%\*.dll build\data\Win32\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\Win32\%CONFIG%\*.exe build\data\Win32\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\x64\%CONFIG%\*.sys build\data\x64\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\x64\%CONFIG%\*.inf build\data\x64\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\x64\%CONFIG%\*.cat build\data\x64\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\x64\%CONFIG%\*.dll build\data\x64\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF
xcopy /s /k /i /y ..\build\bin\x64\%CONFIG%\*.exe build\data\x64\%CONFIG%
IF ERRORLEVEL 1 GOTO :EOF

xcopy /s /y themes "C:\Program Files (x86)\InstallShield\2014 SAB\Support\Themes"
IF ERRORLEVEL 1 GOTO :EOF

CALL 2014build%CONFIG%32Installer.bat
IF ERRORLEVEL 1 GOTO :EOF

CALL 2014build%CONFIG%64Installer.bat
IF ERRORLEVEL 1 GOTO :EOF

GOTO :EOF



:USAGE
echo Usage: %0 config
echo config can be either "Debug" or "Release"
