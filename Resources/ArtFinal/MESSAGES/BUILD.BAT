echo off
echo rebuilding MESSAGES.FL
justlst2 *.txt | sort >messages.fl
echo rebuilding MESSAGES.RES
call res messages
echo Done.
