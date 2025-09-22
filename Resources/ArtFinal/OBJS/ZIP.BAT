pkzip2 -Pru e:\artobjs1.zip @list1.zz
pkzip2 -Pru e:\artobjs2.zip @list2.zz
pkzip2 -Pru e:\artobjs3.zip @list3.zz
pkzip2 -Pru e:\artobjs4.zip @list4.zz
pkzip2 -Pru e:\artobjs5.zip @list5.zz
pkzip2 e:\artobjs6.zip *.*
cd ..
pkzip2 -Pru e:\artfinal.zip *.* -xobjs -xobjs\*.*
