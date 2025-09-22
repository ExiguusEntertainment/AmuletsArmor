echo off
echo rebuilding SPELDESC.FL
justlst2 *. | sort >speldesc.fl
echo rebuilding SPELDESC.RES
call res speldesc
echo Done.
