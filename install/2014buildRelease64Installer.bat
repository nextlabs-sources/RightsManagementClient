@ECHO OFF

"C:\Program Files (x86)\InstallShield\2014 SAB\System\IsCmdBld.exe" -x -p 2014RightsManagementClient_x64.ism -r "Media_EXE" -a "Release_64bit"
IF ERRORLEVEL 1 GOTO END
"C:\Program Files (x86)\InstallShield\2014 SAB\System\IsCmdBld.exe" -x -p 2014RightsManagementClient_x64.ism -r "Media_MSI" -a "Release_64bit"
IF ERRORLEVEL 1 GOTO END

:END
