local LinkHandle = dmz.handle.new ("Node_Link")

function update_object_position (self, ObjectHandle)
   local list = self.list[ObjectHandle]
   if list then
      for _, link in pairs (list) do
         local superPos = dmz.object.position (link.super)
         local subPos = dmz.object.position (link.sub)
         if superPos and subPos then
            local attrPos = superPos + ((subPos - superPos) * .5)
            dmz.object.position (link.attr, nil, attrPos)
         end
      end
   end
end

function destroy_object (self, handle)
   local list = self.list[handle]
   if list then
      for handle, link in pairs (list) do
         if self.list[link.super] then self.list[link.super][link] = nil end
         if self.list[link.sub] then self.list[link.sub][link] = nil end
      end
   end
end

function update_link_object (self, LinkHandle, AttrHandle, Super, Sub, AttrObj, PrevObj)
   if AttrObj then
      local link = { super = Super, sub = Sub, attr = AttrObj }
      if not self.list[Super] then self.list[Super] = {} end
      self.list[Super][LinkHandle] = link
      if not self.list[Sub] then self.list[Sub] = {} end
      self.list[Sub][LinkHandle] = link
      update_object_position (self, Super)
   elseif
         self.list[Super] and
         self.list[Super][LinkHandle] and
         self.list[Super][LinkHandle].attr == PrevObj then
      self.list[Super][LinkHandle] = nil
      self.list[Sub][LinkHandle] = nil
      dmz.object.destroy (PrevObj)
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      objObs = dmz.object_observer.new (),
      list = {},
   }

   self.log:info ("Creating plugin: " .. name)

   local cb = {
      update_object_position = update_object_position,
      destroy_object = destroy_object,
   }

   self.objObs:register (nil, cb, self)

   cb = { update_link_object = update_link_object, }
   self.objObs:register (LinkHandle, cb, self)

   return self
end
