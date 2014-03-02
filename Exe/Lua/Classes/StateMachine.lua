StateMachine = {}
StateMachine_mt = { __index = StateMachine};

StateMachine.create = function()
	local new_instance = { 
		state = "init", 
		laststate = "", 
		flags = {}, 
		state_funcs = {},
	}
	setmetatable( new_instance, StateMachine_mt );
	return new_instance;
end

function StateMachine:update()
	print("SMMAIN: State is now " .. self.state .. " and was state " .. self.laststate .. ".");
    local change = 0;
		local f = self.state_funcs[self.state];
    if (self.laststate ~= self.state) then
	    local state = self.state;
		local last_f = self.state_funcs[self.laststate];
	    if (last_f) then last_f(self, "exit") end;
	    if (f) then f(self, "enter") end;
		print("  NOW -- self: State is now " .. state .. " and was state " .. self.laststate .. ".");
		self.laststate = state;
	end
	if (f) then f(self, "update") end;
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

