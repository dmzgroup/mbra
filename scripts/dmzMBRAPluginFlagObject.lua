local NodeType = dmz.object_type.new ("na_node")
local FlagState = dmz.definitions.lookup_state ("NA_Node_Flagged")

function receive (self, type, data)

   if dmz.data.is_a (data) then

      local message = "Flag Node"
      local handle = data:lookup_handle ("object", 1)

      if handle and dmz.object.is_link (handle) then
         message = "Flag Link"
         handle = dmz.object.link_attribute_object (handle)
      end

      if handle and dmz.object.is_object (handle) then
         local UndoHandle = dmz.undo.start_record (message)
         local state = dmz.object.state (handle)
         if not state then state = dmz.mask.new () end
         if state:contains (FlagState) then state:unset (FlagState)
         else state = state + FlagState
         end
         dmz.object.state (handle, nil, state)
         dmz.undo.stop_record (UndoHandle)
      end

   end

end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("message.name", "FlagObjectMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.log:info ("Message type: " .. self.message:get_name ())
   self.obs:register (self.message, receive, self)

   return self
end
