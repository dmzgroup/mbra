local NodeType = dmz.object_type.new ("na_node")
local NodeLinkType = dmz.object_type.new ("na_link_attribute")
local LinkHandle = dmz.handle.new ("Node_Link")
local DegreeHandle = dmz.handle.new ("NA_Node_Degrees")

local function update_degrees (handle)
   local count = 0
   local sub = dmz.object.sub_links (handle, LinkHandle)
   if sub then count = count + #sub end
   local super = dmz.object.super_links (handle, LinkHandle)
   if super then count = count + #super end
   dmz.object.scalar (handle, DegreeHandle, count)
end

local function create_object (self, handle, objType, locality)
   if objType then
      if objType:is_of_type (NodeType) then
         self.list[handle] = true
      elseif objType:is_of_type (NodeLinkType) then
         dmz.object.scalar (handle, DegreeHandle, 1)
      end
   end
end

local function destroy_object (self, handle)
   self.list[handle] = nil
end

local function link_objects (self, link, attr, super, sub)
   if self.list[super] then update_degrees (super) end
   if self.list[sub] then update_degrees (sub) end
end

local function unlink_objects (self, link, attr, super, sub)
   if self.list[super] then update_degrees (super) end
   if self.list[sub] then update_degrees (sub) end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      objObs = dmz.object_observer.new (),
      list = {},
   }

   self.log:info ("Creating plugin: " .. name)

   local cb = { create_object = create_object, destroy_object = destroy_object }
   self.objObs:register (nil, cb, self)

   cb = { link_objects = link_objects, unlink_objects = unlink_objects, }
   self.objObs:register (LinkHandle, cb, self)

   return self
end
