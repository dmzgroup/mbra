local EliminationHandle = dmz.handle.new ("FT_Threat_Elimination_Cost")
local ConsequenceHandle = dmz.handle.new ("FT_Threat_Consequence")
local ThreatHandle = dmz.handle.new ("FT_Threat_Value")
local VulnerabilityHandle = dmz.handle.new ("FT_Vulnerability_Value")
local VulnerabilityReducedHandle = dmz.handle.new ("FT_Vulnerability_Reduced_Value")
local VulnerabilitySumHandle = dmz.handle.new ("FT_Vulnerability_Sum_Value")
local VulnerabilitySumReducdedHandle = dmz.handle.new (
   "FT_Vulnerability_Sum_Reduced_Value")
local RiskSumHandle = dmz.handle.new ("FT_Risk_Sum_Value")
local RiskSumReducedHandle = dmz.handle.new ("FT_Risk_Sum_Reduced_Value")
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

local function update_vulnerability_reduced (object)
   if object then
      local v = 0
      if not object.allocation then object.allocation = 0 end
      if not object.vulnerability then object.vulnerability = 0 end
      if object.ec and object.ec > 0 then
         v = (1 - (object.allocation / object.ec)) * object.vulnerability
      end
      object.vreduced = v
      dmz.object.scalar (object.handle, VulnerabilityReducedHandle, v)
      local state = dmz.object.state (object.handle)
      if not state then state = dmz.mask.new () end
      state:unset (Level.All)
      if object.vulnerability > 0.0 then
         if v < 0 then v = 0 end
         v = round ((v / object.vulnerability) * 10)
         state = state + Level[v]
      end
      dmz.object.state (object.handle, nil, state)
   end
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
      update_vulnerability_reduced (self.objects[objHandle])
      self.reset = true
   end
end,
}

local ConsequenceCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].consequence = value
      update_vulnerability_reduced (self.objects[objHandle])
      self.reset = true
   end
end,
}

local ThreatCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].threat = value
      update_vulnerability_reduced (self.objects[objHandle])
      self.reset = true
   end
end,
}

local VulnerabilityCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].vulnerability = value
      update_vulnerability_reduced (self.objects[objHandle])
      self.reset = true
   end
end,
}

local AllocationCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   local object = self.objects[objHandle]
   if object then
      object.allocation = value
      update_vulnerability_reduced (self.objects[objHandle])
   end
end,
}

local function create_control_matrix (count)
   local result = {}
   local elements = math.ldexp (1, count) - 1
   local prev = {}
   local place = 1
   for jy = 1, count do prev[jy] = 0 end
   local ix = 0

   return function ()
      ix = ix + 1
      if ix > elements then return end
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
      return current
   end
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

local function calculate_sub_risk (objectList, control)
   local tvProduct = 1
   local consequenceSum = 0
   for index, object in ipairs (objectList) do
      if object.ec > 0 then
         local v = (1 - (object.allocation / object.ec)) * object.vulnerability
         if control[index] > 0 then
            tvProduct = tvProduct * (object.threat * v)
            consequenceSum = consequenceSum + object.consequence
         else
            tvProduct = tvProduct * (1 - (object.threat * v))
         end
      end
   end
   return tvProduct * consequenceSum, tvProduct
end

local function new_risk_is_less (newRisk, oldRisk)
   local result = false
   if not oldRisk then result = true
   elseif (newRisk < oldRisk) and not dmz.math.is_zero (newRisk - oldRisk) then
      result = true
   end
   return result
end

local function create_risk_closure (
      self,
      risk,
      vulnerability,
      control_func,
      calcCount,
      first,
      second,
      firstOrigAllocation,
      secondOrigAllocation)

   return function ()
      local riskResult = true
      local control = control_func ()
      local work = true
      while work do
         calcCount = calcCount + 1
         if self.test_func and  self.test_func (control) > 0 then
            local r, v = calculate_sub_risk (self.index, control)
            risk = risk + r
            vulnerability = vulnerability + v
         end
         if calcCount < 1026 then
            control = control_func ()
            if not control then work = false end
         else work = false
         end
      end
      calcCount = 0
      if not control then
         riskResult = false
         if first and second and firstOrigAllocation and secondOrigAllocation then
            if new_risk_is_less (risk, self.risk) then
               self.risk = risk
               self.vulnerability = vulnerability
               if self.root then
                  dmz.object.scalar (
                     self.root,
                     VulnerabilitySumReducdedHandle,
                     self.vulnerability)
                  dmz.object.scalar (self.root, RiskSumReducedHandle, self.risk)
               end
               dmz.object.scalar (first.handle, AllocationHandle, first.allocation)
               dmz.object.scalar (second.handle, AllocationHandle, second.allocation)
            else
               first.allocation = firstOrigAllocation
               second.allocation = secondOrigAllocation
            end
         else 
            self.risk = risk
            if self.root then
               self.vulnerability = vulnerability
               dmz.object.scalar (
                  self.root,
                  VulnerabilitySumHandle,
                  self.vulnerability)
               dmz.object.scalar (self.root, RiskSumHandle, self.risk)
               dmz.object.scalar (self.root, RiskSumReducedHandle, self.risk)
            end
         end
      end
      return riskResult
   end
end

local function create_test_risk_closure (self, noAdjust)
   local result = nil
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
         if maxAddition > 0.01 or noAdjust then
            if not noAdjust then
               local addition = 0.01 + ((maxAddition - 0.01) * math.random ())
               first.allocation = first.allocation - addition
               second.allocation = second.allocation + addition
            end
            local risk = 0
            local vulnerability = 0
            local calcCount = 0
            result = create_risk_closure (
               self,
               risk,
               vulnerability,
               create_control_matrix (size),
               calcCount,
               first,
               second,
               firstOrigAllocation,
               secondOrigAllocation)
         end
      end
   end
   return result
end

local function start_work (self, mtype, data)
   self.test_func = nil
   self.risk_func = nil
   self.risk = nil
   if data then self.budget = data:lookup_number ("FTBudget", 1) end
   if self.budget and self.root then
      dmz.object.scalar (self.root, VulnerabilitySumHandle, 0.0)
      -- Note: Not needed. Risk is reset when nodes change.
      -- dmz.object.scalar (self.root, RiskSumHandle, 0.0)
      dmz.object.scalar (self.root, VulnerabilitySumReducedHandle, 0.0)
      dmz.object.scalar (self.root, RiskSumReducedHandle, 0.0)
      self.index = {}
      local testStr = init (self, self.root)
      if testStr then
         local funcStr = "return function (value) return " .. testStr .. " end"
         self.test_func = assert (loadstring (funcStr))()
      end
      for _, object in pairs (self.objects) do
         dmz.object.scalar (object.handle, AllocationHandle, 0)
      end
      self.allocate_budget = allocate_budget
      local risk = 0
      local vulnerability = 0
      local calcCount = 0
      self.risk_func = create_risk_closure (
         self,
         risk,
         vulnerability,
         create_control_matrix (#self.index),
         calcCount)
      self.timeSlice:start (self.timeSliceHandle)
   end
end

local function work (self)
   if self.reset then start_work (self); self.reset = nil end
   if self.risk_func then
      if not self.risk_func () then
         if self.allocate_budget then
            self:allocate_budget ()
            self.allocate_budget = nil
            self.risk_func = create_test_risk_closure (self, true)
         else self.risk_func = nil
         end
      end
   end
   if not self.risk_func then self.risk_func = create_test_risk_closure (self) end
end

local function stop_work (self)
   -- self.log:error ("Stoping work")
   self.timeSlice:stop (self.timeSliceHandle)
end

local function start_plugin (self)
   self.timeSliceHandle = self.timeSlice:create (work, self, self.name)
   self.timeSlice:stop (self.timeSliceHandle)
   self.objObs:register (nil, ObjectCallbacks, self)
   self.objObs:register (EliminationHandle, EliminationCallback, self)
   self.objObs:register (ConsequenceHandle, ConsequenceCallback, self)
   self.objObs:register (ThreatHandle, ThreatCallback, self)
   self.objObs:register (AllocationHandle, AllocationCallback, self)
   self.objObs:register (VulnerabilityHandle, VulnerabilityCallback, self)
end

local function stop_plugin (self)
   self.timeSlice:destroy (self.timeSliceHandle)
end

function new (config, name)

   local self = {
      name = name,
      log = dmz.log.new ("lua." .. name),
      timeSlice = dmz.time_slice.new (),
      start_plugin = start_plugin,
      stop_plugin = stop_plugin,
      startWorkMessage =
         config:to_message ("message.start.name", "FTStartWorkMessage"),
      stopWorkMessage =
         config:to_message ("message.stop.name", "FTStopWorkMessage"),
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
