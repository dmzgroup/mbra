local NodeType = dmz.object_type.new ("na_node")

local function update_object (self, obj)
   local value = obj.name
   if self.currentAttr then
      local str = obj[self.currentAttr]
      if str then value = value .. "\n" .. str end
   end
   dmz.object.text (obj.handle, "NA_Node_Label", value)
end

local defaultAttr = {

create_object = function (self, handle, objType)
   if objType:is_of_type (NodeType) then
      self.objects[handle] = { handle = handle, name = "", }
   end
end,

destroy_object = function (self, handle)
   self.objects[handle] = nil
end,
}

local nameAttr = {

update_object_text = function (self, handle, attr, value)

   local obj = self.objects[handle]

   if obj then
      obj.name = value
      update_object (self, obj)
   end
end,

}

function new (config, name)

   local self = {

      log = dmz.log.new ("lua." .. name),
      obs = dmz.object_observer.new (),
      objects = {},
   }

   self.obs:register (nil, defaultAttr, self)
   self.obs:register ("NA_Node_Name", nameAttr, self)
end
