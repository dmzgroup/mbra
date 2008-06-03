local EliminationHandle = dmz.handle.new ("FT_Threat_Elimination_Cost")
local ConsequenceHandle = dmz.handle.new ("FT_Threat_Consequence")
local ThreatHandle = dmz.handle.new ("FT_Threat_Value")
local AllocationHandle = dmz.handle.new ("FT_Threat_Allocation")
local FTLinkHandle = dmz.handle.new ("FT_Link")
local FTLogicLinkHandle = dmz.handle.new ("FT_Logic_Link")
local AndState = 1
local OrState = 2
local AndMask = dmz.definitions.lookup_state ("FT_Logic_And")
local OrMask = dmz.definitions.lookup_state ("FT_Logic_Or")
local LogicMask = AndMask + OrMask
local ThreatType = dmz.object_type.new ("ft_threat")
local ComponentType = dmz.object_type.new ("ft_component")
local ComponentRootType = dmz.object_type.new ("ft_component_root")

local Level = {}
Level[0] = dmz.definitions.lookup_state ("FT_Threat_Level_0")
Level[1] = dmz.definitions.lookup_state ("FT_Threat_Level_1")
Level[2] = dmz.definitions.lookup_state ("FT_Threat_Level_2")
Level[3] = dmz.definitions.lookup_state ("FT_Threat_Level_3")
Level[4] = dmz.definitions.lookup_state ("FT_Threat_Level_4")
Level[5] = dmz.definitions.lookup_state ("FT_Threat_Level_5")
Level[6] = dmz.definitions.lookup_state ("FT_Threat_Level_6")
Level[7] = dmz.definitions.lookup_state ("FT_Threat_Level_7")
Level[8] = dmz.definitions.lookup_state ("FT_Threat_Level_8")
Level[9] = dmz.definitions.lookup_state ("FT_Threat_Level_9")
Level[10] = dmz.definitions.lookup_state ("FT_Threat_Level_10")
Level.All = Level[0] + Level[1] + Level[2] + Level[3] + Level[4] + Level[5] +
            Level[6] + Level[7] + Level[8] + Level[9] + Level[10]

-- forward declare function
local function round (value)
   local x, y= math.modf (value)
   if y >= 0.5 then x = x + 1 end
   return x
end

local function get_logic_state (node)
   local logicTable = dmz.object.sub_links (node, FTLogicLinkHandle)
   local logic = OrState
   if logicTable then
      local logicState = dmz.object.state (logicTable[1])
      if logicState and logicState:contains (AndMask) then logic = AndState end
   end
   return logic
end

local ObjectCallbacks = {

create_object = function (self, objHandle, objType)
   if objType:is_of_type (ThreatType) then
      self.objects[objHandle] = { handle = objHandle, }
      self.reset = true
   elseif objType == ComponentRootType then
      self.root = objHandle
   end
end,

destroy_object = function (self, objHandle)
   if self.objects[objHandle] then
      self.objects[objHandle] = nil
      self.reset = true
   end
   if objHandle == self.root then self.root = nil end
end,

update_object_state = function (self, objHandle, attrHandle, value, prevValue)
   if value and prevValue then
      if value:bit_and (LogicMask) ~= prevValue:bit_and (LogicMask) then
         self.reset = true
      end
   elseif value then
   end
end
}

local EliminationCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].ec = value
      self.reset = true
   end
end,
}

local ConsequenceCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].consequence = value
      self.reset = true
   end
end,
}

local ThreatCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].threat = value
      self.reset = true
   end
end,
}

local AllocationCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   local object = self.objects[objHandle]
   if object then
      object.allocation = value
      local v = 0
      if object.ec and object.ec > 0 then
         v = 1 - (object.allocation / object.ec)
      end
      local state = dmz.object.state (object.handle)
      if not state then state = dmz.mask.new () end
      state:unset (Level.All)
      if v < 0 then v = 0 end
      v = round (v * 10)
      state = state + Level[v]
      dmz.object.state (object.handle, nil, state)
   end
end,
}

local function create_control_matrix (count, test_func)
   local result = {}
   local elements = math.ldexp (1, count) - 1
   local prev = {}
   local place = 1
   for jy = 1, count do prev[jy] = 0 end
   for ix = 1, elements do
      local current = {}
      local carry = 1
      for jy = 1, count do
         if carry == 0 then current[jy] = prev[jy]
         else
            if prev[jy] > 0 then current[jy] = 0
            else current[jy] = carry; carry = 0
            end
         end
      end
      prev = current
      local val = nil
      if test_func (current) > 0 then
         val = "keep: "
         result[place] = current
         place = place + 1
      else val = "drop: "
      end
      for jy = 1, count do val = val .. current[jy] end
      -- print (val)
   end
   return result
end

local function allocate_budget (self)
   local budget = self.budget
   local list = {}
   for index, object in ipairs (self.index) do
      dmz.object.scalar (object.handle, AllocationHandle, 0)
      list[index] = object
      object.allocation = 0
      if not object.ec then object.ec = 0 end
      if not object.threat then object.threat = 0 end
      if not object.consequence then object.consequence = 0 end
   end
   local count = #list
   if count > 0 then
      while (count > 0) and not dmz.math.is_zero (budget) do
         local slice = budget / count
         for index, object in pairs (list) do
            if slice > (object.ec - object.allocation) then
               budget = budget - (object.ec - object.allocation) 
               object.allocation = object.ec
               count = count - 1
               list[index] = nil
            else
               object.allocation = object.allocation + slice
               budget = budget - slice
            end
         end
      end
      if not dmz.math.is_zero (budget) then
         self.log:error ("All threats fully funded, nothing to do!")
      end
      for index, object in ipairs (self.index) do
         dmz.object.scalar (object.handle, AllocationHandle, object.allocation)
      end
   end
end

local function init (self, node)
   local result = nil
   local type = dmz.object.type (node)
   if type:is_of_type (ThreatType) then
      local which = #self.index + 1
      self.index[which] = self.objects[node]
      if not self.index[which] then
         error ("No object defined for node: " .. tostring (node))
      end
      result = "value[" .. which .. "]"
   else
      local children = dmz.object.sub_links (node, FTLinkHandle)
      if children then
         local logicOperator = " + "
         if get_logic_state (node) == AndState then logicOperator = " * " end
         for index, child in ipairs (children) do
            local value = init (self, child)
            if value then
               if not result then result = "(" .. value
               else result = result .. logicOperator .. value
               end
            end
         end
         if result then result = result .. ")" end
      end
   end
   return result
end

local function calculate_risk (self)
   local risk = 0
      if not self.control then self.control = {} end
      for _, control in ipairs (self.control) do
         local tvSum = 0
         local consequenceSum =0
         for index, object in ipairs (self.index) do
            if object.ec > 0 then
               if control[index] > 0 then
                  tvSum = tvSum + (object.threat * (1 - (object.allocation / object.ec)))
                  consequenceSum = consequenceSum + object.consequence
               else
                  tvSum = tvSum +
                     (1 - (object.threat * (1 - (object.allocation / object.ec))))
               end
            end
         end
         risk = risk + (tvSum * consequenceSum)
      end
   return risk
end

local function start_work (self, mtype, data)
   -- self.log:error ("Starting work")
   if data then self.budget = data:lookup_number ("FTBudget", 1) end
   -- print ("Budget: " .. tostring (self.budget))
   if self.budget and self.root then
      self.index = {}
      local testStr = init (self, self.root)
      if testStr then
         local funcStr = "return function (value) return " .. testStr .. " end"
         -- print (tostring (funcStr))
         local test_func = assert (loadstring (funcStr))()
         self.control = create_control_matrix (#self.index, test_func)
      end
      allocate_budget (self)
      self.risk = calculate_risk (self)
      self.sync:start (self.syncHandle)
   end
end

local function work (self)
   if self.reset then start_work (self); self.reset = nil end
   local size = #self.index
   if size > 1 and self.root then
      local first = math.random (size)
      local second = first
      local count = 0
      while (count < 12) and (second == first) do
         count = count + 1
         second = math.random (size)
      end
      first = self.index[first]
      second = self.index[second]
      if first.handle ~= second.handle then
          -- update nodes
          local firstOrigAllocation = first.allocation
          local secondOrigAllocation = second.allocation
          local maxAddition = second.ec - second.allocation
          if maxAddition > first.allocation then maxAddition = first.allocation end
          if maxAddition > 0.01 then
             local addition = 0.01 + ((maxAddition - 0.01) * math.random ())
             first.allocation = first.allocation - addition
             second.allocation = second.allocation + addition
             local risk = calculate_risk (self)
             if risk < self.risk then
                self.risk = risk
                dmz.object.scalar (first.handle, AllocationHandle, first.allocation)
                dmz.object.scalar (second.handle, AllocationHandle, second.allocation)
             else
                first.allocation = firstOrigAllocation
                second.allocation = secondOrigAllocation
             end
          end
      end
   end
end

local function stop_work (self)
   -- self.log:error ("Stoping work")
   self.sync:stop (self.syncHandle)
end

local function start_plugin (self)
   self.syncHandle = self.sync:create (work, self, self.name)
   self.sync:stop (self.syncHandle)
   self.objObs:register (nil, ObjectCallbacks, self)
   self.objObs:register (EliminationHandle, EliminationCallback, self)
   self.objObs:register (ConsequenceHandle, ConsequenceCallback, self)
   self.objObs:register (ThreatHandle, ThreatCallback, self)
   self.objObs:register (AllocationHandle, AllocationCallback, self)
end

local function stop_plugin (self)
   self.sync:destroy (self.syncHandle)
end

function new (config, name)

   local self = {
      name = name,
      log = dmz.log.new ("lua." .. name),
      sync = dmz.sync.new (),
      start_plugin = start_plugin,
      stop_plugin = stop_plugin,
      startWorkMessage =
         config:lookup_message ("message.start.name", "FTStartWorkMessage"),
      stopWorkMessage =
         config:lookup_message ("message.stop.name", "FTStopWorkMessage"),
      msgObs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (),
      index = {},
      objects = {},
   }

   self.log:info ("Creating plugin: " .. name)

   self.msgObs:register (self.startWorkMessage, start_work, self)
   self.msgObs:register (self.stopWorkMessage, stop_work, self)

   return self
end
