require "StateMachine";

smmain = StateMachine:create();

smmain.state_funcs["init"] = function(self, event)
	if (event == "enter") then
		print "smmain: Entered init state";
		self.state = "idle";
	elseif (event == "exit") then
		print "smmain: Exited init state";
	elseif (event == "update") then
		print "smmain: Updated init";
	end
end

return smmain
