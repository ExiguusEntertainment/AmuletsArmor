echo off
echo describing items
describe *.txt >describe.out
echo rebuilding INVDESC.FL
justlst2 *. | sort >invdesc.fl
echo rebuilding INVDESC.RES
call res invdesc
echo Done.
