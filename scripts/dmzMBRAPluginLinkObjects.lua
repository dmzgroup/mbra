local NodeType = dmz.object_type.new ("na_node")

function receive_start (self, type, data)
   if self.linkHandle then
      dmz.object.unlink (self.linkHandle)
      self.linkHandle = nil
   end
   self.handle = nil
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if handle and
           dmz.object.is_object (handle) and
           dmz.object.type (handle):is_of_type (NodeType) then self.handle = handle end
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
         elseif dmz.object.is_object (handle) and
               dmz.object.type (handle):is_of_type (NodeType) and
               self.handle then 
            local undoHandle = dmz.undo.start_record ("Link Nodes")
            local linkHandle = dmz.object.link ("Node_Link", self.handle, handle)
            local attrObj = nil
            if linkHandle then
               attrObj = dmz.object.create ("na_link_attribute")
               dmz.object.link_attribute_object (linkHandle, attrObj)
               local outData = dmz.data.new ()
               outData:store_handle ("object", 1, linkHandle)
               outData:store_handle ("created", 1, linkHandle)
               self.editMessage:send ("NetworkAnalysisLinkProperties", outData)
            end
            if dmz.object.is_link (linkHandle) then
               if attrObj and dmz.object.is_object (attrObj) then
                  dmz.object.activate (attrObj)
               end
               dmz.undo.stop_record (undoHandle)
            else
               if attrObj and dmz.object.is_object (attrObj) then
                  dmz.object.destroy (attrObj)
               end
               dmz.undo.abort_record (undoHandle)
            end
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
         config:to_message ("message.first.name", "FirstLinkObjectMessage"),
      updateMessage =
         config:to_message ("message.update.name", "UpdateLinkPositionMessage"),
      endMessage =
         config:to_message ("message.second.name", "SecondLinkObjectMessage"),
      failedMessage =
         config:to_message ("message.failed.name", "FailedLinkObjectsMessage"),
      editMessage =
         config:to_message ("message.edit.name", "EditObjectAttributesMessage"),
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
