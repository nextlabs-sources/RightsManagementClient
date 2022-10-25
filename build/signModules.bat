SETLOCAL

SET TIMESTAMP_SERVER_SHA256=timestamp.digicert.com

IF "%PROCESSOR_ARCHITECTURE%"=="x86" (
    SET SIGNTOOL="C:\Program Files\Windows Kits\8.1\bin\x86\signtool.exe"
) ELSE (
    SET SIGNTOOL="C:\Program Files (x86)\Windows Kits\8.1\bin\x86\signtool.exe"
)
SET SIGNTOOL_ARGS=sign /ac "C:\release\bin\DigiCert High Assurance EV Root CA.crt" /n "NextLabs Inc."
SET SIGNTOOL_ARGS_SHA256=/fd SHA256 /t http://%TIMESTAMP_SERVER_SHA256%



SET /a SIGNCOUNT=1

:SIGNSHA256AGAIN

%SIGNTOOL% %SIGNTOOL_ARGS% %SIGNTOOL_ARGS_SHA256% *.dll *.exe *.sys *.cat
IF %ERRORLEVEL% EQU 0 GOTO AFTERSIGNSHA256

ECHO ERROR: Signing SHA256 attempt #%SIGNCOUNT% failed!
ping www.google.com
ping %TIMESTAMP_SERVER_SHA256%

IF EXIST C:\cygwin\bin\sleep.exe (
    ECHO Sleeping for 10 seconds before retrying ......
    C:\cygwin\bin\sleep.exe 10
) ELSE IF EXIST C:\cygwin64\bin\sleep.exe (
    ECHO Sleeping for 10 seconds before retrying ......
    C:\cygwin64\bin\sleep.exe 10
)

SET /a SIGNCOUNT=%SIGNCOUNT% + 1
IF %SIGNCOUNT% LEQ 10 GOTO SIGNSHA256AGAIN

:AFTERSIGNSHA256



:END
ENDLOCAL
