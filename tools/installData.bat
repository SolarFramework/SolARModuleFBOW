@echo off
:: Download fbow data 
echo Download fbow database for vocabulary generation 
curl https://repository.solarframework.org/generic/data-solar/datafbow.zip -L -o datafbow.zip
echo Unzip fbow data 
powershell Expand-Archive datafbow.zip -DestinationPath .\data -F
del datafbow.zip
