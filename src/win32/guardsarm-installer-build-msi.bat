SETLOCAL
SET PATH=%PATH%;C:\Program Files\Microsoft SDKs\Windows\v7.0\Bin
SET PATH=%PATH%;C:\Program Files (x86)\WiX Toolset v3.11\bin

set VERSION=%1
set REVISION=%2

REM IF VERSION or REVISION are empty, ask for their value
IF [%VERSION%] == [] set /p VERSION=Enter the version of the GuardSarm agent (x.y.z):
IF [%REVISION%] == [] set /p REVISION=Enter the revision of the GuardSarm agent:

SET MSI_NAME=guardsarm-agent-%VERSION%-%REVISION%.msi

candle.exe -nologo "guardsarm-installer.wxs" -out "guardsarm-installer.wixobj" -ext WixUtilExtension -ext WixUiExtension
light.exe "guardsarm-installer.wixobj" -out "%MSI_NAME%"  -ext WixUtilExtension -ext WixUiExtension

signtool sign /a /tr http://timestamp.digicert.com /fd SHA256 /d "%MSI_NAME%" /td SHA256 "%MSI_NAME%"

pause
