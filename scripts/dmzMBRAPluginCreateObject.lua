function receive (self, type, data)

   if dmz.data.is_a (data) then

      local pos = data:lookup_vector ("position", 1)

      local undoHandle = dmz.undo.start_record ("Create Node")
      local handle = dmz.object.create ("na_node")

      if not handle then self.log:error ("Object Not Created!")
      else
         dmz.object.position (handle, nil, pos)
         dmz.object.activate (handle)
         local outData = dmz.data.new ()
         outData:store_handle ("object", 1, handle)
         self.editMessage:send_message ("dmzMBRAPluginNodeProperties", outData)
      end
      dmz.undo.stop_record (undoHandle)
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:lookup_message_type ("message.name", "CreateObjectMessage"),
      editMessage =
         config:lookup_message_type ("edit.name", "EditObjectAttributesMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.log:info ("Message type: " .. self.message:get_name ())
   self.obs:register (self.message, receive, self)

   return self
end
