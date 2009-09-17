local NodeType = dmz.object_type.new ("na_node")

local function update_object (self, obj)
   local value = ""
   if self.toggle then
      value = obj.name
      if obj.objective and obj.objective ~= "" then
         value = value .. "\n" .. obj.objective
      end
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

local objectiveAttr = {

update_object_text = function (self, handle, attr, value)

   local obj = self.objects[handle]

   if obj then
      obj.objective = value
      update_object (self, obj)
   end
end,

}

local function receive_toggle (self, message, data)
   if dmz.data.is_a (data) then
      self.toggle = data:lookup_boolean ("toggle", 1)
      for _, obj in pairs (self.objects) do update_object (self, obj) end
   end
end

function new (config, name)

   local self = {

      log = dmz.log.new ("lua." .. name),
      obs = dmz.object_observer.new (),
      message =
         config:to_message ("toggle-message.name", "ToggleNodeLabelMessage"),
      msgObs = dmz.message_observer.new (name),
      objects = {},
      toggle = true,
   }

   self.obs:register (nil, defaultAttr, self)
   self.obs:register ("NA_Node_Name", nameAttr, self)
   self.obs:register ("NA_Node_Objective_Label", objectiveAttr, self)
   self.msgObs:register (self.message, receive_toggle, self)
end
