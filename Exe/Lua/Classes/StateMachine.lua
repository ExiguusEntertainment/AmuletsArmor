StateMachine = {}
StateMachine_mt = { __index = StateMachine};

StateMachine.create = function()
	local new_instance = { 
		state = nil, 
		laststate = nil, 
		flags = {}, 
	}
	setmetatable( new_instance, StateMachine_mt );
	return new_instance;
end

function StateMachine:update()
    local change = 0;
	local f = self.state;
	print(f)
    if (self.laststate ~= self.state) then
	    local state = self.state;
		local last_f = self.laststate;
	    if (last_f) then last_f(self, "exit") end;
	    if (f) then f(self, "enter") end;
		self.laststate = state;
	else
		-- Check for any state changes
		if (f) then f(self, "check") end;
		
		-- If the state did not change, then go ahead and update
		if (self.laststate == self.state) then
			if (f) then f(self, "update") end;
		end
	end
end

function StateMachine:set(flag)
	self.flags[flag] = true;
end

function StateMachine:clear(flag)
	self.flags[flag] = false;
end

function StateMachine:is(flag)
	return self.flags[flag];
end

function StateMachine:check(flag, newState)
	if (self:is(flag)) then self.state = newState end;
end

