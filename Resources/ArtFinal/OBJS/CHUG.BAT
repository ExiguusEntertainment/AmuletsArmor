cd %1
copy /y a:%1.bbm .
convbbmo %1.bbm
qrotate %1.pic
cpc %1.pic
copy ..\sword\sword.inf %1.inf
echo %1.cpc      00000  >%1.fl
echo %1.oaf      Info   >>%1.fl
echo ; >>%1.fl
call obj %1
pause
cd ..
