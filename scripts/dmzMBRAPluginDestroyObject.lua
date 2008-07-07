local NodeType = dmz.object_type.new ("na_node")

function receive (self, type, data)
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if handle then 
         local objType = dmz.object.type (handle)
         if
               objType and
               objType:is_of_type (NodeType) and
               dmz.object.is_object (handle) then
            local undoHandle = dmz.undo.start_record ("Delete Node")
            dmz.object.destroy (handle)
            dmz.undo.stop_record (undoHandle)
         else if dmz.object.is_link (handle) then
            local undoHandle = dmz.undo.start_record ("Unlink Nodes")
            dmz.object.unlink (handle)
            dmz.undo.stop_record (undoHandle)
         end
      else self.log:error ("Got null handle!")
      end
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:lookup_message ("message.name", "DestroyObjectMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.log:info ("Message type: " .. self.message:get_name ())
   self.obs:register (self.message, receive, self)

   return self
end
