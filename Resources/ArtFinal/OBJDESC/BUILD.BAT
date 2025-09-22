echo off
echo rebuilding OBJDESC.FL
justlst2 *.txt | sort >objdesc.fl
echo rebuilding OBJDESC.RES
call res objdesc
echo Done.
