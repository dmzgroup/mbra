
function receive (self, type, data)
   if dmz.data.is_a (data) then
      local undoHandle = dmz.undo.start_record ("Unlink Nodes")
      local handle = data:lookup_handle ("object", 1)
      if handle and dmz.object.is_link (handle) then dmz.object.unlink (handle)
      elseif not handle then self.log:error ("Got null handle!")
      end
      dmz.undo.stop_record (undoHandle)
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:lookup_message_type ("message.name", "UnlinkObjectsMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.log:info ("Message type: " .. self.message:get_name ())
   self.obs:register (self.message, receive, self)

   return self
end
