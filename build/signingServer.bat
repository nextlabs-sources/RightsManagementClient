@ECHO OFF

SETLOCAL

set SIGNING_SHARE_DIR=C:\signing
set SIGNING_REQ_FILENAME=signingReq.txt
set SIGNING_DONE_FILENAME=signingDone.txt

set SIGNMODULES_BAT=%CD%\signModules.bat

:AGAIN

FOR /R "%SIGNING_SHARE_DIR%" %%P IN (.) DO (
    IF EXIST "%%P\%SIGNING_REQ_FILENAME%" (
        ECHO Signing files in "%%P"
        del "%%P\%SIGNING_REQ_FILENAME%"
        pushd "%%P"
        CALL "%SIGNMODULES_BAT%"
        popd
        copy nul "%%P\%SIGNING_DONE_FILENAME%"
    )
)

IF EXIST C:\cygwin\bin\sleep.exe (
    C:\cygwin\bin\sleep.exe 10
) ELSE IF EXIST C:\cygwin64\bin\sleep.exe (
    C:\cygwin64\bin\sleep.exe 10
) ELSE IF EXIST .\Sleep.exe (
    .\Sleep.exe 10000
)

GOTO AGAIN

ENDLOCAL
