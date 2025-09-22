@echo off
cd %1
copy ..\0\0.inf %1.inf
objinf %1.inf
cd ..
