function receive_start (self, type, data)
   if self.linkHandle then
      dmz.object.unlink (self.linkHandle)
      self.linkHandle = nil
   end
   self.handle = nil
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if handle and dmz.object.is_object (handle) then self.handle = handle end
      if self.handle and self.firstHandle and self.secondHandle then
         local pos = dmz.object.position (self.handle)
         if pos then
            dmz.object.position (self.firstHandle, nil, pos)
            dmz.object.position (self.secondHandle, nil, pos)
            self.linkHandle =
               dmz.object.link ("Node_Link", self.firstHandle, self.secondHandle)
         end
      end
   end
end

function receive_update (self, type, data)
   if dmz.data.is_a (data) then
      local pos = data:lookup_vector ("position", 1)
      if pos and self.secondHandle then
         dmz.object.position (self.secondHandle, nil, pos)
      end
   end
end

function receive_end (self, type, data)
   if self.linkHandle then
      dmz.object.unlink (self.linkHandle)
      self.linkHandle = nil
   end
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if handle then
         if handle == self.handle then self.handle = nil; handle = nil
         elseif dmz.object.is_object (handle) and self.handle then 
            local undoHandle = dmz.undo.start_record ("Link Nodes")
            local linkHandle = dmz.object.link ("Node_Link", self.handle, handle)
            if linkHandle then
               local outData = dmz.data.new ()
               outData:store_handle ("object", 1, linkHandle)
               self.editMessage:send_message ("dmzMBRAPluginNodeProperties", outData)
            end
            dmz.undo.stop_record (undoHandle)
         end
      end
   end
   self.handle = nil
end

function receive_fail (self, type, data)
   if self.linkHandle then
      dmz.object.unlink (self.linkHandle)
      self.linkHandle = nil
   end
   self.handle = nil;
end

function new (config, name)
   local self = {
      log = dmz.log.new ("lua." .. name),
      firstHandle = dmz.object.create ("na_tool_link_node"),
      secondHandle = dmz.object.create ("na_tool_link_node"),
      startMessage =
         config:lookup_message_type ("message.first.name", "FirstLinkObjectMessage"),
      updateMessage =
         config:lookup_message_type ("message.update.name", "UpdateLinkPositionMessage"),
      endMessage =
         config:lookup_message_type ("message.second.name", "SecondLinkObjectMessage"),
      failedMessage =
         config:lookup_message_type ("message.failed.name", "FailedLinkObjectsMessage"),
      editMessage =
         config:lookup_message_type ("message.edit.name", "EditObjectAttributesMessage"),
      obs = dmz.message_observer.new (name),
   }

   if self.firstHandle then
      dmz.object.set_temporary (self.firstHandle)
      dmz.object.activate (self.firstHandle)
   end

   if self.secondHandle then
      dmz.object.set_temporary (self.secondHandle)
      dmz.object.activate (self.secondHandle)
   end

   self.log:info ("Creating plugin: " .. name)
   self.obs:register (self.startMessage, receive_start, self)
   self.obs:register (self.updateMessage, receive_update, self)
   self.obs:register (self.endMessage, receive_end, self)
   self.obs:register (self.failedMessage, receive_fail, self)

   return self
end
