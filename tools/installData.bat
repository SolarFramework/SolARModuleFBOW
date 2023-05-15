@echo off
:: Download fbow data 
echo Download fbow database for vocabulary generation 
curl https://repository.solarframework.org/generic/data-solar/datafbowLoopDesktopA.zip -L -o datafbowLoopDesktopA.zip
echo Unzip fbow data 
powershell Expand-Archive datafbowLoopDesktopA.zip -DestinationPath .\data -F
del datafbowLoopDesktopA.zip
