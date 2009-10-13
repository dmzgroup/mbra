local NodeType = dmz.object_type.new ("na_node")
local NodeLinkType = dmz.object_type.new ("na_link_attribute")

function receive (self, type, data)
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if handle then 
         local link = self.objects[handle]
         if link then handle = link end
         local objType = dmz.object.type (handle)
         if
               objType and
               objType:is_of_type (NodeType) and
               dmz.object.is_object (handle) then
            local undoHandle = dmz.undo.start_record ("Delete Node")
            dmz.object.destroy (handle)
            dmz.undo.stop_record (undoHandle)
         elseif dmz.object.is_link (handle) then
            local undoHandle = dmz.undo.start_record ("Unlink Nodes")
            dmz.object.unlink (handle)
            dmz.undo.stop_record (undoHandle)
         end
      else self.log:error ("Got null handle!")
      end
   end
end

local DefaultCallbacks = {

destroy_object = function (self, object)
   self.objects[object] = nil
end,

}

local LinkCallbacks = {

update_link_object = function (self, link, attr, super, sub, object, prev)
   if object and dmz.object.type (object):is_of_type (NodeLinkType) then
      self.objects[object] = link
   end
end,

}

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("message.name", "DestroyObjectMessage"),
      obs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (name),
      objects = {},
   }

   self.log:info ("Creating plugin: " .. name)
   self.log:info ("Message type: " .. self.message:get_name ())
   self.obs:register (self.message, receive, self)

   self.objObs:register (nil, DefaultCallbacks, self)
   self.objObs:register ("Node_Link", LinkCallbacks, self)

   return self
end
