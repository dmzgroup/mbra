var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , defs: require("dmz/runtime/definitions")
      , data: require("dmz/runtime/data")
      , mask: require("dmz/types/mask")
      , message: require("dmz/runtime/messaging")
      , util: require("dmz/types/util")
      , time: require("dmz/runtime/time")
      }
   , ObjectiveBarValueHandle = dmz.defs.createNamedHandle("NA_Objective_Bar_Value")
   , ObjectiveBarLabel = dmz.defs.createNamedHandle("NA_Objective_Bar_Label")
   , NodeType = dmz.objectType.lookup("na_node")
   , NodeLinkType = dmz.objectType.lookup("na_link_attribute")
   , SimType = dmz.objectType.lookup("na_simulator")
   , LabelHandle = dmz.defs.createNamedHandle("NA_Node_Objective_Label")
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")
   , LinkFlowHandle = dmz.defs.createNamedHandle("NA_Link_Flow")
   , ThreatHandle = dmz.defs.createNamedHandle("NA_Node_Threat")
   , VulnerabilityHandle = dmz.defs.createNamedHandle("NA_Node_Vulnerability")
   , VulnerabilityReducedHandle = dmz.defs.createNamedHandle(
         "NA_Node_Vulnerability_Reduced")
   , PreventionCostHandle = dmz.defs.createNamedHandle("NA_Node_Prevention_Cost")
   , PreventionAllocationHandle = dmz.defs.createNamedHandle(
         "NA_Node_Prevention_Allocation")
   , ConsequenceHandle = dmz.defs.createNamedHandle("NA_Node_Consequence")
   , RiskInitialHandle = dmz.defs.createNamedHandle("NA_Node_Risk_Initial")
   , RiskReducedHandle = dmz.defs.createNamedHandle("NA_Node_Risk_Reduced")
   , WeightHandle = dmz.defs.createNamedHandle("NA_Node_Weight")
   , WeightAndObjectiveHandle = dmz.defs.createNamedHandle(
         "NA_Node_Weight_And_Objective")
   , GammaHandle = dmz.defs.createNamedHandle("NA_Node_Gamma")
   , RankHandle = dmz.defs.createNamedHandle("NA_Node_Rank")
   , DegreeHandle = dmz.defs.createNamedHandle("NA_Node_Degrees")
   , BetweennessHandle = dmz.defs.createNamedHandle("NA_Node_Betweenness")
   , HeightHandle = dmz.defs.createNamedHandle("NA_Node_Height")
   , ContagiousHandle = dmz.defs.createNamedHandle("NA_Node_Contagiousness")
   , OverlayState = dmz.defs.lookupState("NA_Node_Overlay")
   , ForwardState = dmz.defs.lookupState("NA_Flow_Forward")
   , ReverseState = dmz.defs.lookupState("NA_Flow_Reverse")
   , FlowStateMask = ForwardState.or(ReverseState)

   , WeightDegreesHandle = dmz.defs.createNamedHandle("NA_Weight_Degrees")
   , WeightBetweennessHandle = dmz.defs.createNamedHandle("NA_Weight_Betweenness")
   , WeightHeightHandle = dmz.defs.createNamedHandle("NA_Weight_Height")
   , WeightContagiousHandle = dmz.defs.createNamedHandle("NA_Weight_Contagiousness")

   , ObjectiveNoneHandle = dmz.defs.createNamedHandle("NA_Objective_None")
   , ObjectiveRiskHandle = dmz.defs.createNamedHandle("NA_Objective_Risk")
   , ObjectiveTxVHandle = dmz.defs.createNamedHandle("NA_Objective_TxV")
   , ObjectiveThreatHandle = dmz.defs.createNamedHandle("NA_Objective_Threat")
   , ObjectiveVulnerabilityHandle = dmz.defs.createNamedHandle(
         "NA_Objective_Vulnerability")
   , simulatorMessage = dmz.message.create(
      self.config.string("simulator-message.name", "NASimulatorMessage"))
   , preventionBudgetMessage = dmz.message.create(
      self.config.string("message.prevention-budget.name",
                             "PreventionBudgetMessage"))
   , responseBudgetMessage = dmz.message.create(
      self.config.string("message.response-budget.name", "ResponseBudgetMessage"))
   , vinfinityMessage = dmz.message.create(
      self.config.string("v-infinity-message.name", "NAVulnerabilityInfinityMessage"))
   , updateObjectiveGraphMessage = dmz.message.create(
      self.config.string("update-objective-graph-message.name",
                          "NA_Objective_Graph_Visible_Message"))
   , updateSumsMessage = dmz.message.create(
      self.config.string("message.sums.name", "NA_Objective_Sums_Message"))
   , doRankCount = 0
   , do_rank = function () {
      doRankCount = 2;
   }
   , doGraphCount = 0
   , do_graph = function () {
      doGraphCount = 2;
   }
   , objects = []
   , calc_objective_none = function (object) {
      dmz.object.text(object, LabelHandle, "");
      return [1, 1];
   }
   , objective = calc_objective_none
   , weightList = []
   , preventionBudget = 0
   , maxPreventionBudget = 0
   , responseBudget = 0
   , maxResponseBudget = 0
   , maxDegrees = 0
   , maxHeight = 0
   , maxBetweenness = 0
   , maxContagious = 0
   , vinf = 0.05
   , barCount = 10
   , bars = []
   , visible = false
   , reducedSum = 0
   , origSum = 0
   , updateGraph = false
   , rankLimit = self.config.number("rank.limit", 9)

   , update_objective_graph
   , weigh_object
   , allocate_prevention_budget
   , calc_risk_reduced
   , receive_rank
   ;

(function () {
   var ix;
   for (ix = 0; ix <= barCount; ix += 1) {
      bars[ix] = dmz.object.create("na_objective_bar");
      dmz.object.counter(bars[ix], "NA_Objective_Bar_Number", ix);
      dmz.object.counter(bars[ix], "NA_Objective_Bar_Value", 0);
      dmz.object.activate(bars[ix]);
   }
}());

var work_func = function () {
   if (doGraphCount > 1) {
      doGraphCount -= 1;
   }
   else if (doGraphCount == 1) {
      update_objective_graph();
      doGraphCount = 0;
   }
   if (doRankCount > 1) {
      doRankCount -= 1;
   }
   else if (doRankCount == 1) {
      receive_rank();
      doRankCount = 0;
   }
};

var timeSlice = dmz.time.setRepeatingTimer(self, work_func);

var not_zero = function (value) {
   return dmz.util.isNotZero(value);
};

var calc_risk_initial = function (object) {
   var Threat = dmz.object.scalar(object, ThreatHandle)
     , Vulnerability = dmz.object.scalar(object, VulnerabilityHandle)
     , Consequence = dmz.object.scalar(object, ConsequenceHandle)
     ;
   if (Threat && Vulnerability && Consequence) {
      dmz.object.scalar(
         object,
         RiskInitialHandle,
         Threat * Vulnerability * Consequence);
   }
   else {
      dmz.object.scalar(object, RiskInitialHandle, 0);
   }
};

var calc_vulnerability = function (object) {
   var result = 0
     , Allocation = dmz.object.scalar(object, PreventionAllocationHandle)
     , Vulnerability = dmz.object.scalar(object, VulnerabilityHandle)
     , Cost
     , Gamma
     ;
   if (Vulnerability) {
      result = Vulnerability;
   }
   Cost = dmz.object.scalar(object, PreventionCostHandle);
   Gamma = dmz.object.scalar(object, GammaHandle);
   if (Gamma && not_zero(Gamma) && Vulnerability && (Vulnerability > 0) &&
         Cost && (Cost > 0) && not_zero(Cost) &&
         Allocation && not_zero(Allocation)) {
      result = Vulnerability * Math.exp(-Gamma * Allocation / Cost);
   }
   dmz.object.scalar(object, VulnerabilityReducedHandle, result);
   return result;
};

var calc_risk_reduced = function (object) {
   var result = 0
     , Threat = dmz.object.scalar(object, ThreatHandle)
     , Vulnerability = calc_vulnerability(object)
     , Consequence = dmz.object.scalar(object, ConsequenceHandle)
     ;
   if (Threat && Vulnerability && Consequence) {
      result = Threat * Vulnerability * Consequence;
   }
   dmz.object.scalar(object, RiskReducedHandle, result);
   return result;
};

var format_result = function (value) {
   return "" + Math.round(value * 100) + "%";
};

var calc_objective_risk = function (object) {
   var result = dmz.object.scalar(object, RiskReducedHandle)
     , orig = dmz.object.scalar(object, RiskInitialHandle)
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle, "Risk = " + result.toFixed(2));
   }
   return [result, orig];
};

var calc_objective_contagiousness = function (object) {
   var result = 0
     , orig = 0
     , Threat = dmz.object.scalar(object, ThreatHandle)
     , VulnerabilityReduced = dmz.object.scalar(object, VulnerabilityReducedHandle)
     , Vulnerability = dmz.object.scalar(object, VulnerabilityHandle)
     , Degrees = dmz.object.scalar(object, DegreeHandle)
     ;
   if (Threat && VulnerabilityReduced && Degrees) {
      result = Threat * VulnerabilityReduced * Degrees;
   }
   if (Threat && Vulnerability && Degrees) {
      orig = Threat * Vulnerability * Degrees;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle, "Contagiousness = " + result.toFixed(2));
   }
   return [result, orig];
};

var calc_objective_txv = function (object) {
   var result = 0
     , orig = 0
     , Threat = dmz.object.scalar(object, ThreatHandle)
     , VulnerabilityReduced = dmz.object.scalar(object, VulnerabilityReducedHandle)
     , Vulnerability = dmz.object.scalar(object, VulnerabilityHandle)
     ;
   if (Threat && VulnerabilityReduced) {
      result = Threat * VulnerabilityReduced;
   }
   if (Threat && Vulnerability) {
      orig = Threat * Vulnerability;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle, "T x V = " + format_result(result));
   }
   return [result, orig];
};

var calc_objective_threat = function (object) {
   var result = dmz.object.scalar(object, ThreatHandle)
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle, "Threat = " + format_result(result));
   }
   return [result, result];
};

var calc_objective_vulnerability = function (object) {
   var result = dmz.object.scalar(object, VulnerabilityReducedHandle)
     , orig = dmz.object.scalar(object, VulnerabilityHandle)
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle,
                       "Vulnerability = " + format_result(result));
   }
   return [result, orig];
};

var calc_objective_consequence = function (object) {
   var result = dmz.object.scalar(object, ConsequenceHandle)
     , str
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      str = "Consequence = $" + result.toFixed(2);
      dmz.object.text(object, LabelHandle, str);
   }
   return [result, result];
};

var weight_degrees = {

   setup: function () {

      maxDegrees = 0;
      Object.keys(objects).forEach(function (key) {
         var value = dmz.object.scalar(objects[key], DegreeHandle);
         if (value && (value > maxDegrees)) {
            maxDegrees = value;
         }
      });
   },
   
   calc: function (object) {
      var result = 0
        , value = dmz.object.scalar(object, DegreeHandle)
        ;
      if (value && (maxDegrees > 0)) {
         result = value / maxDegrees;
      }
      return result;
   }

};

var EmptyMask = dmz.mask.create();

var link_reachable = function (link, flowState) {
   var result = true
     , obj = dmz.object.linkAttributeObject(link)
     , state
     ;
   if (obj) {
      state = dmz.object.state(obj, LinkFlowHandle);
      if (state && (!state.and(FlowStateMask).equal(EmptyMask))) {
         if (!state.contains(flowState)) {
            result = false;
         }
      }
   }
   return result;
};

var all_links_flow = function (superList, subList, flowState) {
   var result = true
     , subBool = true
     , link
     , obj
     , state
     ;
   if (superList && subList) {
      Object.keys(superList).forEach(function (superkey) {
         if (!result) {
            return;
         }
         Object.keys(subList).forEach(function (subkey) {
            if (!subList[subkey]) {
               return;
            }
            link = dmz.object.linkHandle(LinkHandle, superList[superkey],
                                                    subList[subkey]);
            obj = null;
            if (link) {
               obj = dmz.object.linkAttributeObject(link);
            }
            else {
               self.log.error("No link found");
            }
            if (obj) {
               state = dmz.object.state(obj, LinkFlowHandle);
               if (state.bool()) {
                  if (!state.and(FlowStateMask).equal(flowState)) {
                     result = false;
                     subBool = false;
                  }
               }
               else {
                  result = false;
                  subBool = false;
               }
            }
            else {
               result = false;
               subBool = false;
            }
         });
      });
   }
   return result;
};

var is_sink = function (object) {
   var sinkObjList = []
     , result = false
     , sub = dmz.object.subLinks(object, LinkHandle)
     , superLink = dmz.object.superLinks(object, LinkHandle)
     ;
   sinkObjList[0] = object;
   if (sub || superLink) {
      if (all_links_flow(sinkObjList, sub, ReverseState) &&
          all_links_flow(superLink, sinkObjList, ForwardState)) {
         result = true;
      }
   }
   return result;
};

var add_to_node_betweenness_counter = function (object) {
   var value = dmz.object.addToCounter(object, BetweennessHandle);
   if (value > maxBetweenness) {
      maxBetweenness = value;
   }
};

var add_to_link_betweenness_counter = function (link) {
   var linkObj = dmz.object.linkAttributeObject(link)
     , value
     ;
   if (linkObj) {
      value = dmz.object.addToCounter(linkObj, BetweennessHandle);
      if (value > maxBetweenness) {
         maxBetweenness = value;
      }
   }
};

var find_betweenness = function (current, target, visited) {
   var found = false
     , list = []
     , item
     , subs
     , link
     , supers
     , place
     ;
   Object.keys(current).forEach(function (key) {
      visited[current[key].object] = true;
   });
   Object.keys(current).forEach(function (key) {
      item = current[key];
      subs = dmz.object.subLinks(item.object, LinkHandle);
      if (subs) {
         Object.keys(subs).forEach(function (sub) {
            link = dmz.object.linkHandle(LinkHandle, item.object, subs[sub]);
            if (link_reachable(link, ForwardState)) {
               if (subs[sub] == target) {
                  found = true;
                  item.found = true;
                  add_to_node_betweenness_counter(target);
                  add_to_link_betweenness_counter(link);
               }
               else if (!visited[subs[sub]]) {
                  list.push({ object: subs[sub], link: link, parent: item });
               }
            }
         });
      }
      supers = dmz.object.superLinks(item.object, LinkHandle);
      if (supers) {
         Object.keys(supers).forEach(function (superLink) {
            link = dmz.object.linkHandle(LinkHandle, supers[superLink], item.object);
            if (link_reachable(link, ReverseState)) {
               if (supers[superLink] == target) {
                  found = true;
                  item.found = true;
                  add_to_node_betweenness_counter(target);
                  add_to_link_betweenness_counter(link);
               }
               else if (!visited[supers[superLink]]) {
                  list.push({ object: supers[superLink], link: link, parent: item});
               }
            }
         });
      }
   });
   if (!found && list.length > 0) {
      find_betweenness(list, target, visited);
      for (place = list.length - 1; place >= 0; place -= 1) {
         item = list[place];
         if (item.found) {
            add_to_node_betweenness_counter(item.object);
            add_to_link_betweenness_counter(item.link);
            item.parent.found = true;
         }
      }
   }
};

var weight_betweenness = {

   setup: function () {
      var root
        , target
        , list
        , visited
        ;
      maxBetweenness = 0;
      Object.keys(objects).forEach(function (key) {
         dmz.object.counter(objects[key], BetweennessHandle, 0);
      });
      Object.keys(objects).forEach(function (key) {
         root = objects[key];
         if (dmz.object.type(root).isOfType(NodeType)) {
            Object.keys(objects).forEach(function (index) {
               target = objects[index];
               if (root != target && dmz.object.type(target).isOfType(NodeType)) {
                  list = [{object: root}];
                  visited = [];
                  find_betweenness(list, target, visited);
                  if (list[0]) {
                     add_to_node_betweenness_counter(root);
                  }
               }
            });
         }
      });
   },

   calc: function (object) {
      var result = 0
        , value = dmz.object.counter(object, BetweennessHandle)
        ;
      if (value && (maxBetweenness > 0)) {
         result = value / maxBetweenness;
      }
      return result;
   }

};

var update_height = function (obj, level, visited) {
   var result = false
     , value
     ;
   if (dmz.object.isLink(obj)) {
      obj = dmz.object.linkAttributeObject(obj);
   }
   if (obj && !visited[obj]) {
      result = true;
      visited[obj] = true;
      value = dmz.object.counter(obj, HeightHandle);
      if (!value) {
         value = 0;
      }
      if (level > value) {
         dmz.object.counter(obj, HeightHandle, level);
      }
   }
   return result;
};

var find_height = function (sink) {
   var queue = []
     , visited = []
     , node
     , height
     , subList
     , superList
     , link
     ;
   queue.push(sink);
   update_height(sink, 1, visited);
   while (queue.length > 0)  {
      node = queue.shift();
      height = dmz.object.counter(node, HeightHandle);
      if (!height) {
         self.log.error("No height found for.", node);
         height = 0;
      }
      subList = dmz.object.subLinks(node, LinkHandle);
      if (subList) {
         Object.keys(subList).forEach(function (key) {
            link = dmz.object.linkHandle(LinkHandle, node, subList[key]);
            if (link) {
               if (link_reachable(link, ReverseState)) {
                  update_height(link, height + 1, visited);
                  if (update_height(subList[key], height + 2, visited)) {
                     queue.push(subList[key]);
                  }
               }
            }
         });
      }
      superList = dmz.object.superLinks(node, LinkHandle);
      if (superList) {
         Object.keys(superList).forEach(function (key) {
            link = dmz.object.linkHandle(LinkHandle, superList[key], node);
            if (link) {
               if (link_reachable(link, ForwardState)) {
                  update_height(link, height + 1, visited);
                  if (update_height(superList[key], height + 2, visited)) {
                     queue.push(superList[key]);
                  }
               }
            }
         });
      }
   }
};

var weight_height = {

   setup: function () {
      var sinkFound
        , height
        ;
      maxHeight = 0;
      Object.keys(objects).forEach(function (key) {
         dmz.object.counter(objects[key], HeightHandle, 0);
      });

      sinkFound = false;
      Object.keys(objects).forEach(function (key) {
         if (is_sink(objects[key])) {
            sinkFound = true;
            find_height(objects[key]);
         }
      });
      if (sinkFound) {
         Object.keys(objects).forEach(function (key) {
            height = dmz.object.counter(objects[key], HeightHandle);
            if (height && (height > maxHeight)) {
               maxHeight = height;
            }
         });
      }
      else {
         maxHeight = 1;
         Object.keys(objects).forEach(function (key) {
            dmz.object.counter(objects[key], HeightHandle, 0);
         });
      }
   },

   calc: function (object) {
      var result = 0
        , value = dmz.object.counter(object, HeightHandle)
        ;
      if (value && (maxHeight > 0)) {
         result = value / maxHeight;
      }
      return result;
   }
};

var weight_contagious = {

   setup: function () {
      maxContagious = 0;
      Object.keys(objects).forEach(function (key) {
         var degree = dmz.object.scalar(objects[key], DegreeHandle)
           , threat = dmz.object.scalar(objects[key], ThreatHandle)
           , vuln = dmz.object.scalar(objects[key], VulnerabilityHandle)
           , value = degree * threat * vuln;
         if (degree > 0 && threat > 0 && vuln > 0) {
            if (value && (value > maxContagious)) {
               maxContagious = value;
            }
            dmz.object.scalar(objects[key], ContagiousHandle, value);
         }
         else {
            dmz.object.scalar(objects[key], ContagiousHandle, 0);
         }
      });
   },

   calc: function (object) {
      var result = 0
        , value = dmz.object.scalar(objects[object], ContagiousHandle)
      if (value > 0 && (maxContagious > 0)) {
         result = value / maxContagious;
      }
      return result;
   }
};

var weigh_object = function (object) {
   var value = 1;
   Object.keys(weightList).forEach(function (key) {
      value *= weightList[key].calc(object);
   });
   dmz.object.scalar(object, WeightHandle, value);
};

var log_defender_term = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma;
   if (not_zero(result)) {
      result = object.cost / result;
      if (not_zero(object.gamma) && not_zero(result)) {
         result = (object.cost / object.gamma) * Math.log(result);
      }
      else {
         result = 0;
      }
   }
   else {
      result = 0;
   }
   return result;
};

var log_defender = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma;
   if (not_zero(result) && not_zero(object.cost)) {
      result = Math.log(object.cost / result);
   }
   else {
      result = 0;
   }
   return result;
};

var allocate_prevention_budget = function (handleList, budget, maxBudget, vinf) {
   var objectList
     , object
     , A
     , B
     , totalAllocation
     , logLamda
     , scale
     , size
     , count
     , remainder
     , max
     ;
   if (dmz.util.isZero(budget)) {
      Object.keys(handleList).forEach(function (key) {
         dmz.object.scalar(handleList[key], PreventionAllocationHandle, 0);
         dmz.object.scalar(handleList[key], GammaHandle, 0);
      });
   }
   else {
      objectList = [];
      Object.keys(handleList).forEach(function (key) {
         object = { handle: handleList[key] };
         object.vul = dmz.object.scalar(handleList[key], VulnerabilityHandle);
         if (!object.vul || (object.vul <= 0)) {
            object.vul = 1;
         }
         object.gamma = -Math.log(vinf / object.vul);
         if (object.gamma < 0) {
            object.gamma = 0;
         }
         dmz.object.scalar(handleList[key], GammaHandle, object.gamma);
         object.cost = dmz.object.scalar(handleList[key], PreventionCostHandle);
         if (!object.cost) {
            object.cost = 0;
         }
         object.weight = dmz.object.scalar(handleList[key], WeightHandle);
         if (!object.weight) {
            object.weight = 0;
         }
         object.threat = dmz.object.scalar(handleList[key], ThreatHandle);
         if (!object.threat) {
            object.threat = 0;
         }
         object.consequence = dmz.object.scalar(handleList[key], ConsequenceHandle);
         if (!object.consequence) {
            object.consequence = 0;
         }
         object.allocation = 0;
         objectList.push(object);
      });
      A = 0;
      B = 0;
      Object.keys(objectList).forEach(function (key) {
         A = A + log_defender_term(objectList[key]);
         if (not_zero(objectList[key].gamma)) {
            B = B + (objectList[key].cost / objectList[key].gamma);
         }
      });
      totalAllocation = 0;
      logLamda = 0;
      if (not_zero(B)) {
         logLamda = (-budget - A) / B;
      }
      Object.keys(objectList).forEach(function (key) {
         object = objectList[key];
         A = 0;
         if (not_zero(objectList[key].gamma)) {
            A = object.cost / object.gamma;
         }
         B = log_defender(object);
         object.allocation = -A * (logLamda + B);
         if (object.allocation < 0) {
            object.allocation = 0;
         }
         if (object.allocation > object.cost) {
            object.allocation = object.cost;
         }
         totalAllocation += object.allocation;
      });
      scale = 1;
      if (totalAllocation < budget) {
         size = objectList.length - 1;
         count = 0;
         remainder = budget - totalAllocation;
         while (not_zero(remainder) && (count <= size))  {
            object = objectList[count];
            max = object.cost - object.allocation;
            if (max > remainder) {
               max = remainder;
               remainder = 0;
            }
            else {
               remainder = remainder - max;
            }
            object.allocation += max;
            count += 1;
         }
      }
      else {
         scale = budget / totalAllocation;
      }
      totalAllocation = 0;
      Object.keys(objectList).forEach(function (key) {
         object = objectList[key];
         object.allocation = object.allocation * scale;
         totalAllocation = totalAllocation + object.allocation;
         dmz.object.scalar(object.handle, PreventionAllocationHandle,
                            object.allocation);
      });
   }
};

var rank_object = function (object) {
   var result = dmz.object.scalar(object, WeightHandle)
     , objectiveArray
     , reduced
     , Orig
     ;
   if (!result) {
      result = 1;
   }
   if (objective) {
      objectiveArray = objective(object);
      reduced = objectiveArray[0];
      Orig = objectiveArray[1];
      reducedSum += reduced;
      origSum += Orig;
      result *= reduced;
   }
   dmz.object.scalar(object, WeightAndObjectiveHandle, result);
   return result;
};

var receive_rank = function () {
   var list = []
     , state
     , object
     , data
     , count
     , lastRank
     ;
   visible = true;
   Object.keys(weightList).forEach(function (key) {
      weightList[key].setup();
   });
   Object.keys(objects).forEach(function (key) {
      weigh_object(objects[key]);
   });
   allocate_prevention_budget(objects, preventionBudget, maxPreventionBudget, vinf);
   reducedSum = 0;
   origSum = 0;
   Object.keys(objects).forEach(function (key) {

      if (dmz.object.isObject(objects[key])) {
         state = dmz.object.state(objects[key]);
         if (state) {
            state = state.unset(OverlayState);
            dmz.object.state(objects[key], null, state);
         }
         if (dmz.object.text(objects[key], RankHandle)) {
            dmz.object.text.remove(objects[key], RankHandle);
         }
         calc_risk_reduced(objects[key]);
         object = { handle: objects[key] };
         object.rank = rank_object(object.handle);
         list.push(object);
      }
   });
   data = dmz.data.create();
   data.number("Float64", 0, reducedSum);
   data.number("Float64", 1, origSum);
   updateSumsMessage.send(data);
   list.sort(function (obj1, obj2) {
      return obj2.rank - obj1.rank;
   });
   count = 1;
   lastRank = null;
   Object.keys(list).forEach(function (index) {
      object = list[index];
      if (!lastRank) {
         lastRank = object.rank;
      }
      else if (lastRank > object.rank) {
         count += 1;
         lastRank = object.rank;
      }
      dmz.object.text(object.handle, RankHandle, count);
      if (rankLimit && count <= rankLimit && object.rank > 0) {
         state = dmz.object.state(object.handle);
         if (!state) {
            state = dmz.mask.create();
         }
         state = state.or(OverlayState);
         dmz.object.state(object.handle, null, state);
      }
   });
};

var receive_hide = function () {
   var handle
     , state
     ;
   visible = false;
   Object.keys(objects).forEach(function (key) {
      handle = objects[key];
      if (dmz.object.isObject(handle)) {
         dmz.object.text(handle, LabelHandle, "");
         state = dmz.object.state(handle);
         if (state) {
            state = state.unset(OverlayState);
            dmz.object.state(handle, null, state);
         }
         if (dmz.object.text(handle, RankHandle)) {
            dmz.object.text.remove(handle, RankHandle);
         }
      }
   });
};

update_objective_graph = function () {
   var max = 0
     , budgets = []
     , list = []
     , ix
     , result
     , budget
     , array
     ;
   if (updateGraph) {
      for (ix = 0; ix <= barCount; ix += 1) {
         budget = maxPreventionBudget * (ix / barCount);
         budgets[ix] = budget;
         Object.keys(weightList).forEach(function (key) {
            weightList[key].setup();
         });
         Object.keys(objects).forEach(function (object) {
            weigh_object(objects[object]);
         });
         allocate_prevention_budget(objects, budget, maxPreventionBudget, vinf);
         result = 0;
         if (objective) {
            Object.keys(objects).forEach(function (key) {
               calc_risk_reduced(objects[key]);
               array = objective(objects[key]);
               self.log.warn ("a: " + array + " : " + array[0] + " " + array[1]);
               result += array[0];
               self.log.warn ("result: " + result);
            });
         }
         self.log.warn ("final result: " + result);
         if (result > max) {
            max = result;
         }
         list[ix] = result;
         self.log.warn ("list["+ix+"] = "+list[ix]);
      }
      if (max > 0) {
         self.log.warn("max: " + max);
         for (ix = 0; ix <= barCount; ix += 1) {
            list[ix] = Math.ceil(list[ix] / max * 100);
            self.log.warn("norm["+ix+"]" + list[ix]);
         }
      }
      else {
         for (ix = 0; ix <= barCount; ix += 1) {
            list[ix] = 0;
         }
      }
      for (ix = 0; ix <= barCount; ix += 1) {
         dmz.object.counter(bars[ix], ObjectiveBarValueHandle, list[ix]);
         dmz.object.text(bars[ix], ObjectiveBarLabel, "$" + Math.floor(budgets[ix]));
      }
   }
};

// function receive_simulator
simulatorMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
         do_rank();
      }
      else {
         receive_hide();
      }
   }
});

// function receive_prevention_budget
preventionBudgetMessage.subscribe(self, function (data, message) {
   if (dmz.data.isTypeOf(data)) {
      preventionBudget = data.number("Budget", 0);
      if (!preventionBudget) {
         preventionBudget = 0;
      }
      maxPreventionBudget = data.number("Budget", 1);
      if (!maxPreventionBudget) {
         maxPreventionBudget = 0;
      }
      if (visible) {
         do_rank();
      }
   }
});

// function receive_response_budget
responseBudgetMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      responseBudget = data.number("Budget", 0);
      maxResponseBudget = data.number("Budget", 1);
      if (visible) {
         do_rank();
      }
   }
});

// function receive_vinfinity
vinfinityMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      vinf = data.number("value", 0);
      if (!vinf) {
         vinf = 0.05;
      }
      if (visible) {
         do_rank();
      }
   }
});

updateObjectiveGraphMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      updateGraph = data.boolean("Boolean", 0);
      update_objective_graph();
   }
});

dmz.object.create.observe(self, function (handle, objType, varity) {
   if (objType) {
      if (objType.isOfType(NodeType) || objType.isOfType(NodeLinkType)) {
         objects[handle] = handle;
         if (visible && objects[handle]) {
            do_rank();
         }
         do_graph();
      }
   }
});

var update_object_scalar = function (handle) {
   if (visible && objects[handle]) {

      do_rank();
   }
   calc_risk_initial(handle);
   do_graph();
};

dmz.object.scalar.observe(self, ThreatHandle, update_object_scalar);
dmz.object.scalar.observe(self, VulnerabilityHandle, update_object_scalar);
dmz.object.scalar.observe(self, PreventionCostHandle, update_object_scalar);
dmz.object.scalar.observe(self, ConsequenceHandle, update_object_scalar);
dmz.object.scalar.observe(self, DegreeHandle, update_object_scalar);


var update_simulator_flag = function (handle, attr, value) {
   if (value) {
      if (attr == WeightDegreesHandle) {
         weightList[WeightDegreesHandle] = weight_degrees;
      }
      else if (attr == WeightBetweennessHandle) {
         weightList[WeightBetweennessHandle] = weight_betweenness;
      }
      else if (attr == WeightHeightHandle) {
         weightList[WeightHeightHandle] = weight_height;
      }
      else if (attr == WeightContagiousHandle) {
         weightList[WeightContagiousHandle] = weight_contagious;
      }
      else if (attr == ObjectiveNoneHandle) {
         objective = calc_objective_none;
      }
      else if (attr == ObjectiveRiskHandle) {
         objective = calc_objective_risk;
      }
      else if (attr == ObjectiveTxVHandle) {
         objective = calc_objective_txv;
      }
      else if (attr == ObjectiveThreatHandle) {
         objective = calc_objective_threat;
      }
      else if (attr == ObjectiveVulnerabilityHandle) {
         objective = calc_objective_vulnerability;
      }
      do_graph();
   }
   else if (weightList[attr]) {
      delete weightList[attr];
      do_graph();
   }
   if (visible) {
      do_rank();
   }
};

dmz.object.flag.observe(self, WeightDegreesHandle, update_simulator_flag);
dmz.object.flag.observe(self, WeightBetweennessHandle, update_simulator_flag);
dmz.object.flag.observe(self, WeightHeightHandle, update_simulator_flag);
dmz.object.flag.observe(self, WeightContagiousHandle, update_simulator_flag);
dmz.object.flag.observe(self, ObjectiveNoneHandle, update_simulator_flag);
dmz.object.flag.observe(self, ObjectiveRiskHandle, update_simulator_flag);
dmz.object.flag.observe(self, ObjectiveTxVHandle, update_simulator_flag);
dmz.object.flag.observe(self, ObjectiveThreatHandle, update_simulator_flag);
dmz.object.flag.observe(self, ObjectiveVulnerabilityHandle, update_simulator_flag);


dmz.object.destroy.observe(self, function (handle) {
   var updateRank = false;
   if (visible && objects[handle]) {
      updateRank = true;
   }
   delete objects[handle];
   if (updateRank) {
      do_rank();
   }
   do_graph();
});

dmz.object.link.observe(self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) {
      do_rank();
   }
   do_graph();
});

dmz.object.unlink.observe(self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) {
      do_rank();
   }
   do_graph();
});

dmz.object.linkAttributeObject.observe(self, LinkHandle,
function (link, attr, Super, sub, object) {
   if (visible && object && objects[object]) {
      do_rank();
   }
   do_graph();
});

dmz.object.state.observe(self, LinkFlowHandle, function (object) {
   if (visible && object && objects[object]) {
      do_rank();
   }
   do_graph();
});
