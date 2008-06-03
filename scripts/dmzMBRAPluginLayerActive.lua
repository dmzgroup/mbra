local ActiveHandle = dmz.handle.new ("Layer_Active")

function update_object_flag (self, handle, attr, value)
   if value then
      if self.active and self.active ~= handle then
         dmz.object.flag (self.active, ActiveHandle, false)
      end
      self.active = handle
   else
      if self.active == handle then self.active = nil end
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      objObs = dmz.object_observer.new (),
   }

   self.log:info ("Creating plugin: " .. name)

   local cb = { update_object_flag = update_object_flag }
   self.objObs:register (ActiveHandle, cb, self)

   return self
end
