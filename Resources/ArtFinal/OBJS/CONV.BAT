cd %1
pkunzip2 sources\sources.zip
convbbmo *.bbm
conpal *.pic
call qr
cpc *.pic
pkzip2 sources\pics.zip *.pic
del *.pic
del *.bbm
call obj %1
cd ..
