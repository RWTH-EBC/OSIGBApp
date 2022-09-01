C:\git\esp-idf\export.ps1
# Write-Host -ForegroundColor Yellow "go to software.c"
# Set-Location C:\git\iotdevelopment\software\software.c
Write-Host -ForegroundColor Yellow "save original sdkconfig"
Rename-Item .\sdkconfig .\sdkconfig.orig

Write-Host -ForegroundColor Yellow "Compile for Digital board"
Copy-Item .\sdkconfig.dio .\sdkconfig
idf.py -B .\dio build 
Remove-Item .\sdkconfig 
# Write-Host -ForegroundColor Yellow "copy .bin from dio-build folder to dio folder"
# Copy-Item C:\git\iotdevelopment\software\software.c\dio\IoTGateway.bin C:\git\iotdevelopment\ota\dio\

Write-Host -ForegroundColor Yellow "Compile for Analog board"
Copy-Item .\sdkconfig.aio .\sdkconfig
idf.py -B .\aio build 
Remove-Item .\sdkconfig 
# Write-Host -ForegroundColor Yellow "copy .bin from aio-build folder to aio folder"
# Copy-Item C:\git\iotdevelopment\software\software.c\aio\IoTGateway.bin C:\git\iotdevelopment\ota\aio\

Write-Host -ForegroundColor Yellow "Compile for Temperature board"
Copy-Item .\sdkconfig.tmp .\sdkconfig
idf.py -B .\tmp build 
Remove-Item .\sdkconfig 
# Write-Host -ForegroundColor Yellow "copy .bin from tmp-build folder to tmp folder"
# Copy-Item C:\git\iotdevelopment\software\software.c\tmp\IoTGateway.bin C:\git\iotdevelopment\ota\tmp\

Rename-Item .\sdkconfig.orig .\sdkconfig
Write-Host -ForegroundColor Green "Finished Compiling"
Write-Host -ForegroundColor Yellow "Flash with idf.py flash -B .\<tmp/aio/dio> build"