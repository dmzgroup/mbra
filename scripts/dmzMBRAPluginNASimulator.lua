local NodeType = dmz.object_type.new ("na_node")
local NodeLinkType = dmz.object_type.new ("na_link_attribute")
local SimType = dmz.object_type.new ("na_simulator")
local LabelHandle = dmz.handle.new ("NA_Node_Objective_Label")
local LinkHandle = dmz.handle.new ("Node_Link")
local LinkFlowHandle = dmz.handle.new ("NA_Link_Flow")
local ThreatHandle = dmz.handle.new ("NA_Node_Threat")
local VulnerabilityHandle = dmz.handle.new ("NA_Node_Vulnerability")
local VulnerabilityReducedHandle = dmz.handle.new ("NA_Node_Vulnerability_Reduced")
local PreventionCostHandle = dmz.handle.new ("NA_Node_Prevention_Cost")
local PreventionAllocationHandle = dmz.handle.new ("NA_Node_Prevention_Allocation")
local ConsequenceHandle = dmz.handle.new ("NA_Node_Consequence")
local RiskInitialHandle = dmz.handle.new ("NA_Node_Risk_Initial")
local RiskReducedHandle = dmz.handle.new ("NA_Node_Risk_Reduced")
local WeightHandle = dmz.handle.new ("NA_Node_Weight")
local WeightAndObjectiveHandle = dmz.handle.new ("NA_Node_Weight_And_Objective")
local GammaHandle = dmz.handle.new ("NA_Node_Gamma")
local RankHandle = dmz.handle.new ("NA_Node_Rank")
local DegreeHandle = dmz.handle.new ("NA_Node_Degrees")
local BetweennessHandle = dmz.handle.new ("NA_Node_Betweenness")
local HeightHandle = dmz.handle.new ("NA_Node_Height")
local OverlayState = dmz.definitions.lookup_state ("NA_Node_Overlay")
local ForwardState = dmz.definitions.lookup_state ("NA_Flow_Forward")
local ReverseState = dmz.definitions.lookup_state ("NA_Flow_Reverse")
local FlowStateMask = ForwardState + ReverseState

local WeightDegreesHandle = dmz.handle.new ("NA_Weight_Degrees")
local WeightBetweennessHandle = dmz.handle.new ("NA_Weight_Betweenness")
local WeightHeightHandle = dmz.handle.new ("NA_Weight_Height")

local ObjectiveNoneHandle = dmz.handle.new ("NA_Objective_None")
local ObjectiveRiskHandle = dmz.handle.new ("NA_Objective_Risk")
local ObjectiveContagiousHandle = dmz.handle.new ("NA_Objective_Contagiousness")
local ObjectiveTxVHandle = dmz.handle.new ("NA_Objective_TxV")
local ObjectiveThreatHandle = dmz.handle.new ("NA_Objective_Threat")
local ObjectiveVulnerabilityHandle = dmz.handle.new ("NA_Objective_Vulnerability")
local ObjectiveConsequenceHandle = dmz.handle.new ("NA_Objective_Consequence")

local function not_zero (value) return not dmz.math.is_zero (value) end

local function calc_risk_initial (object)
   local Threat = dmz.object.scalar (object, ThreatHandle)
   local Vulnerability = dmz.object.scalar (object, VulnerabilityHandle)
   local Consequence = dmz.object.scalar (object, ConsequenceHandle)
   if Threat and Vulnerability and Consequence then
      dmz.object.scalar (
         object,
         RiskInitialHandle,
         Threat * Vulnerability * Consequence)
   else
      dmz.object.scalar (object, RiskInitialHandle, 0)
   end
end

local function calc_vulnerability (object)
   local result = 0
   local Allocation = dmz.object.scalar (object, PreventionAllocationHandle)
   local Vulnerability = dmz.object.scalar (object, VulnerabilityHandle)
   if Vulnerability then result = Vulnerability end
   local Cost = dmz.object.scalar (object, PreventionCostHandle)
   local Gamma = dmz.object.scalar (object, GammaHandle)
   if Gamma and not_zero (Gamma) and Vulnerability and (Vulnerability > 0) and
         Cost and (Cost > 0) and not_zero (Cost) and
         Allocation and not_zero (Allocation) then
      result = Vulnerability * math.exp (-Gamma * Allocation / Cost)
   end
   dmz.object.scalar (object, VulnerabilityReducedHandle, result)
   return result;
end

local function calc_risk_reduced (object)
   local result = 0
   local Threat = dmz.object.scalar (object, ThreatHandle)
   local Vulnerability = calc_vulnerability (object)
   local Consequence = dmz.object.scalar (object, ConsequenceHandle)
   if Threat and Vulnerability and Consequence then
      result = Threat * Vulnerability * Consequence
   end
   dmz.object.scalar (object, RiskReducedHandle, result)
   return result
end

local function calc_objective_none (self, object)
   dmz.object.text (object, LabelHandle, "")
   return 1
end

local function format_result (value)
   value = value * 100
   local ival = math.floor (value)
   if (value - ival) >= 0.5 then ival = ival + 1 end
   return tostring (ival) .. "%"
end

local function calc_objective_risk (self, object)
   local result = dmz.object.scalar (object, RiskReducedHandle)
   if self.visible then
      dmz.object.text (object, LabelHandle, "Risk = " .. string.format ("%.2f", result))
   end
   return result
end

local function calc_objective_contagiousness (self, object)
   local result = 0
   local Threat = dmz.object.scalar (object, ThreatHandle)
   local Vulnerability = dmz.object.scalar (object, VulnerabilityReducedHandle)
   local Degrees = dmz.object.scalar (object, DegreeHandle)
   if Threat and Vulnerability and Degrees then
      result = Threat * Vulnerability * Degrees
   end
   if self.visible then
      dmz.object.text (
         object,
         LabelHandle,
         "Contagiousness = " .. string.format ("%.2f", result))
   end
   return result
end

local function calc_objective_txv (self, object)
   local result = 0
   local Threat = dmz.object.scalar (object, ThreatHandle)
   local Vulnerability = dmz.object.scalar (object, VulnerabilityReducedHandle)
   if Threat and Vulnerability then result = Threat * Vulnerability end
   if self.visible then
      dmz.object.text (object, LabelHandle, "T x V = " .. format_result (result))
   end
   return result
end

local function calc_objective_threat (self, object)
   local result = dmz.object.scalar (object, ThreatHandle)
   if not result then result = 0 end
   if self.visible then
      dmz.object.text (object, LabelHandle, "Threat = " .. format_result (result))
   end
   return result
end

local function calc_objective_vulnerability (self, object)
   local result = dmz.object.scalar (object, VulnerabilityReducedHandle)
   if not result then result = 0 end
   if self.visible then
      dmz.object.text (object, LabelHandle, "Vulnerability = " .. format_result (result))
   end
   return result
end

local function calc_objective_consequence (self, object)
   local result = dmz.object.scalar (object, ConsequenceHandle)
   if not result then result = 0 end
   if self.visible then
      dmz.object.text (
         object,
         LabelHandle,
         "Consequence = $" .. string.format ("%.2f", result))
   end
   return result
end

local weight_degrees = {

setup = function (self)
   self.maxDegrees = 0
   for object in pairs (self.objects) do
      local value = dmz.object.scalar (object, DegreeHandle)
      if value and (value > self.maxDegrees) then self.maxDegrees = value end
   end
end,

calc = function (self, object)
   local result = 0
   local value = dmz.object.scalar (object, DegreeHandle)
   if value and (self.maxDegrees > 0) then result = value / self.maxDegrees end
   return result
end,

}

local EmptyMask = dmz.mask.new ()

local function link_reachable (link, flowState)
   local result = true
   local obj = dmz.object.link_attribute_object (link)
   if obj then
      local state = dmz.object.state (obj, LinkFlowHandle)
      if state and (state:bit_and (FlowStateMask) ~= EmptyMask) then
         if not state:contains (flowState) then result = false end
      end
   end
   return result
end

local function all_links_flow (superList, subList, flowState)
   local result = true
   if superList and subList then
      for _, super in ipairs (superList) do
         if not result then break end
         for _, sub in ipairs (subList) do
            local link = dmz.object.lookup_link_handle (LinkHandle, super, sub)
            local obj = nil
            if link then
               obj = dmz.object.link_attribute_object (link)
            else
               cprint ("No link found")
            end
            if obj then
               local state = dmz.object.state (obj, LinkFlowHandle)
               if state then
                  if state:bit_and (FlowStateMask) ~= flowState then
                     result = false
                     break
                  end
               else
                  result = false
                  break
               end
            else
               result = false
               break
            end
         end
      end
   end
   return result
end

local function is_sink (object)
   local result = false
   local sub = dmz.object.sub_links (object, LinkHandle)
   local super = dmz.object.super_links (object, LinkHandle)
   if sub or super then
      if all_links_flow ({object}, sub, ReverseState) and
            all_links_flow (super, {object}, ForwardState) then
         result = true
      end
   end
   return result
end

local function add_to_node_betweenness_counter (self, object)
   local value = dmz.object.add_to_counter (object, BetweennessHandle)
   if value > self.maxBetweenness then self.maxBetweenness = value end
end

local function add_to_link_betweenness_counter (self, link)
   local linkObj = dmz.object.link_attribute_object (link)
   if linkObj then
      local value = dmz.object.add_to_counter (linkObj, BetweennessHandle)
      if value > self.maxBetweenness then self.maxBetweenness = value end
   end
end

local function find_betweenness (self, current, target, visited)
   for _, item in ipairs (current) do visited[item.object] = true end
   local found = false
   local list = {}
   for _, item in ipairs (current) do
      local subs = dmz.object.sub_links (item.object, LinkHandle)
      if subs then
         for _, sub in ipairs (subs) do
            local link = dmz.object.lookup_link_handle (LinkHandle, item.object, sub)
            if link_reachable (link, ForwardState) then
               if sub == target then
                  found = true
                  item.found = true
                  add_to_node_betweenness_counter (self, target)
                  add_to_link_betweenness_counter (self, link)
               elseif not visited[sub] then
                  list[#list + 1] = { object = sub, link = link, parent = item, }
               end
            end
         end
      end
      local supers = dmz.object.super_links (item.object, LinkHandle)
      if supers then
         for _, super in ipairs (supers) do
            local link = dmz.object.lookup_link_handle (LinkHandle, super, item.object)
            if link_reachable (link, ReverseState) then
               if super == target then
                  found = true
                  item.found = true
                  add_to_node_betweenness_counter (self, target)
                  add_to_link_betweenness_counter (self, link)
               elseif not visited[super] then
                  list[#list + 1] = { object = super, link = link, parent = item, }
               end
            end
         end
      end
   end
   if not found and #list > 0 then
      find_betweenness (self, list, target, visited)
      local place = #list
      while place > 0 do
         local item = list[place]
         if item.found then
            add_to_node_betweenness_counter (self, item.object)
            add_to_link_betweenness_counter (self, item.link)
            item.parent.found = true
         end
         place = place - 1
      end
   end
end

local weight_betweenness = {

setup = function (self)
   self.maxBetweenness = 0
   for object in pairs (self.objects) do
      dmz.object.counter (object, BetweennessHandle, 0)
   end
   for root in pairs (self.objects) do
      if (dmz.object.type (root):is_of_type (NodeType)) then
         for target in pairs (self.objects) do
            if root ~= target and dmz.object.type (target):is_of_type (NodeType) then
               local list = {}
               list[#list + 1] = {object = root}
               local visited = {}
               find_betweenness (self, list, target, visited)
               if list[1].found then add_to_node_betweenness_counter (self, root) end
            end
         end
      end
   end
end,

calc = function (self, object)
   local result = 0
   local value = dmz.object.counter (object, BetweennessHandle)
   if value and (self.maxBetweenness > 0) then result = value / self.maxBetweenness end
   return result
end,

}

local function update_height (obj, level)
   if dmz.object.is_link (obj) then
      obj = dmz.object.link_attribute_object (obj)
   end
   if obj then
      local value = dmz.object.counter (obj, HeightHandle)
      if not value then value = 0 end
      if level > value then dmz.object.counter (obj, HeightHandle, level) end
   end
end

local function find_height (self, current, visited, level)
   if not visited[current] then
      visited[current] = true
      update_height (current, level)
      local subList = dmz.object.sub_links (current, LinkHandle)
      if subList then
         for _, sub in ipairs (subList) do
            local link = dmz.object.lookup_link_handle (LinkHandle, current, sub)
            if link then
               if link_reachable (link, ReverseState) then
                  update_height (link, level + 1)
                  find_height (self, sub, visited, level + 2)
               end
            else
self.log:error ("No link found!")
            end
         end
      end
      local superList = dmz.object.super_links (current, LinkHandle)
      if superList then
         for _, super in ipairs (superList) do
            local link = dmz.object.lookup_link_handle (LinkHandle, super, current)
            if link then
               if link_reachable (link, ForwardState) then
                  update_height (link, level + 1)
                  find_height (self, super, visited, level + 2)
               end
            else
self.log:error ("No link found!")
            end
         end
      end
   end
end

local weight_height = {

setup = function (self)
   self.maxHeight = 0
   for object in pairs (self.objects) do
      dmz.object.counter (object, HeightHandle, 0)
   end
   for root in pairs (self.objects) do
      if is_sink (root) then
         --self.log:error ("Is sink:", root)
         find_height (self, root, {}, 1)
      end
   end
   for object in pairs (self.objects) do
      local height = dmz.object.counter (object, HeightHandle)
      if height and (height > self.maxHeight) then self.maxHeight = height end
   end
end,

calc = function (self, object)
   local result = 0
   local value = dmz.object.counter (object, HeightHandle)
   if value and (self.maxHeight > 0) then result = value / self.maxHeight end
   return result
end,

}

local function weigh_object (self, object)
   local value = 1
   for _, weight in pairs (self.weightList) do
      value = value * weight.calc (self, object)
   end
   dmz.object.scalar (object, WeightHandle, value)
end

local function log_defender_term (object)
   local result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma
   if not_zero (result) then
      result = object.cost / result
      if not_zero (object.gamma) and not_zero (result) then
         result = (object.cost / object.gamma) * math.log (result)
      else result = 0
      end
   else result = 0
   end
   return result
end

local function log_defender (object)
   local result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma
   if not_zero (result) and not_zero (object.cost) then
      result = math.log (object.cost / result)
   else result = 0
   end
   return result
end

local function allocate_prevention_budget (handleList, budget, maxBudget, vinf)
   if dmz.math.is_zero (budget) then
      for handle in pairs (handleList) do
         dmz.object.scalar (handle, PreventionAllocationHandle, 0)
         dmz.object.scalar (handle, GammaHandle, 0)
      end
   else
      local objectList = {}
      for handle in pairs (handleList) do
         local object = { handle = handle }
         object.vul = dmz.object.scalar (handle, VulnerabilityHandle)
         if not object.vul or (object.vul <= 0) then object.vul = 1 end
         object.gamma = -math.log (vinf / object.vul)
         if object.gamma < 0 then object.gamma = 0 end
         dmz.object.scalar (handle, GammaHandle, object.gamma)
         object.cost = dmz.object.scalar (handle, PreventionCostHandle)
         if not object.cost then object.cost = 0 end
         object.weight = dmz.object.scalar (handle, WeightHandle)
         if not object.weight then object.weight = 0 end
         object.threat = dmz.object.scalar (handle, ThreatHandle)
         if not object.threat then object.threat = 0 end
         object.consequence = dmz.object.scalar (handle, ConsequenceHandle)
         if not object.consequence then object.consequence = 0 end
         object.allocation = 0
         objectList[#objectList + 1] = object
      end
      local A = 0
      local B = 0
      for _, object in ipairs (objectList) do
         A = A + log_defender_term (object)
         if not_zero (object.gamma) then
            B = B + (object.cost / object.gamma)
         end
      end
      local totalAllocation = 0
      local logLamda = 0
      if not_zero (B) then logLamda = (-budget - A) / B end
      for _, object in ipairs (objectList) do
         local A = 0
         if not_zero (object.gamma) then A = object.cost / object.gamma end
         local B = log_defender (object)
         object.allocation = -A * (logLamda + B)
         if object.allocation < 0 then object.allocation = 0 end
         if object.allocation > object.cost then object.allocation = object.cost end
         totalAllocation = totalAllocation + object.allocation
      end
      local scale = 1
      if totalAllocation < budget then
         local size = #(objectList)
         local count = 1
         local remainder = budget - totalAllocation
         while not_zero (remainder) and (count <= size) do
            local object = objectList[count]
            local max = object.cost - object.allocation
            if max > remainder then
               max = remainder
               remainder = 0
            else
               remainder = remainder - max
            end
            object.allocation = object.allocation + max
            count = count + 1
         end
      else scale = budget / totalAllocation
      end
      totalAllocation = 0
      for _, object in ipairs (objectList) do
         object.allocation = object.allocation * scale
         totalAllocation = totalAllocation + object.allocation
         dmz.object.scalar (object.handle, PreventionAllocationHandle, object.allocation)
      end
   end
end

local function rank_object (self, object)
   local result = dmz.object.scalar (object, WeightHandle)
   if not result then result = 1 end
   if self.objective then
      result = result * self.objective (self, object)
   end
   dmz.object.scalar (object, WeightAndObjectiveHandle, result)
   return result
end

local function receive_rank (self)
   self.visible = true
   local list = {}
   for _, weight in pairs (self.weightList) do
      weight.setup (self)
   end
   for object in pairs (self.objects) do
      weigh_object (self, object)
   end
   allocate_prevention_budget (
      self.objects,
      self.preventionBudget,
      self.maxPreventionBudget,
      self.vinf)
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
         calc_risk_reduced (handle)
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
      if self.rankLimit and count <= self.rankLimit and object.rank > 0 then
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
         dmz.object.text (handle, LabelHandle, "")
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

local function update_objective_graph (self)
   if self.updateGraph then 
      local max = 0
      local budgets = {}
      local list = {}
      for ix = 0, self.barCount do
         local budget = self.maxPreventionBudget * (ix / self.barCount)
         budgets[ix] = budget
         for _, weight in pairs (self.weightList) do
            weight.setup (self)
         end
         for object in pairs (self.objects) do
            weigh_object (self, object)
         end
         allocate_prevention_budget (
            self.objects,
            budget,
            self.maxPreventionBudget,
            self.vinf)
         local result = 0
         if self.objective then
            for object in pairs (self.objects) do
               calc_risk_reduced (object)
               result = result + self.objective (self, object)
            end
         end
         if result > max then max = result end
         list[ix] = result
      end
      if max > 0 then
         for ix = 0, self.barCount do
            list[ix] = math.ceil (list[ix] / max * 100)
         end
      else 
         for ix = 0, self.barCount do
            list[ix] = 0
         end
      end
      for ix = 0, self.barCount do
--self.log:error (ix, list[ix])
         dmz.object.counter (self.bars[ix], "NA_Objective_Bar_Value", list[ix])
         dmz.object.text (
            self.bars[ix],
            "NA_Objective_Bar_Label",
            "$" .. string.format ("%d", math.floor (budgets[ix])))
      end
   end
end

local function receive_simulator (self, message, data)
   if dmz.data.is_a (data) then
      if data:lookup_boolean ("Boolean", 1) then self:do_rank ()
      else receive_hide (self)
      end
   end
end

local function receive_prevention_budget (self, message, data)
   if dmz.data.is_a (data) then
      self.preventionBudget = data:lookup_number ("Budget", 1)
      if not self.preventionBudget then self.preventionBudget = 0 end
      self.maxPreventionBudget = data:lookup_number ("Budget", 2)
      if not self.maxPreventionBudget then self.maxPreventionBudget = 0 end
      if self.visible then self:do_rank () end
   end
end

local function receive_response_budget (self, message, data)
   if dmz.data.is_a (data) then
      self.responseBudget = data:lookup_number ("Budget", 1)
      self.maxResponseBudget = data:lookup_number ("Budget", 2)
      if self.visible then self:do_rank () end
   end
end

local function receive_vinfinity (self, msg, data)
   if dmz.data.is_a (data) then
      self.vinf = data:lookup_number ("value", 1)
      if not self.vinf then self.vinf = 0.05 end
      if self.visible then self:do_rank () end
   end   
end

local function create_object (self, handle, objType, locality)
   if objType then
      if objType:is_of_type (NodeType) or objType:is_of_type (NodeLinkType) then
         self.objects[handle] = true
         if self.visible and self.objects[handle] then self:do_rank () end
         self:do_graph ()
      end
   end
end

local function update_object_scalar (self, handle, attr, value)
   if self.visible and self.objects[handle] then self:do_rank () end
   calc_risk_initial (handle)
   self:do_graph ()
end

local function update_simulator_flag (self, handle, attr, value)
   if value then
      if attr == WeightDegreesHandle then
         self.weightList[WeightDegreesHandle] = weight_degrees
      elseif attr == WeightBetweennessHandle then
         self.weightList[WeightBetweennessHandle] = weight_betweenness
      elseif attr == WeightHeightHandle then
         self.weightList[WeightHeightHandle] = weight_height
      elseif attr == ObjectiveNoneHandle then
         self.objective = calc_objective_none
      elseif attr == ObjectiveRiskHandle then
         self.objective = calc_objective_risk
      elseif attr == ObjectiveContagiousHandle then
         self.objective = calc_objective_contagiousness
      elseif attr == ObjectiveTxVHandle then
         self.objective = calc_objective_txv
      elseif attr == ObjectiveThreatHandle then
         self.objective = calc_objective_threat
      elseif attr == ObjectiveVulnerabilityHandle then
         self.objective = calc_objective_vulnerability
      elseif attr == ObjectiveConsequenceHandle then
         self.objective = calc_objective_consequence
      end
      self:do_graph ()
   elseif self.weightList[attr] then
      self.weightList[attr] = nil
      self:do_graph ()
   end
   if self.visible then self:do_rank () end
end

local function destroy_object (self, handle)
   local updateRank = false
   if self.visible and self.objects[handle] then updateRank = true end
   self.objects[handle] = nil
   if updateRank then self:do_rank () end
   self:do_graph ()
end

local function link_objects (self, link, attr, super, sub)
   if self.visible and self.objects[super] then self:do_rank () end
   self:do_graph ()
end

local function unlink_objects (self, link, attr, super, sub)
   if self.visible and self.objects[super] then self:do_rank () end
   self:do_graph ()
end

local function update_link_object (self, link, attr, super, sub, object)
   if self.visible and object and self.objects[object] then self:do_rank () end
   self:do_graph ()
end

local function work_func (self)
   if self.doGraphCount > 1 then self.doGraphCount = self.doGraphCount - 1
   elseif self.doGraphCount == 1 then
      update_objective_graph (self)
      self.doGraphCount = 0
   end
   if self.doRankCount > 1 then self.doRankCount = self.doRankCount - 1
   elseif self.doRankCount == 1 then
--local start = os.clock ()
      receive_rank (self)
--self.log:info ("Time to run: " .. tostring (os.clock () - start))
      self.doRankCount = 0;
   end
end

function new (config, name)

   local self = {
      visible = false,
      rankLimit = config:to_number ("rank.limit", 9),
      log = dmz.log.new ("lua." .. name),
      simulatorMessage =
         config:to_message ("simulator-message.name", "NASimulatorMessage"),
      preventionBudgetMessage =
         config:to_message ("message.prevention-budget.name", "PreventionBudgetMessage"),
      responseBudgetMessage =
         config:to_message ("message.response-budget.name", "ResponseBudgetMessage"),
      vinfinityMessage =
         config:to_message ("v-infinity-message.name", "NAVulnerabilityInfinityMessage"),
      updateObjectivGraphMessage =
         config:to_message (
            "update-objective-graph-message.name",
            "NA_Objective_Graph_Visible_Message"),
      msgObs = dmz.message_observer.new (name),
      objObs = dmz.object_observer.new (),
      timeSlice = dmz.time_slice.new (),
      doRankCount = 0,
      do_rank = function (self) self.doRankCount = 2 end,
      doGraphCount = 0,
      do_graph = function (self) self.doGraphCount = 2 end,
      objects = {},
      objective = calc_objective_none,
      weightList = {},
      preventionBudget = 0,
      maxPreventionBudget = 0,
      responseBudget = 0,
      maxResponseBudget = 0,
      vinf = 0.05,
      barCount = 10,
      bars = {},
   }

   for ix = 0, self.barCount do
      self.bars[ix] = dmz.object.create ("na_objective_bar")
      dmz.object.counter (self.bars[ix], "NA_Objective_Bar_Number", ix)
      dmz.object.counter (self.bars[ix], "NA_Objective_Bar_Value", 0)
      dmz.object.activate (self.bars[ix])
      dmz.object.set_temporary (self.bars[ix])
   end

   self.log:info ("Creating plugin: " .. name)

   self.msgObs:register (self.simulatorMessage, receive_simulator, self)
   self.msgObs:register (self.preventionBudgetMessage, receive_prevention_budget, self)
   self.msgObs:register (self.responseBudgetMessage, receive_response_budget, self)
   self.msgObs:register (self.vinfinityMessage, receive_vinfinity, self)
   self.msgObs:register (
      self.updateObjectivGraphMessage,
      function (self, msg, data)
         if dmz.data.is_a (data) then
            self.updateGraph = data:lookup_boolean ("Boolean", 1)
            update_objective_graph (self)
         end
      end,
      self)

   local cb = { create_object = create_object, destroy_object = destroy_object }
   self.objObs:register (nil, cb, self)

   cb = { update_object_scalar = update_object_scalar, }
   self.objObs:register (ThreatHandle, cb, self)
   self.objObs:register (VulnerabilityHandle, cb, self)
   self.objObs:register (PreventionCostHandle, cb, self)
   self.objObs:register (ConsequenceHandle, cb, self)
   self.objObs:register (DegreeHandle, cb, self)

   cb = { update_object_flag = update_simulator_flag, }
   self.objObs:register (WeightDegreesHandle, cb, self)
   self.objObs:register (WeightBetweennessHandle, cb, self)
   self.objObs:register (WeightHeightHandle, cb, self)
   self.objObs:register (ObjectiveNoneHandle, cb, self)
   self.objObs:register (ObjectiveRiskHandle, cb, self)
   self.objObs:register (ObjectiveContagiousHandle, cb, self)
   self.objObs:register (ObjectiveTxVHandle, cb, self)
   self.objObs:register (ObjectiveThreatHandle, cb, self)
   self.objObs:register (ObjectiveVulnerabilityHandle, cb, self)
   self.objObs:register (ObjectiveConsequenceHandle, cb, self)

   cb = {
      link_objects = link_objects,
      unlink_objects = unlink_objects,
      update_link_object = update_link_object,
   }

   self.objObs:register (LinkHandle, cb, self)

   self.tsHandle = self.timeSlice:create (work_func, self, self.name)

   return self
end
