local NodeType = dmz.object_type.new ("na_node")
local NodeLinkType = dmz.object_type.new ("na_link_attribute")
local LinkHandle = dmz.handle.new ("Node_Link")
local ThreatHandle = dmz.handle.new ("NA_Node_Threat")
local VulnerabilityHandle = dmz.handle.new ("NA_Node_Vulnerability")
local PreventionCostHandle = dmz.handle.new ("NA_Node_Prevention_Cost")
local ConsequenceHandle = dmz.handle.new ("NA_Node_Consequence")
local RankHandle = dmz.handle.new ("NA_Node_Rank")
local DegreeHandle = dmz.handle.new ("NA_Node_Degrees")
local Betweennessandle = dmz.handle.new ("NA_Node_Betweenness")
local OverlayState = dmz.definitions.lookup_state ("NA_Node_Overlay")

local WeightDegreesHandle = dmz.handle.new ("NA_Weight_Degrees")
local WeightBetweennessHandle = dmz.handle.new ("NA_Weight_Betweenness")

local function calc_objective_risk (self, object)
   local result = 0
   local Threat = dmz.object.scalar (object, ThreatHandle)
   local Vulnerability = dmz.object.scalar (object, VulnerabilityHandle)
   local Consequence = dmz.object.scalar (object, ConsequenceHandle)
   if Threat and Vulnerability and Consequence then
      result = Threat * Vulnerability * Consequence
   end
   return result
end

local function setup_weight_degrees (self)
   self.maxDegrees = 0
   for object in pairs (self.objects) do
      local value = dmz.object.scalar (object, DegreeHandle)
      if value and (value > self.maxDegrees) then self.maxDegrees = value end
   end
end

local function calc_weight_degrees (self, object)
   local result = 0
   local value = dmz.object.scalar (object, DegreeHandle)
   if value and (self.maxDegrees > 0) then result = value / self.maxDegrees end
   return result
end

local function add_to_link_betweenness_counter (self, link)
   local linkObj = dmz.object.link_attribute_object (link)
   if linkObj then
      local value = dmz.object.add_to_counter (linkObj, BetweennessHandle)
      if value > self.maxBetweenness then self.maxBetweenness = value end
   end
end

local function find_betweenness (self, current, target, visited)
   local list = {}
   local subs = dmz.object.sub_links (current.object, LinkHandle)
   if subs then
      for _, sub in ipairs (subs) do
         local link = dmz.object.lookup_link_handle (LinkHandle, current.object, sub)
         if sub == target then
            current.found = true
            add_to_link_betweenness_counter (self, link)
         elseif not visited[sub] then
            list[#list + 1] = { object = sub, link = link, }
         end
      end
   end
   local supers = dmz.object.super_links (current.object, LinkHandle)
   if supers then
      for _, super in ipairs (supers) do
         local link = dmz.object.lookup_link_handle (LinkHandle, super, current.object)
         if super == target then
            current.found = true
            add_to_link_betweenness_counter (self, link)
         elseif not visited[super] then
            list[#list + 1] = { object = super, link = link, }
         end
      end
   end
   if not current.found then
      for _, group in ipairs (list) do
         visited[group.object] = true
         local node = { object = group.object }
         find_betweenness (self, node, target, visited)
         if node.found then
            local value = dmz.object.add_to_counter (group.object, BetweennessHandle)
            if value > self.maxBetweenness then self.maxBetweenness = value end
            add_to_link_betweenness_counter (self, group.link)
            current.found = true
         end
      end
   end
end

local function setup_weight_betweenness (self)
   self.maxBetweenness = 0
   for object in pairs (self.objects) do
      dmz.object.counter (object, BetweennessHandle, 0)
   end
   for root in pairs (self.objects) do
      if (dmz.object.type (root):is_of_type (NodeType)) then
         for target in pairs (self.objects) do
            if root ~= target and dmz.object.type (target):is_of_type (NodeType) then
               local list = {object = root}
               local visited = {}
               visited[root] = true
               find_betweenness (self, list, target, visited)
               if list.found then
                  local value = dmz.object.add_to_counter (root, BetweennessHandle)
                  if value > self.maxBetweenness then self.maxBetweenness = value end
                  value = dmz.object.add_to_counter (target, BetweennessHandle)
                  if value > self.maxBetweenness then self.maxBetweenness = value end
               end
            end
         end
      end
   end
end

local function calc_weight_betweenness (self, object)
   local result = 0
   local value = dmz.object.counter (object, BetweennessHandle)
   if value and (self.maxBetweenness > 0) then result = value / self.maxBetweenness end
   return result
end

local function rank_object (self, object)
   local result = 0
   if self.objective then
      result = self.objective (self, object)
      for _, calc in pairs (self.weightList) do
         result = result * calc (self, object)
      end
   end
   return result
end

local function receive_rank (self)
   self.visible = true
   local list = {}
   for _, setup in pairs (self.setupList) do
      setup (self)
   end
   for handle in pairs (self.objects) do
      if dmz.handle.is_a (handle) and dmz.object.is_object (handle) then
         local state = dmz.object.state (handle)
         if state then 
            state:unset (OverlayState)
            dmz.object.state (handle, nil, state)
         end
         if dmz.object.text (handle, self.rankAttr) then
            dmz.object.remove_attribute (handle, RankHandle, dmz.object.TextMask)
         end
         local object = { handle = handle, }
         object.rank = rank_object (self, handle)
         list[#list + 1] = object
      end
   end
   table.sort (list, function (obj1, obj2) return obj1.rank > obj2.rank end)
   local count = 1
   local lastRank = nil
   for index, object in ipairs (list) do
      if not lastRank then lastRank = object.rank
      elseif lastRank > object.rank then
         count = count + 1
         lastRank = object.rank
      end
      dmz.object.text (object.handle, RankHandle, tostring (count))
      if self.rankLimit and index <= self.rankLimit and object.rank > 0 then
         local state = dmz.object.state (object.handle)
         if not state then state = dmz.mask.new () end
         state = state:bit_or (OverlayState)
         dmz.object.state (object.handle, nil, state)
      end
   end
end

local function receive_hide (self)
   self.visible = false
   for handle in pairs (self.objects) do
      if dmz.handle.is_a (handle) and dmz.object.is_object (handle) then
         local state = dmz.object.state (handle)
         if state then 
            state:unset (OverlayState)
            dmz.object.state (handle, nil, state)
         end
         if dmz.object.text (handle, RankHandle) then
            dmz.object.remove_attribute (handle, RankHandle, dmz.object.TextMask)
         end
      end
   end
end

local function create_object (self, handle, objType, locality)
   if objType then
      if objType:is_of_type (NodeType) or objType:is_of_type (NodeLinkType) then
         self.objects[handle] = true
         if self.visible and self.objects[handle] then receive_rank (self) end
      end
   end
end

local function update_object_scalar (self, handle, attr, value)
   if self.visible and self.objects[handle] then receive_rank (self) end
end

local function destroy_object (self, handle)
   local updateRank = false
   if self.visible and self.objects[handle] then updateRank = true end
   self.objects[handle] = nil
   if updateRank then receive_rank (self) end
end

local function link_objects (self, link, attr, super, sub)
   if self.visible and self.objects[super] then receive_rank (self) end
end

local function unlink_objects (self, link, attr, super, sub)
   if self.visible and self.objects[super] then receive_rank (self) end
end

function new (config, name)

   local self = {
      visible = false,
      rankLimit = config:to_number ("rank.limit", 9),
      log = dmz.log.new ("lua." .. name),
      rankMessage =
         config:to_message ("message.rank.name", "NARankObjectsMessage"),
      hideMessage =
         config:to_message ("message.rank.name", "NAHideObjectsMessage"),
      msgObs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (),
      objects = {},
      objective = calc_objective_risk,
      setupList = {},
      weightList = {},
   }

--   self.setupList[WeightDegreesHandle] = setup_weight_degrees
--   self.weightList[WeightDegreesHandle] = calc_weight_degrees
   self.setupList[WeightBetweennessHandle] = setup_weight_betweenness
   self.weightList[WeightBetweennessHandle] = calc_weight_betweenness

   self.log:info ("Creating plugin: " .. name)

   self.msgObs:register (self.rankMessage, receive_rank, self)
   self.msgObs:register (self.hideMessage, receive_hide, self)

   local cb = { create_object = create_object, destroy_object = destroy_object }
   self.objObs:register (nil, cb, self)

   cb = { update_object_scalar = update_object_scalar }
   self.objObs:register (ThreatHandle, cb, self)
   self.objObs:register (VulnerabilityHandle, cb, self)
   self.objObs:register (PreventionCostHandle, cb, self)
   self.objObs:register (ConsequenceHandle, cb, self)
   self.objObs:register (DegreeHandle, cb, self)

   cb = { link_objects = link_objects, unlink_objects = unlink_objects, }
   self.objObs:register (LinkHandle, cb, self)

   return self
end
