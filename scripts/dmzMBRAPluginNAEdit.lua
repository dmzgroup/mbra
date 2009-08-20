function receive (self, type, data)
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if dmz.object.is_object (handle) then 
         self.message:send ("NetworkAnalysisNodeProperties", data)
      elseif dmz.object.is_link (handle) then
         self.message:send ("NetworkAnalysisLinkProperties", data)
      end
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("edit.name", "EditObjectAttributesMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.obs:register (self.message, receive, self)

   return self
end