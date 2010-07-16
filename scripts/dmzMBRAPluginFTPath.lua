local FTNameHandle = dmz.handle.new ("FT_Name")
local FTLinkHandle = dmz.handle.new ("FT_Link")
local FTPathHandle = dmz.handle.new ("FT_Node_Path_Name")
local iparis = iparis
local pairs = pairs

local function update_path (path, handle)
   local name = dmz.object.text (handle, FTNameHandle)
   if name then
      if path == "" then path = name
      else path = path .. "." .. name
      end
   end
   dmz.object.text (handle, FTPathHandle, path)
   local list = dmz.object.sub_links (handle, FTLinkHandle)
   if list then for _, sub in ipairs (list) do update_path (path, sub) end end
end

local function find_root (handle)
   local result = handle
   local list = dmz.object.super_links (handle, FTLinkHandle)
   if list then  result = find_root (list[1]) end
   return result
end

local function work (self)
   if self.list then
      local roots = {}
      for handle, _ in pairs (self.list) do
         roots[find_root (handle)] = true
      end
      for handle, _ in pairs (roots) do
         update_path ("", handle)
      end
      self.list = nil
   end
end

local function update_name (self, handle)
   if not self.list then self.list = {} end
   self.list[handle] = true
end

local function update_link (self, link, attr, super, sub)
   if not self.list then self.list = {} end
   self.list[super] = true
   self.list[sub] = true
end

local function start_plugin (self)
   self.timeSliceHandle = self.timeSlice:create (work, self, self.name)
   self.objObs:register (FTNameHandle, { update_object_text = update_name }, self)
   self.objObs:register (
      FTLinkHandle,
      { link_objects = update_link, unlink_objects = update_link },
      self)
end

local function stop_plugin (self)
   if self.timeSliceHandle then self.timeSlice:destroy (self.timeSliceHandle) end
end

function new (config, name)

   local self = {
      name = name,
      log = dmz.log.new ("lua." .. name),
      timeSlice = dmz.time_slice.new (),
      start_plugin = start_plugin,
      stop_plugin = stop_plugin,
      objObs = dmz.object_observer.new (),
   }

   self.log:info ("Creating plugin: " .. name)

   return self
end

