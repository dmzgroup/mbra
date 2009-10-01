local EliminationHandle = dmz.handle.new ("FT_Threat_Elimination_Cost")
local ConsequenceHandle = dmz.handle.new ("FT_Threat_Consequence")
local ThreatHandle = dmz.handle.new ("FT_Threat_Value")
local VulnerabilityHandle = dmz.handle.new ("FT_Vulnerability_Value")
local VulnerabilityReducedHandle = dmz.handle.new ("FT_Vulnerability_Reduced_Value")
local RiskHandle = dmz.handle.new ("FT_Risk_Value")
local RiskReducedHandle = dmz.handle.new ("FT_Risk_Reduced_Value")
local VulnerabilitySumHandle = dmz.handle.new ("FT_Vulnerability_Sum_Value")
local VulnerabilitySumReducedHandle = dmz.handle.new (
   "FT_Vulnerability_Sum_Reduced_Value")
local RiskSumHandle = dmz.handle.new ("FT_Risk_Sum_Value")
local RiskSumReducedHandle = dmz.handle.new ("FT_Risk_Sum_Reduced_Value")
local AllocationHandle = dmz.handle.new ("FT_Threat_Allocation")
local FTLinkHandle = dmz.handle.new ("FT_Link")
local FTLogicLinkHandle = dmz.handle.new ("FT_Logic_Link")
local FTActiveFaultTree = dmz.handle.new ("FT_Active_Fault_Tree")
local BudgetHandle = dmz.handle.new ("FT_Budget")
local AndState = 1
local OrState = 2
local XOrState = 3
local AndMask = dmz.definitions.lookup_state ("FT_Logic_And")
local OrMask = dmz.definitions.lookup_state ("FT_Logic_Or")
local XOrMask = dmz.definitions.lookup_state ("FT_Logic_XOr")
local LogicMask = AndMask + OrMask + XOrMask
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

local function not_zero (value) return not dmz.math.is_zero (value) end

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
      if logicState then
         if logicState:contains (AndMask) then logic = AndState
         elseif logicState:contains (XOrMask) then logic = XOrState
         end
      end
   end
   return logic
end

local function new_object (handle)
   return {

      handle = handle,
      cost = 0,
      threat = 0,
      vulnerability = 0,
      consequence = 0,
      allocation = 0,
      gamma = 0,
      vreduced = 0,
   }
end

local function update_vulnerability_reduced (object)
   if object then
      local value = 0
      if object.cost > 0 then
         value = object.vulnerability *
            math.exp (-object.gamma * object.allocation / object.cost)
      end
      object.vreduced = value
      dmz.object.scalar (
         object.handle,
         RiskHandle,
         object.vulnerability * object.consequence * object.threat)
      dmz.object.scalar (
         object.handle,
         RiskReducedHandle,
         value * object.consequence * object.threat)
      dmz.object.scalar (object.handle, VulnerabilityReducedHandle, value)
      local state = dmz.object.state (object.handle)
      if not state then state = dmz.mask.new () end
      state:unset (Level.All)
      if object.vulnerability > 0 then
         if value < 0 then value = 0 end
         value = round ((value / object.vulnerability) * 10)
         state = state + Level[value]
      end
      dmz.object.state (object.handle, nil, state)
   end
end

local ObjectCallbacks = {

create_object = function (self, objHandle, objType)
   if objType:is_of_type (ThreatType) then
      self.objects[objHandle] = new_object (objHandle)
      self.reset = true
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

local LinkCallbacks = {

link_objects = function (self, link, attr, super, sub)
   if self.object[super] or self.object[sub] then
      self.reset = true
   end
end,

unlink_objects = function (self, link, attr, super, sub)
   if self.object[super] or self.object[sub] then
      self.reset = true
   end
end,
}

local RootCallback = {

update_object_flag = function (self, object, attrHandle, value)
   if value then
      self.root = object
      self.reset = true
   end
end
}

local EliminationCallback = {

update_object_scalar = function (self, objHandle, attrHandle, value)
   if self.objects[objHandle] then
      self.objects[objHandle].cost = value
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

local function build_index (self, node)
   local nodeList = dmz.object.sub_links (node, FTLinkHandle)
   for _, object in ipairs (nodeList) do
      local otype = dmz.object.type (object)
      if otype:is_of_type (ThreatType) then
         local ref = self.objects[object]
         if ref then self.index[#(self.index) + 1] = ref end
      elseif otype:is_of_type (ComponentType) then
         build_index (self, object)
      end
   end
end

local function risk_and (objects)
   local value = 0
   if #objects > 0 then value = 1 end
   for _, object in ipairs (objects) do
      value = value * object.threat * object.vreduced
   end
   local result = 0
   for _, object in ipairs (objects) do
      result = result + (value * object.consequence)
   end
   return result
end

local function risk_or (objects)
   local value = 0
   for _, object in ipairs (objects) do
      value = value + (object.threat * object.vreduced)
   end
   local result = 0
   for _, object in ipairs (objects) do
      result = result + (value * object.consequence)
   end
   return result
end

local function risk_xor (objects)
   local result = 0
   for idex, current in ipairs (objects) do
      local value = 1
      for jdex, object in ipairs (objects) do
         if idex ~= jdex then value = value * (1 - (object.threat * object.vreduced))
         else value = value * object.threat * object.vreduced
         end
      end
      result = result + (value * current.consequence)
   end
   return result
end

local function vulnerability_and (subv)
   local result = 1
   for _, value in ipairs (subv) do
      result = result * value
   end
   return result
end

local function vulnerability_or (subv)
   local result = 1
   for _, value in ipairs (subv) do
      result = result * (1 - value)
   end
   return 1 - result
end

local function vulnerability_xor (subv)
   local result = 1
   for idex, _ in ipairs (subv) do
      local product = 1
      for jdex, value in ipairs (subv) do
         if jdex ~= idex then product = product * (1 - value)
         else product = product * value
         end
      end
      result = result * product
   end
   return result
end

local function traverse_fault_tree (self, node)
   local nodeList = dmz.object.sub_links (node, FTLinkHandle)
   if not nodeList then nodeList = {} end
   local threatList = {}
   local subv = {}
   for _, object in ipairs (nodeList) do
      local otype = dmz.object.type (object)
      if otype:is_of_type (ThreatType) then
         local ref = self.objects[object]
         if ref then
            subv[#subv + 1] = ref.threat * ref.vreduced
            threatList[#threatList + 1] = ref
         end
      elseif otype:is_of_type (ComponentType) then
         subv[#subv + 1] = traverse_fault_tree (self, object)
      end
   end
   local op = get_logic_state (node)
   local result = 0
   if AndState == op then
      self.risk = self.risk + risk_and (threatList)
      result = vulnerability_and (subv)
   elseif OrState == op then
      self.risk = self.risk + risk_or (threatList)
      result = vulnerability_or (subv)
   elseif XOrState == op then
      self.risk = self.risk + risk_xor (threatList)
      result = vulnerability_xor (subv)
   else
      self.log:error ("Unknown logic operator:", op)
   end
   return result
end

local function log_defender_term (object)
   local result = object.threat * object.consequence * object.vulnerability *
      object.gamma
   if not_zero (result) then
      result = object.cost / result
      if not_zero (object.gamma) then
         result = (object.cost / object.gamma) * math.log (result)
      else result = 0
      end
   else result = 0
   end
   return result
end

local function start_work (self)
   self.index = {}
   build_index (self, self.root)
   for _, object in ipairs (self.index) do
      dmz.object.scalar (object.handle, AllocationHandle, 0)
   end
   if self.root then
      self.risk = 0
      local vulnerability = traverse_fault_tree (self, self.root)
      dmz.object.scalar (self.root, RiskSumHandle, self.risk)
      dmz.object.scalar (self.root, RiskSumReducedHandle, self.risk)
      dmz.object.scalar (self.root, VulnerabilitySumHandle, vulnerability)
      dmz.object.scalar (self.root, VulnerabilitySumReducedHandle, vulnerability)
   end
   if not_zero (self.budget) then
      local A = 0
      local B = 0
      for _, object in ipairs (self.index) do
         if object.vulnerability <= 0 then object.vulnerability = 1 end
         object.gamma = -math.log (0.05 / object.vulnerability)
         object.logDefenderTerm = log_defender_term (object)
         A = A + object.logDefenderTerm
         if not_zero (object.gamma) then
            B = B + (object.cost / object.gamma)
         end
      end
      local totalAllocation = 0
      local logLamda = 0
      if not_zero (B) then logLamda = (-self.budget - A) / B end
      for _, object in ipairs (self.index) do
         local A = 0
         if not_zero (object.gamma) then A = object.cost / object.gamma end
         local B = 0
         if object.cost > 0 then B = object.logDefenderTerm / object.cost end
         object.allocation = -A * (logLamda + B)
         if object.allocation < 0 then object.allocation = 0 end
         totalAllocation = totalAllocation + object.allocation
      end
      local scale = 1
      if totalAllocation > 0 then
         scale = self.budget / totalAllocation
      end
      totalAllocation = 0
      for _, object in ipairs (self.index) do
         object.allocation = object.allocation * scale
         totalAllocation = totalAllocation + object.allocation
         dmz.object.scalar (object.handle, AllocationHandle, object.allocation)
      end
      if self.root then
         self.risk = 0
         local vulnerability = traverse_fault_tree (self, self.root)
         dmz.object.scalar (self.root, RiskSumReducedHandle, self.risk)
         dmz.object.scalar (self.root, VulnerabilitySumReducedHandle, vulnerability)
      end
   end
   if self.timeSliceHandle then self.timeSlice:start (self.timeSliceHandle) end
   self.reset = nil
end

local function work (self)
   if self.reset then start_work (self) end
   local size = #(self.index)
   if size > 1 then
      local source = math.random (size)
      local target = source
      local count = 0
      while (count < 12) and (source == target) do
         count = count + 1
         target = math.random (size)
      end
      source = self.index[source]
      target = self.index[target]
--cprint (source.handle, target.handle)
      if source and target and source.handle ~= target.handle then
         local sourceAllocation = source.allocation
         local targetAllocation = target.allocation
         local maxAddition = target.cost - target.allocation
         if maxAddition > source.allocation then maxAddition = source.allocation end
         if maxAddition > 0.01 then
            local addition = 0.01 + ((maxAddition - 0.01) * math.random ())
            dmz.object.scalar (
               source.handle,
               AllocationHandle,
               source.allocation - addition)
            dmz.object.scalar (
               target.handle,
               AllocationHandle,
               target.allocation + addition)
            local riskOrig = dmz.object.scalar (self.root, RiskSumReducedHandle)
            local vulnerabilityOrig = dmz.object.scalar (
               self.root,
               VulnerabilitySumReducedHandle)
            self.risk = 0
            local vulnerability = traverse_fault_tree (self, self.root)
--cprint (vulnerability, vulnerabilityOrig)
            if vulnerabilityOrig < vulnerability then
               dmz.object.scalar (
                  source.handle,
                  AllocationHandle,
                  sourceAllocation)
               dmz.object.scalar (
                  target.handle,
                  AllocationHandle,
                  targetAllocation)
            else
               dmz.object.scalar (self.root, RiskSumReducedHandle, self.risk)
               dmz.object.scalar (self.root, VulnerabilitySumReducedHandle, vulnerability)
            end 
         end
      end
   end
end

local function stop_work (self)
   self.reset = nil
   if self.timeSliceHandle then self.timeSlice:stop (self.timeSliceHandle) end
end

local function receive_work (self, msg, data)
   if dmz.data.is_a (data) then
      if data:lookup_boolean ("Boolean", 1) then start_work (self)
      else stop_work (self)
      end
   end
end

local function receive_budget (self, msg, data)
   if dmz.data.is_a (data) then
      self.budget = data:lookup_number (BudgetHandle, 1)
      if not self.budget then self.budget = 0 end
      self.maxBudget = data:lookup_number (BudgetHandle, 2)
      if not self.maxBudget then self.maxBudget = 0 end
      self.reset = true
   end
end

local function start_plugin (self)
   self.timeSliceHandle = self.timeSlice:create (work, self, self.name)
   self.timeSlice:stop (self.timeSliceHandle)
   self.objObs:register (nil, ObjectCallbacks, self)
   self.objObs:register (FTLinkHandle, LinkCallbacks, self)
   self.objObs:register (FTActiveFaultTree, RootCallback, self)
   self.objObs:register (EliminationHandle, EliminationCallback, self)
   self.objObs:register (ConsequenceHandle, ConsequenceCallback, self)
   self.objObs:register (ThreatHandle, ThreatCallback, self)
   self.objObs:register (AllocationHandle, AllocationCallback, self)
   self.objObs:register (VulnerabilityHandle, VulnerabilityCallback, self)

   self.msgObs:register (self.simMessage, receive_work, self)
   self.msgObs:register (self.budgetMessage, receive_budget, self)
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
      simMessage =
         config:to_message ("simulator-message.name", "FTSimulatorMessage"),
      budgetMessage =
         config:to_message ("budget-message.name", "FTBudgetMessage"),
      msgObs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (),
      index = {},
      objects = {},
      budget = 0,
      maxBudget = 0,
   }

   self.log:info ("Creating plugin: " .. name)

   return self
end
