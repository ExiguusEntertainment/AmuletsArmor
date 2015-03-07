local StateMachine = {}
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
    if ((self.laststate ~= self.state) or (self.forceNewState ~= nil)) then
	    local state = self.state;
		local last_f = self.laststate;
	    if (last_f) then last_f(self, "exit") end;
	    if (f) then f(self, "enter") end;
		self.laststate = state;
		self.forceNewState = nil;
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
	if (type(flag) == "table") then
		for i=1,#flag do
			self.flags[flag[i]] = true;
		end
	else
		self.flags[flag] = true;
	end
end

function StateMachine:clear(flag)
	if (type(flag) == "table") then
		for i=1,#flag do
			self.flags[flag[i]] = false;
		end
	else
		self.flags[flag] = false;
	end
end

function StateMachine:is(flag)
	return self.flags[flag];
end

function StateMachine:check(flag, newState)
	if (self:is(flag)) then self.state = newState; self.forceNewState = 1; end;
end

return StateMachine
