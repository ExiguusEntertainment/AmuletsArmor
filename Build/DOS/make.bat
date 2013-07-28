@call owsetenv.bat
@if "%1"=="incremental" goto compile
@if "%1"=="clean" goto clean
@if "%1"=="all" goto all

:all
@echo "Make -- All"
@call clean.bat
@goto compile

:compile
@echo "Make -- Compile"
@del testme.exe >NUL 2>NUL
@wmake
@copy testme.exe c:\lshields\aa\aa\exe\.
@goto end

:clean
@echo "Make -- Clean"
@call clean.bat
@del testme.exe >NUL 2>NUL
@goto end

:end
@echo "Make -- Done"
