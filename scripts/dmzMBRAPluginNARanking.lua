local NodeType = dmz.object_type.new ("na_node")
local NodeLinkType = dmz.object_type.new ("na_link_attribute")
local LinkHandle = dmz.handle.new ("Node_Link")
local ECHandle = dmz.handle.new ("NA_Node_Elimination_Cost")
local ConsequenceHandle = dmz.handle.new ("NA_Node_Consequence")
local RankHandle = dmz.handle.new ("NA_Node_Rank")

function rank_object (handle)
   local result = 0
   local ec = dmz.object.scalar (handle, ECHandle)
   local consequence = dmz.object.scalar (handle, ConsequenceHandle)
   local count = 0
   local isLink = false
   local objType = dmz.object.type (handle)
   if objType and objType:is_of_type (NodeLinkType) then isLink = true end
   if isLink then count = 1
   else
      local sub = dmz.object.sub_links (handle, LinkHandle)
      if sub then count = count + #sub end
      local super = dmz.object.super_links (handle, LinkHandle)
      if super then count = count + #super end
   end
   if ec and ec > 0 and consequence and count then result = (count * consequence) / ec end
   return result
end

function receive_rank (self)
   self.visible = true
   local list = {}
   for handle in pairs (self.list) do
      if dmz.handle.is_a (handle) and dmz.object.is_object (handle) then
         local state = dmz.object.state (handle)
         if state then 
            state:unset (self.overlayState)
            dmz.object.state (handle, nil, state)
         end
         if dmz.object.text (handle, self.rankAttr) then
            dmz.object.remove_attribute (handle, RankHandle, dmz.object.TextMask)
         end
         local object = { handle = handle, }
         object.rank = rank_object (handle)
         list[#list + 1] = object
      end
   end
   table.sort (list, function (obj1, obj2) return obj1.rank > obj2.rank end)
   for index, object in ipairs (list) do
      dmz.object.text (object.handle, RankHandle, tostring (index))
      if self.rankLimit and index <= self.rankLimit and object.rank > 0 then
         local state = dmz.object.state (object.handle)
         if not state then state = dmz.mask.new () end
         state = state:bit_or (self.overlayState)
         dmz.object.state (object.handle, nil, state)
      end
   end
end

function receive_hide (self)
   self.visible = false
   for handle in pairs (self.list) do
      if dmz.handle.is_a (handle) and dmz.object.is_object (handle) then
         local state = dmz.object.state (handle)
         if state then 
            state:unset (self.overlayState)
            dmz.object.state (handle, nil, state)
         end
         if dmz.object.text (handle, RankHandle) then
            dmz.object.remove_attribute (handle, RankHandle, dmz.object.TextMask)
         end
      end
   end
end

function create_object (self, handle, objType, locality)
   if objType then
      if objType:is_of_type (NodeType) or objType:is_of_type (NodeLinkType) then
         self.list[handle] = true
         if self.visible and self.list[handle] then receive_rank (self) end
      end
   end
end

function update_object_scalar (self, handle, attr, value)
   if self.visible and self.list[handle] then receive_rank (self) end
end

function destroy_object (self, handle)
   local updateRank = false
   if self.visible and self.list[handle] then updateRank = true end
   self.list[handle] = nil
   if updateRank then receive_rank (self) end
end

function link_objects (self, link, attr, super, sub)
   if self.visible and self.list[super] then receive_rank (self) end
end

function unlink_objects (self, link, attr, super, sub)
   if self.visible and self.list[super] then receive_rank (self) end
end

function new (config, name)

   local self = {
      visible = false,
      rankLimit = config:to_number ("rank.limit", 9),
      overlayState = dmz.definitions.lookup_state ("NA_Node_Overlay"),
      log = dmz.log.new ("lua." .. name),
      rankMessage =
         config:to_message ("message.rank.name", "NARankObjectsMessage"),
      hideMessage =
         config:to_message ("message.rank.name", "NAHideObjectsMessage"),
      msgObs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (),
      list = {},
   }

   self.log:info ("Creating plugin: " .. name)

   self.msgObs:register (self.rankMessage, receive_rank, self)
   self.msgObs:register (self.hideMessage, receive_hide, self)

   local cb = { create_object = create_object, destroy_object = destroy_object }
   self.objObs:register (nil, cb, self)

   cb = { update_object_scalar = update_object_scalar }
   self.objObs:register (ECHandle, cb, self)
   self.objObs:register (ConsequenceHandle, cb, self)

   cb = { link_objects = link_objects, unlink_objects = unlink_objects, }
   self.objObs:register (LinkHandle, cb, self)

   return self
end
