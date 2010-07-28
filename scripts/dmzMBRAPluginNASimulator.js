var dmz =
   { object: require("dmz/components/object")
   , objectType: require("dmz/runtime/objectType")
   , defs: require("dmz/runtime/definitions")
   , data: require("dmz/runtime/data")
   , mask: require("dmz/types/mask")
   , message: require("dmz/runtime/messaging")
   , util: require ("dmz/types/util")
   , time: require("dmz/runtime/time")
   }
   , ObjectiveBarValueHandle = dmz.defs.createNamedHandle ("NA_Objective_Bar_Value")
   , ObjectiveBarLabel = dmz.defs.createNamedHandle ("NA_Objective_Bar_Label")
   , NodeType = dmz.objectType.lookup("na_node")
   , NodeLinkType = dmz.objectType.lookup("na_link_attribute")
   , SimType = dmz.objectType.lookup("na_simulator")
   , LabelHandle = dmz.defs.createNamedHandle("NA_Node_Objective_Label")
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")
   , LinkFlowHandle = dmz.defs.createNamedHandle("NA_Link_Flow")
   , ThreatHandle = dmz.defs.createNamedHandle("NA_Node_Threat")
   , VulnerabilityHandle = dmz.defs.createNamedHandle ("NA_Node_Vulnerability")
   , VulnerabilityReducedHandle = dmz.defs.createNamedHandle ("NA_Node_Vulnerability_Reduced")
   , PreventionCostHandle = dmz.defs.createNamedHandle ("NA_Node_Prevention_Cost")
   , PreventionAllocationHandle = dmz.defs.createNamedHandle ("NA_Node_Prevention_Allocation")
   , ConsequenceHandle = dmz.defs.createNamedHandle ("NA_Node_Consequence")
   , RiskInitialHandle = dmz.defs.createNamedHandle ("NA_Node_Risk_Initial")
   , RiskReducedHandle = dmz.defs.createNamedHandle ("NA_Node_Risk_Reduced")
   , WeightHandle = dmz.defs.createNamedHandle ("NA_Node_Weight")
   , WeightAndObjectiveHandle = dmz.defs.createNamedHandle ("NA_Node_Weight_And_Objective")
   , GammaHandle = dmz.defs.createNamedHandle ("NA_Node_Gamma")
   , RankHandle = dmz.defs.createNamedHandle ("NA_Node_Rank")
   , DegreeHandle = dmz.defs.createNamedHandle ("NA_Node_Degrees")
   , BetweennessHandle = dmz.defs.createNamedHandle ("NA_Node_Betweenness")
   , HeightHandle = dmz.defs.createNamedHandle ("NA_Node_Height")
   , OverlayState = dmz.defs.lookupState ("NA_Node_Overlay")
   , ForwardState = dmz.defs.lookupState ("NA_Flow_Forward")
   , ReverseState = dmz.defs.lookupState ("NA_Flow_Reverse")
   , FlowStateMask = ForwardState.or(ReverseState)

   , WeightDegreesHandle = dmz.defs.createNamedHandle ("NA_Weight_Degrees")
   , WeightBetweennessHandle = dmz.defs.createNamedHandle ("NA_Weight_Betweenness")
   , WeightHeightHandle = dmz.defs.createNamedHandle ("NA_Weight_Height")

   , ObjectiveNoneHandle = dmz.defs.createNamedHandle ("NA_Objective_None")
   , ObjectiveRiskHandle = dmz.defs.createNamedHandle ("NA_Objective_Risk")
   , ObjectiveContagiousHandle = dmz.defs.createNamedHandle ("NA_Objective_Contagiousness")
   , ObjectiveTxVHandle = dmz.defs.createNamedHandle ("NA_Objective_TxV")
   , ObjectiveThreatHandle = dmz.defs.createNamedHandle ("NA_Objective_Threat")
   , ObjectiveVulnerabilityHandle = dmz.defs.createNamedHandle ("NA_Objective_Vulnerability")
   , ObjectiveConsequenceHandle = dmz.defs.createNamedHandle ("NA_Objective_Consequence")
   , simulatorMessage = dmz.message.create(
         self.config.string ("simulator-message.name", "NASimulatorMessage"))
   , preventionBudgetMessage = dmz.message.create(
         self.config.string ("message.prevention-budget.name",
                             "PreventionBudgetMessage"))
   , responseBudgetMessage = dmz.message.create(
         self.config.string ("message.response-budget.name", "ResponseBudgetMessage"))
   , vinfinityMessage = dmz.message.create(
         self.config.string ("v-infinity-message.name",
                             "NAVulnerabilityInfinityMessage"))
   , updateObjectiveGraphMessage = dmz.message.create(
         self.config.string ("update-objective-graph-message.name",
                             "NA_Objective_Graph_Visible_Message"))
   , updateSumsMessage = dmz.message.create(
         self.config.string ("message.sums.name", "NA_Objective_Sums_Message"))
   , doRankCount = 0
   , do_rank = function () { doRankCount = 2; }
   , doGraphCount = 0
   , do_graph = function () { doGraphCount = 2; }
   , objects = []
   , calc_objective_none = function (object) {
      dmz.object.text (object, LabelHandle, "");
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
   , vinf = 0.05
   , barCount = 10
   , bars = []
   , visible = false
   , reducedSum = 0
   , origSum = 0
   , updateGraph = false
   , rankLimit = self.config.number ("rank.limit", 9)
   ;

(function () {
   for (var ix = 0; ix <= barCount; ix += 1) {
      bars[ix] = dmz.object.create ("na_objective_bar");
      dmz.object.counter (bars[ix], "NA_Objective_Bar_Number", ix);
      dmz.object.counter (bars[ix], "NA_Objective_Bar_Value", 0);
      dmz.object.activate (bars[ix]);
   }
})();

var work_func = function () {
   if (doGraphCount > 1) { doGraphCount -= 1; }
   else if (doGraphCount == 1) {
      update_objective_graph ();
      doGraphCount = 0;
   }
   if (doRankCount > 1) { doRankCount -= 1; }
   else if (doRankCount == 1) {
      receive_rank ();
      doRankCount = 0;
   }
};

var timeSlice = dmz.time.setRepeatingTimer (self, work_func);

var not_zero = function (value) { return dmz.util.isNotZero (value); };

var calc_risk_initial = function (object) {
   var Threat = dmz.object.scalar (object, ThreatHandle);
   var Vulnerability = dmz.object.scalar (object, VulnerabilityHandle);
   var Consequence = dmz.object.scalar (object, ConsequenceHandle);
   if (Threat && Vulnerability && Consequence) {
      dmz.object.scalar (
         object,
         RiskInitialHandle,
         Threat * Vulnerability * Consequence);
   } else { dmz.object.scalar (object, RiskInitialHandle, 0); }
};

var calc_vulnerability = function (object) {
   var result = 0;
   var Allocation = dmz.object.scalar (object, PreventionAllocationHandle);
   var Vulnerability = dmz.object.scalar (object, VulnerabilityHandle);
   if (Vulnerability) { result = Vulnerability; }
   var Cost = dmz.object.scalar (object, PreventionCostHandle);
   var Gamma = dmz.object.scalar (object, GammaHandle);
   if (Gamma && not_zero (Gamma) && Vulnerability && (Vulnerability > 0) &&
         Cost && (Cost > 0) && not_zero (Cost) &&
         Allocation && not_zero (Allocation)) {
      result = Vulnerability * Math.exp (-Gamma * Allocation / Cost);
   }
   dmz.object.scalar (object, VulnerabilityReducedHandle, result);
   return result;
};

var calc_risk_reduced = function (object) {
   var result = 0;
   var Threat = dmz.object.scalar (object, ThreatHandle);
   var Vulnerability = calc_vulnerability (object);
   var Consequence = dmz.object.scalar (object, ConsequenceHandle);
   if (Threat && Vulnerability && Consequence) {
      result = Threat * Vulnerability * Consequence;
   }
   dmz.object.scalar (object, RiskReducedHandle, result);
   return result;
};

var format_result = function (value) { return "" + Math.round (value * 100) + "%"; };

var calc_objective_risk = function (object) {
   var result = dmz.object.scalar (object, RiskReducedHandle);
   var orig = dmz.object.scalar (object, RiskInitialHandle);
   if (!result) { result = 0; }
   if (visible) { dmz.object.text(object, LabelHandle, "Risk = " + result.toFixed(2));}
   return [result, orig];
};

var calc_objective_contagiousness = function (object) {
   var result = 0;
   var orig = 0;
   var Threat = dmz.object.scalar (object, ThreatHandle);
   var VulnerabilityReduced = dmz.object.scalar (object, VulnerabilityReducedHandle);
   var Vulnerability = dmz.object.scalar (object, VulnerabilityHandle);
   var Degrees = dmz.object.scalar (object, DegreeHandle);
   if (Threat && VulnerabilityReduced && Degrees) {
      result = Threat * VulnerabilityReduced * Degrees;
   }
   if (Threat && Vulnerability && Degrees) { orig = Threat * Vulnerability * Degrees; }
   if (visible) {
      dmz.object.text (object,LabelHandle,"Contagiousness = " + result.toFixed(2));
   }
   return [result, orig];
};

var calc_objective_txv = function (object) {
   var result = 0;
   var orig = 0;
   var Threat = dmz.object.scalar (object, ThreatHandle);
   var VulnerabilityReduced = dmz.object.scalar (object, VulnerabilityReducedHandle);
   var Vulnerability = dmz.object.scalar (object, VulnerabilityHandle);
   if (Threat && VulnerabilityReduced) { result = Threat * VulnerabilityReduced; }
   if (Threat && Vulnerability) { orig = Threat * Vulnerability; }
   if (visible) {
      dmz.object.text (object, LabelHandle, "T x V = " + format_result (result));
   }
   return [result, orig];
};

var calc_objective_threat = function (object) {
   var result = dmz.object.scalar (object, ThreatHandle);
   if (!result) { result = 0; }
   if (visible) {
      dmz.object.text (object, LabelHandle, "Threat = " + format_result (result));
   }
   return [result, result];
};

var calc_objective_vulnerability = function (object) {
   var result = dmz.object.scalar (object, VulnerabilityReducedHandle);
   var orig = dmz.object.scalar (object, VulnerabilityHandle);
   if (!result) { result = 0; }
   if (visible) {
      dmz.object.text (object, LabelHandle,
                       "Vulnerability = " + format_result (result));
   }
   return [result, orig];
};

var calc_objective_consequence = function (object) {
   var result = dmz.object.scalar (object, ConsequenceHandle);
   if (!result) { result = 0; }
   if (visible) {
      var str = "Consequence = $" + result.toFixed(2);
      dmz.object.text (object, LabelHandle, str);
   }
   return [result, result];
};

var weight_degrees = {

   setup: function () {
      maxDegrees = 0;
      var keys = Object.keys (objects);
      keys.forEach(function (key){
         var value = dmz.object.scalar (objects[key], DegreeHandle);
         if (value && (value > maxDegrees)) { maxDegrees = value; }
      });
   },
   
   calc: function (object) {
      var result = 0;
      var value = dmz.object.scalar (object, DegreeHandle);
      if (value && (maxDegrees > 0)) { result = value / maxDegrees; }
      return result;
   }

};

var EmptyMask = dmz.mask.create();

var link_reachable = function (link, flowState) {
   var result = true;
   var obj = dmz.object.linkAttributeObject (link);
   if (obj) {
      var state = dmz.object.state (obj, LinkFlowHandle);
      if (state && (!state.and (FlowStateMask).equal (EmptyMask))) {
         if (!state.contains (flowState)) { result = false; }
      }
   }
   return result;
};

var all_links_flow = function (superList, subList, flowState) {
   var result = true;
   var subBool = true;
   if (superList && subList) {
      var keys = Object.keys (superList);
      keys.forEach(function (superkey) {
         if (!result) { return; }
         var subkeys = Object.keys (subList);
         keys.forEach(function (subkey) {
            if (!subList[subkey]) { return; }
            var link = dmz.object.linkHandle (LinkHandle, superList[superkey],
                                                    subList[subkey]);
            var obj = null;
            if (link) { obj = dmz.object.linkAttributeObject (link); }
            else { self.log.error ("No link found"); }
            if (obj) {
               var state = dmz.object.state (obj, LinkFlowHandle);
               if (state.bool ()) {
                  if (!state.and (FlowStateMask).equal(flowState)) {
                     result = false;
                     subBool = false;
                  }
               } else { result = false; subBool = false; }
            } else { result = false; subBool = false; }
         });
      });
   }
   return result;
};

var is_sink = function (object) {
   var sinkObjList = [];
   sinkObjList[0] = object;
   var result = false;
   var sub = dmz.object.subLinks (object, LinkHandle);
   var superLink = dmz.object.superLinks (object, LinkHandle);
   if (sub || superLink) {
      if (all_links_flow (sinkObjList, sub, ReverseState) &&
          all_links_flow (superLink, sinkObjList, ForwardState)) {
         result = true;
      }
   }
   return result;
};

var add_to_node_betweenness_counter = function (object) {
   var value = dmz.object.addToCounter (object, BetweennessHandle);
   if (value > maxBetweenness) { maxBetweenness = value; }
};

var add_to_link_betweenness_counter = function (link) {
   var linkObj = dmz.object.linkAttributeObject (link);
   if (linkObj) {
      var value = dmz.object.addToCounter (linkObj, BetweennessHandle);
      if (value > maxBetweenness) { maxBetweenness = value; }
   }
};

var find_betweenness = function (current, target, visited) {
   //self.log.warn ("find_betweeness");
   var keys = Object.keys (current);
   keys.forEach(function (key) { visited[current[key].object] = true; });
   var found = false;
   var list = [];
   var keys = Object.keys (current);
   keys.forEach(function (key) {
      var item = current[key];
      var subs = dmz.object.subLinks (item.object, LinkHandle)
      //self.log.warn("subs: " + subs);
      if (subs) {
         var subkeys = Object.keys (subs);
         subkeys.forEach(function (sub) {
            var link = dmz.object.linkHandle (LinkHandle, item.object, subs[sub]);
            //self.log.warn ("link: " + link);
           if (link_reachable (link, ForwardState)) {
              if (subs[sub] == target) {
                  found = true;
                  item.found = true;
                  //self.log.warn ("incrementing counters:" + target);
                  add_to_node_betweenness_counter (target);
                  add_to_link_betweenness_counter (link);
               } else if (!visited[subs[sub]]) {
                  list.push ({ object: subs[sub], link: link, parent: item });
               }
            }
         });
      }
      var supers = dmz.object.superLinks (item.object, LinkHandle)
      if (supers) {
         var superLinks = Object.keys (supers);
         superLinks.forEach(function (superLink) {
            var link = dmz.object.linkHandle (LinkHandle, supers[superLink],
                                              item.object);
            if (link_reachable (link, ReverseState)) {
               if (supers[superLink] == target) {
                  found = true;
                  item.found = true;
                  //self.log.warn ("incrementing counters:" + target);
                  add_to_node_betweenness_counter (target);
                  add_to_link_betweenness_counter (link);
               } else if (!visited[supers[superLink]]) {
                  list.push ({ object: supers[superLink], link: link, parent: item});
               }
            }
         });
      }
   });
   if (!found && list.length > 0) {
      find_betweenness (list, target, visited);
      for (var place = list.length - 1; place >= 0; place -= 1) {
         var item = list[place];
         if (item.found) {
            add_to_node_betweenness_counter (item.object);
            add_to_link_betweenness_counter (item.link);
            item.parent.found = true;
         }
      }
   }
}

var weight_betweenness = {

   setup: function () {
      //self.log.warn ("w_b: setup start");
      maxBetweenness = 0;
      var keys = Object.keys (objects);
      keys.forEach(function (key) {
         dmz.object.counter (objects[key], BetweennessHandle, 0);
      });
      var keys = Object.keys (objects);
      keys.forEach(function (key) {
         var root = objects[key];
//         self.log.warn ("w_b: " + dmz.object.isObject (root) + " " +
//                        dmz.object.type (root));
         if (dmz.object.type (root).isOfType (NodeType)) {
            var handles = Object.keys (objects);
            handles.forEach(function(index) {
               var target = objects[index];
               if (root != target && dmz.object.type (target).isOfType (NodeType)) {
                  var list = [{object: root}];
                  var visited = [];
//                  self.log.warn ("calling f_b");
                  find_betweenness (list, target, visited);
                  if (list[0]) { add_to_node_betweenness_counter (root); }
               }
            });
         }
      });
   },

   calc: function (object) {
      var result = 0;
      var value = dmz.object.counter (object, BetweennessHandle);
      if (value && (maxBetweenness > 0)) { result = value / maxBetweenness; }
      return result;
   }

};

var update_height = function (obj, level, visited) {
   var result = false;
   if (dmz.object.isLink (obj)) { obj = dmz.object.linkAttributeObject (obj); }
   if (obj && !visited[obj]) {
      result = true;
      visited[obj] = true;
      var value = dmz.object.counter (obj, HeightHandle);
      if (!value) { value = 0; }
      if (level > value) { dmz.object.counter (obj, HeightHandle, level); }
   }
   return result;
}

var find_height = function (sink) {
   var queue = [];
   var visited = [];
   queue.push (sink);
   update_height (sink, 1, visited);
   while (queue.length > 0)  {
      var node = queue.shift ();
      var height = dmz.object.counter (node, HeightHandle);
      if (!height) {
         self.log.error ("No height found for.", node);
         height = 0;
      }
      var subList = dmz.object.subLinks (node, LinkHandle);
      if (subList) {
         var keys = Object.keys (subList);
         keys.forEach(function (key){
            var link = dmz.object.linkHandle (LinkHandle, node, subList[key]);
            if (link) {
               if (link_reachable (link, ReverseState)) {
                  update_height (link, height + 1, visited)
                  if (update_height (subList[key], height + 2, visited)) {
                     queue.push (subList[key]);
                  }
               }
            }
         });
      }
      var superList = dmz.object.superLinks (node, LinkHandle);
      if (superList) {
         var keys = Object.keys (superList);
         keys.forEach(function (key){
            var link = dmz.object.linkHandle (LinkHandle, superList[key], node);
            if (link) {
               if (link_reachable (link, ForwardState)) {
                  update_height (link, height + 1, visited);
                  if (update_height (superList[key], height + 2, visited)) {
                     queue.push (superList[key]);
                  }
               }
            }
         });
      }
   }
};

var weight_height = {

   setup: function () {
      maxHeight = 0;
      var keys = Object.keys (objects);
      keys.forEach(function (key) {
         dmz.object.counter (objects[key], HeightHandle, 0);
      });

      var sinkFound = false;
      var keys = Object.keys (objects);
      keys.forEach(function (key){
         if (is_sink (objects[key])) {
            sinkFound = true;
            find_height (objects[key]);
         }
      });
      if (sinkFound) {
         var keys = Object.keys (objects);
         keys.forEach(function (key){
            var height = dmz.object.counter (objects[key], HeightHandle);
            if (height && (height > maxHeight)) { maxHeight = height; }
         });
      } else {
         maxHeight = 1;
         var keys = Object.keys (objects);
         keys.forEach(function (key) {
            dmz.object.counter (objects[key], HeightHandle, 0);
         });
      }
   },

   calc: function (object) {
      var result = 0;
      var value = dmz.object.counter (object, HeightHandle);
      //self.log.warn ("VAL::: value");
      if (value && (maxHeight > 0)) { result = value / maxHeight; }
      return result;
   }
};

var weigh_object = function (object) {
   var value = 1;
   //self.log.warn (dmz.object.text(object, dmz.defs.createNamedHandle("NA_Node_Name")));
   var keys = Object.keys (weightList);
   keys.forEach(function (key) {
      value *= weightList[key].calc (object);
      //self.log.warn ("\tvalue: " + value + " " + weightList[key].calc(object));
   });
   dmz.object.scalar (object, WeightHandle, value);
}

var log_defender_term = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma;
   if (not_zero (result)) {
      result = object.cost / result;
      if (not_zero (object.gamma) && not_zero (result)) {
         result = (object.cost / object.gamma) * Math.log (result);
      } else { result = 0; }
   } else { result = 0; }
   return result;
}

var log_defender = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
         object.gamma
   if (not_zero (result) && not_zero (object.cost)) {
      result = Math.log (object.cost / result)
   } else { result = 0; }
   return result;
}

var allocate_prevention_budget = function (handleList, budget, maxBudget, vinf) {
   if (dmz.util.isZero (budget)) {
      var keys = Object.keys (handleList);
      keys.forEach(function (key) {
         dmz.object.scalar (handleList[key], PreventionAllocationHandle, 0);
         dmz.object.scalar (handleList[key], GammaHandle, 0);
      });
   } else {
      var objectList = [];
      var keys = Object.keys (handleList);
      keys.forEach(function (key) {
         var object = { handle: handleList[key] };
         object.vul = dmz.object.scalar (handleList[key], VulnerabilityHandle);
         if (!object.vul || (object.vul <= 0)) { object.vul = 1; }
         object.gamma = -Math.log (vinf / object.vul);
         if (object.gamma < 0) { object.gamma = 0; }
         dmz.object.scalar (handleList[key], GammaHandle, object.gamma);
         object.cost = dmz.object.scalar (handleList[key], PreventionCostHandle);
         if (!object.cost) { object.cost = 0; }
         object.weight = dmz.object.scalar (handleList[key], WeightHandle);
         if (!object.weight) { object.weight = 0; }
         object.threat = dmz.object.scalar (handleList[key], ThreatHandle);
         if (!object.threat) { object.threat = 0; }
         object.consequence = dmz.object.scalar (handleList[key], ConsequenceHandle);
         if (!object.consequence) { object.consequence = 0; }
         object.allocation = 0;
         objectList.push (object);
      });
      var A = 0;
      var B = 0;
      var keys = Object.keys (objectList);
      keys.forEach(function (key){
         A = A + log_defender_term (objectList[key]);
         if (not_zero (objectList[key].gamma)) {
            B = B + (objectList[key].cost / objectList[key].gamma);
         }
      });
      var totalAllocation = 0;
      var logLamda = 0;
      if (not_zero (B)) { logLamda = (-budget - A) / B; }
      var keys = Object.keys (objectList);
      keys.forEach(function (key) {
         var object = objectList[key];
         var A = 0;
         if (not_zero (objectList[key].gamma)) {
            A = object.cost / object.gamma;
         }
         var B = log_defender (object);
         object.allocation = -A * (logLamda + B);
         if (object.allocation < 0) { object.allocation = 0; }
         if (object.allocation > object.cost) {
            object.allocation = object.cost;
         }
         totalAllocation += object.allocation;
      });
      var scale = 1;
      if (totalAllocation < budget) {
         var size = objectList.length - 1;
         var count = 0;
         var remainder = budget - totalAllocation;
         while (not_zero (remainder) && (count <= size))  {
            var object = objectList[count];
            var max = object.cost - object.allocation;
            if (max > remainder) {
               max = remainder;
               remainder = 0;
            } else { remainder = remainder - max; }
            object.allocation += max;
            count += 1;
         }
      } else { scale = budget / totalAllocation; }
      totalAllocation = 0;
      var keys = Object.keys (objectList);
      keys.forEach(function (key) {
         var object = objectList[key];
         object.allocation = object.allocation * scale;
         totalAllocation = totalAllocation + object.allocation;
         dmz.object.scalar (object.handle, PreventionAllocationHandle,
                            object.allocation);
      });
   }
}

var rank_object = function (object) {
   var result = dmz.object.scalar (object, WeightHandle);
   if (!result) { result = 1; }
   if (objective) {
      var objectiveArray = objective (object);
      var reduced = objectiveArray[0];
      var Orig = objectiveArray[1];
      reducedSum += reduced;
      origSum += Orig;
      result *= reduced;
   }
   dmz.object.scalar (object, WeightAndObjectiveHandle, result);
   return result;
};

var receive_rank = function () {
   visible = true;
   var list = [];
   var keys = Object.keys (weightList);
   keys.forEach(function (key){ weightList[key].setup (); });
   var keys = Object.keys (objects);
   keys.forEach(function (key){ weigh_object (objects[key]); });
   allocate_prevention_budget (objects, preventionBudget, maxPreventionBudget, vinf);
   reducedSum = 0;
   origSum = 0;
   var keys = Object.keys (objects);
   keys.forEach(function (key){

      if (dmz.object.isObject (objects[key])) {
         var state = dmz.object.state (objects[key]);
         if (state) {
            state = state.unset (OverlayState)
            dmz.object.state (objects[key], null, state)
         }
         if (dmz.object.text (objects[key], RankHandle)) {
            dmz.object.text.remove (objects[key], RankHandle);
         }
         calc_risk_reduced (objects[key]);
         var object = { handle: objects[key] };
         object.rank = rank_object (object.handle);
         list.push (object);
      }
   });
   var data = dmz.data.create ();
   data.number ("Float64", 0, reducedSum);
   data.number ("Float64", 1, origSum);
   //self.log.error ("mB: " + maxBetweenness);
   updateSumsMessage.send (data);
   list.sort (function (obj1, obj2) { return obj2.rank - obj1.rank; });
   var keys = Object.keys (list);
   var count = 1;
   var lastRank = null;
   var keys = Object.keys (list);
   keys.forEach(function (index) {
      var object = list[index];
      if (!lastRank) { lastRank = object.rank; }
      else if (lastRank > object.rank) {
         count += 1;
         lastRank = object.rank;
      }
      //self.log.warn ("Count: " + count);
      dmz.object.text (object.handle, RankHandle, count);
      if (rankLimit && count <= rankLimit && object.rank > 0) {
         var state = dmz.object.state (object.handle);
         if (!state) { state = dmz.mask.create (); }
         state = state.or (OverlayState);
         dmz.object.state (object.handle, null, state);
      }
   });
}

var receive_hide = function () {
   visible = false;
   var keys = Object.keys (objects);
   keys.forEach(function (key){
      var handle = objects[key];
      if (dmz.object.isObject (handle)) {
         dmz.object.text (handle, LabelHandle, "");
         var state = dmz.object.state (handle);
         if (state) {
            state = state.unset (OverlayState);
            dmz.object.state (handle, null, state);
         }
         if (dmz.object.text (handle, RankHandle)) {
            dmz.object.text.remove (handle, RankHandle);
         }
      }
   });
}

var update_objective_graph = function () {
   if (updateGraph) {
      var max = 0;
      var budgets = [];
      var list = [];
      for (var ix = 0; ix <= barCount; ix += 1) {
         var budget = maxPreventionBudget * (ix / barCount);
         budgets[ix] = budget;
         Object.keys (weightList).forEach(function (key) { weightList[key].setup (); });
         Object.keys (objects).forEach(function (object) {
            weigh_object (objects[object]);
         });
         allocate_prevention_budget (objects, budget, maxPreventionBudget, vinf);
         var result = 0;
         if (objective) {
            var keys = Object.keys (objects);
            keys.forEach(function (key){
               calc_risk_reduced (objects[key]);
               var array = objective (objects[key]);
               result += array[0];
            });
         }
         if (result > max) { max = result; }
         list[ix] = result;
      }
      if (max > 0) {
         for (var ix = 0; ix <= barCount; ix += 1) {
            list[ix] = Math.ceil (list[ix] / max * 100);
         }
      } else {
         for (var ix = 0; ix <= barCount; ix += 1) {
            list[ix] = 0;
         }
      }
      for (var ix = 0; ix <= barCount; ix += 1) {
         dmz.object.counter (bars[ix], ObjectiveBarValueHandle, list[ix])
         dmz.object.text (bars[ix], ObjectiveBarLabel, "$" + Math.floor (budgets[ix]));
      }
   }
}

// function receive_simulator
simulatorMessage.subscribe (self, function (data) {
   if (dmz.data.isTypeOf (data)) {
      if (data.boolean ("Boolean", 0)) { do_rank (); }
      else { receive_hide (); }
   }
});

// function receive_prevention_budget
preventionBudgetMessage.subscribe (self, function (data, message) {
   if (dmz.data.isTypeOf (data)) {
      preventionBudget = data.number ("Budget", 0);
      if (!preventionBudget) { preventionBudget = 0; }
      maxPreventionBudget = data.number ("Budget", 1);
      if (!maxPreventionBudget) { maxPreventionBudget = 0; }
      if (visible) { do_rank (); }
   }
});

// function receive_response_budget
responseBudgetMessage.subscribe (self, function (data) {
   if (dmz.data.isTypeOf (data)) {
      responseBudget = data.number ("Budget", 0);
      maxResponseBudget = data.number ("Budget", 1);
      if (visible) { do_rank (); }
   }
});

// function receive_vinfinity
vinfinityMessage.subscribe (self, function (data) {
   if (dmz.data.isTypeOf (data)) {
      vinf = data.number ("value", 0);
      if (!vinf) { vinf = 0.05; }
      if (visible) { do_rank (); }
   }
});

updateObjectiveGraphMessage.subscribe (self, function (data) {
   if (dmz.data.isTypeOf (data)) {
      updateGraph = data.boolean ("Boolean", 0);
      update_objective_graph ();
   }
});

dmz.object.create.observe (self, function (handle, objType, varity) {
   if (objType) {
      if (objType.isOfType (NodeType) || objType.isOfType (NodeLinkType)) {
         objects[handle] = handle;
         if (visible && objects[handle]) { do_rank (); }
         do_graph ();
      }
   }
});

var update_object_scalar = function (handle) {
   if (visible && objects[handle]) { do_rank (); }
   calc_risk_initial (handle);
   do_graph ();
}

dmz.object.scalar.observe (self, ThreatHandle, update_object_scalar);
dmz.object.scalar.observe (self, VulnerabilityHandle, update_object_scalar);
dmz.object.scalar.observe (self, PreventionCostHandle, update_object_scalar);
dmz.object.scalar.observe (self, ConsequenceHandle, update_object_scalar);
dmz.object.scalar.observe (self, DegreeHandle, update_object_scalar);


var update_simulator_flag = function (handle, attr, value) {
    if (value) {
       if (attr == WeightDegreesHandle) {
          weightList[WeightDegreesHandle] = weight_degrees;
       } else if (attr == WeightBetweennessHandle) {
          weightList[WeightBetweennessHandle] = weight_betweenness;
       } else if (attr == WeightHeightHandle) {
          weightList[WeightHeightHandle] = weight_height;
       } else if (attr == ObjectiveNoneHandle) {
          objective = calc_objective_none;
       } else if (attr == ObjectiveRiskHandle) {
          objective = calc_objective_risk;
       } else if (attr == ObjectiveContagiousHandle) {
          objective = calc_objective_contagiousness;
       } else if (attr == ObjectiveTxVHandle) {
          objective = calc_objective_txv;
       } else if (attr == ObjectiveThreatHandle) {
          objective = calc_objective_threat;
       } else if (attr == ObjectiveVulnerabilityHandle) {
          objective = calc_objective_vulnerability;
       } else if (attr == ObjectiveConsequenceHandle) {
          objective = calc_objective_consequence;
       }
       do_graph ();
    } else if (weightList[attr]) {
       delete weightList[attr];
       do_graph ();
    }
    if (visible) { do_rank (); }
};

dmz.object.flag.observe (self, WeightDegreesHandle, update_simulator_flag);
dmz.object.flag.observe (self, WeightBetweennessHandle, update_simulator_flag);
dmz.object.flag.observe (self, WeightHeightHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveNoneHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveRiskHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveContagiousHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveTxVHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveThreatHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveVulnerabilityHandle, update_simulator_flag);
dmz.object.flag.observe (self, ObjectiveConsequenceHandle, update_simulator_flag);


dmz.object.destroy.observe (self, function (handle) {
   var updateRank = false;
   if (visible && objects[handle]) { updateRank = true; }
   delete objects[handle];
   if (updateRank) { do_rank (); }
   do_graph ();
});

dmz.object.link.observe (self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) { do_rank (); }
   do_graph ();
});

dmz.object.unlink.observe (self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) { do_rank (); }
   do_graph ();
});

dmz.object.linkAttributeObject.observe (self, LinkHandle,
function (link, attr, Super, sub, object) {
   if (visible && object && objects[object]) { do_rank (); }
   do_graph ();
});

dmz.object.state.observe (self, LinkFlowHandle, function (object) {
   if (visible && object && objects[object]) { do_rank (); }
   do_graph ();
});
