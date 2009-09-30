local EliminationHandle = dmz.handle.new ("FT_Threat_Elimination_Cost")
local ConsequenceHandle = dmz.handle.new ("FT_Threat_Consequence")
local ThreatHandle = dmz.handle.new ("FT_Threat_Value")
local VulnerabilityHandle = dmz.handle.new ("FT_Vulnerability_Value")
local VulnerabilityReducedHandle = dmz.handle.new ("FT_Vulnerability_Reduced_Value")
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

local function update_vulnerability_reduced (object)
   if object then
      local value = 0
      if not object.allocation then object.allocation = 0 end
      if not object.vulnerability then object.vulnerability = 0 end
      if not object.gamma then object.gamma = 0 end
      if object.cost and object.cost > 0 then
         value = object.vulnerability *
            math.exp (-object.gamma * object.allocation / object.cost)
      end
      object.vreduced = value
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
      self.objects[objHandle] = { handle = objHandle, }
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
   local index = {}
   for object in pairs (self.objects) do
      dmz.object.scalar (object, AllocationHandle, 0)
      index[#index + 1] = object
   end
   self.index = index
   if not_zero (self.budget) then
      local A = 0
      local B = 0
      for handle, object in pairs (self.objects) do
         if not object.vulnerability or (object.vulnerability <= 0) then
            object.vulnerability = 1
         end
         object.gamma = -math.log (0.05 / object.vulnerability)
         if not object.cost then object.cost = 0 end
         if not object.threat then object.threat = 0 end
         if not object.consequence then object.consequence = 0 end
         object.logDefenderTerm = log_defender_term (object)
         A = A + object.logDefenderTerm
         if not_zero (object.gamma) then
            B = B + (object.cost / object.gamma)
         end
      end
      local totalAllocation = 0
      local logLamda = 0
      if not_zero (B) then logLamda = (-self.budget - A) / B end
      for _, object in pairs (self.objects) do
         local A = 0
         if not_zero (object.gamma) then A = object.cost / object.gamma end
         local B = 0
         if object.cost > 0 then B = object.logDefenderTerm / object.cost end
         object.allocation = -A * (logLamda + B)
         if object.allocation < 0 then object.allocation = 0 end
         totalAllocation = totalAllocation + object.allocation
      end
      local scale = 1
      if totalAllocation ~= self.budget then
         scale =  self.budget / totalAllocation
      end
      totalAllocation = 0
      for _, object in pairs (self.objects) do
         object.allocation = object.allocation * scale
         totalAllocation = totalAllocation + object.allocation
         dmz.object.scalar (object.handle, AllocationHandle, object.allocation)
      end
   end
   if self.timeSliceHandle then self.timeSlice:start (self.timeSliceHandle) end
   self.reset = nil
end

local function work (self)
   if self.reset then start_work (self) end
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
      self.maxBudget = data:lookup_number (BudgetHandle, 2)
cprint (tostring (self.budget), tostring (self.maxBudget))
      self.reset = true
   end
end

local function start_plugin (self)
   self.timeSliceHandle = self.timeSlice:create (work, self, self.name)
   self.timeSlice:stop (self.timeSliceHandle)
   self.objObs:register (nil, ObjectCallbacks, self)
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
