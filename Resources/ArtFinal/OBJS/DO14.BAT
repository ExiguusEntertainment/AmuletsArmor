cd %1
pkunzip2 sources\pics.zip
call c14 *.pic
pkzip2 sources\pics.zip *.pic
cpc *.pic
del *.pic
call obj %1
cd ..
