copy /y wand%1.bbm ..\wand%1\.
cd ..
cd wand%1
convbbm
qrotate wand%1.pic
call obj wand%1
cd ..
cd wands
