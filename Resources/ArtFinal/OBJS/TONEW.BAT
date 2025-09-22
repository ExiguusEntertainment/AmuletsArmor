@echo off
cd %1
rem copy %1.inf %1.ifb
rem copy %1.oaf %1.ooa
rem disoaf2 %1.oaf "%1.inf"
objinf %1.inf
call res %1
rem copy %1.ooa %1.oaf
rem pause
cd ..
