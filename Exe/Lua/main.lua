require "AAGame/smMain" ;
local G_lastOftenTime = 0

function updateOften()
	local time = ticker.get()
	
	delta = time - G_lastOftenTime
	G_lastOftenTime = time
	sound.update()
	color.update(delta)
end

function sprintf(fmt, ...)
	return string.format(fmt, ...);
end

function printf(fmt, ...)
	print(string.format(fmt, ...));
end

function AABacktrace(errmsg)
	local backtrace = debug.traceback();
	print("A&A Backtrace: " .. errmsg)
	print(backtrace);
	return errmsg;
end

function protected_main()
	print("** MAIN ENTERED **")
	local lastTick = ticker.get();

	while (not smMain:isDone()) do
		local delta = ticker.get() - lastTick;
		if (delta < 1) then
			-- Too fast! Slow down and let the CPU cool off
			ticker.sleep(1)
		end
		-- Okay, proceed
		lastTick = ticker.get()
-- TODO:            UpdateCmdqueue() ;
--            DebugCompare("main") ;
		updateOften()
		smMain:update()
	end
end

function main()
	xpcall(protected_main, AABacktrace)
end

-- Run!
main()