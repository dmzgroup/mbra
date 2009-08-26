local HighlightState = dmz.definitions.lookup_state ("NA_Node_Highlight")

function receive (self, type, data)

   if dmz.data.is_a (data) then

      local handle = data:lookup_handle ("object", 1)

      if handle then
         if dmz.object.is_object (handle) then
            local state = dmz.object.state (handle)
            if not state then state = dmz.mask.new () end
            state = state + HighlightState
            dmz.object.state (handle, nil, state)
         elseif dmz.object.is_link (handle) then
         end
      end

      if handle ~= self.handle then
         if self.handle then
            if dmz.object.is_object (self.handle) then
               local state = dmz.object.state (self.handle)
               if state then
                  state:unset (HighlightState)
                  dmz.object.state (self.handle, nil, state)
               end
            end
         end
         self.handle = handle
      end

   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("message.name", "MouseMoveEvent"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.obs:register (self.message, receive, self)

   return self
end
