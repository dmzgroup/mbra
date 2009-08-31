local HighlightState = dmz.definitions.lookup_state ("NA_Node_Highlight")

function receive (self, type, data)

   if dmz.data.is_a (data) then

      local handle = data:lookup_handle ("object", 1)

      if handle and dmz.object.is_link (handle) then
         handle = dmz.object.link_attribute_object (handle)
      end

      if handle and dmz.object.is_object (handle) then
         local state = dmz.object.state (handle)
         if not state then state = dmz.mask.new () end
         state = state + HighlightState
         dmz.object.state (handle, nil, state)
      end

      if handle ~= self.handle then
         if self.handle then
            local prev = self.handle
            self.handle = handle
            if dmz.object.is_object (prev) then
               local state = dmz.object.state (prev)
               if state then
                  state:unset (HighlightState)
                  dmz.object.state (prev, nil, state)
               end
            end
         else self.handle = handle
         end
      end

   end
end

local cb = {

update_object_state = function (self, object, attribute, value)
   if value:contains (HighlightState) and self.handle ~= object then
      local state = dmz.object.state (object)
      if state then
         state:unset (HighlightState);
         dmz.object.state (object, nil, state)
      end
   end
end,

}

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("message.name", "MouseMoveEvent"),
      obs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.obs:register (self.message, receive, self)
   self.objObs:register (nil, cb, self)

   return self
end
