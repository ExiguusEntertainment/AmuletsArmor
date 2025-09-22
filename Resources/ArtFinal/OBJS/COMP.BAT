cd %1
mkdir sources
cd sources
pkzip2 sources.zip ..\*.bbm
pkzip2 pics.zip ..\*.pic
cd ..
pause
del *.bbm
del *.pic
dir /w
dir sources /w
cd ..

