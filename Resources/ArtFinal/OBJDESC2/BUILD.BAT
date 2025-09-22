echo off
echo rebuilding OBJDESC2.FL
justlst2 *.txt | sort >objdesc2.fl
echo rebuilding OBJDESC2.RES
call res objdesc2
echo Done.
