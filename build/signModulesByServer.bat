REM
REM This script should only be run as the sole user that has acccess to
REM \\%SIGNING_SERVER%\%SIGNING_SHARE%.  Currently this is NEXTLABS\build-op.
REM

SETLOCAL

set SIGNING_SHARE=signing
set SIGNING_REQ_FILENAME=signingReq.txt
set SIGNING_DONE_FILENAME=signingDone.txt



REM
REM Signing server info.
REM

set SIGNING_SERVER=BORDUY



REM
REM Copy the files to a unique directory on the server.
REM

net use "\\%SIGNING_SERVER%\%SIGNING_SHARE%" /persistent:no
set SIGNING_PATH=\\%SIGNING_SERVER%\%SIGNING_SHARE%\%COMPUTERNAME%_%RANDOM%
md "%SIGNING_PATH%"
xcopy /k /i /y *.sys "%SIGNING_PATH%"
xcopy /k /i /y *.cat "%SIGNING_PATH%"
xcopy /k /i /y *.dll "%SIGNING_PATH%"
xcopy /k /i /y *.exe "%SIGNING_PATH%"



REM
REM Request the server to start signing by creating the file
REM %SIGNING_REQ_FILENAME%.
REM

copy nul "%SIGNING_PATH%\%SIGNING_REQ_FILENAME%"



REM
REM Wait for the server to finish signing by waiting for the presence of the
REM file %SIGNING_DONE_FILENAME%.
REM

:WAIT

IF NOT EXIST "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" (
    ECHO Waiting for "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" ......

    IF EXIST C:\cygwin\bin\sleep.exe (
	C:\cygwin\bin\sleep.exe 10
    ) ELSE IF EXIST C:\cygwin64\bin\sleep.exe (
	C:\cygwin64\bin\sleep.exe 10
    )

    GOTO WAIT
)

ECHO "%SIGNING_PATH%\%SIGNING_DONE_FILENAME%" detected.



REM
REM Move the files back to our directory.
REM

xcopy /k /i /y "%SIGNING_PATH%\*.sys"
xcopy /k /i /y "%SIGNING_PATH%\*.cat"
xcopy /k /i /y "%SIGNING_PATH%\*.dll"
xcopy /k /i /y "%SIGNING_PATH%\*.exe"

rd /s /q "%SIGNING_PATH%"

net use "\\%SIGNING_SERVER%\%SIGNING_SHARE%" /del



:END
ENDLOCAL
