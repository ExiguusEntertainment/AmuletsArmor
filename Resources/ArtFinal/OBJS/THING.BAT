echo off
cd %1
echo %1.cpc     00001   >>%1.fl
echo %1.oaf     Info    >>%1.fl
copy ..\thing.inf %1.inf
call obj %1
cd ..
