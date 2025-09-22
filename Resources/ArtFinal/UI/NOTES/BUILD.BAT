echo off
echo rebuilding NOTES.FL
justlst2 *. | sort >notes.fl
echo rebuilding NOTES.RES
call res NOTES
echo Done.
