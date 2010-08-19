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
        self.config.string(
           "message.prevention-budget.name",
           "PreventionBudgetMessage"))
   , responseBudgetMessage = dmz.message.create(
        self.config.string("message.response-budget.name", "ResponseBudgetMessage"))
   , vinfinityMessage = dmz.message.create(
        self.config.string("v-infinity-message.name", "NAVulnerabilityInfinityMessage"))
   , updateObjectiveGraphMessage = dmz.message.create(
        self.config.string(
           "update-objective-graph-message.name",
           "NA_Objective_Graph_Visible_Message"))
   , updateSumsMessage = dmz.message.create(
        self.config.string(
           "message.sums.name",
           "NA_Objective_Sums_Message"))
   , doRankCount = 0
   , doRank = function () {
        doRankCount = 2;
     }
   , doGraphCount = 0
   , doGraph = function () {
        doGraphCount = 2;
     }
   , objects = {}
   , calcObjectiveNone = function (object) {
        dmz.object.text(object, LabelHandle, "");
        return [1, 1];
     }
   , objective = calcObjectiveNone
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
   , EmptyMask = dmz.mask.create()

   , updateObjectiveGraph
   , weighObject
   , allocatePreventionBudget
   , calcRiskReduced
   , receiveRank
   , workFunc
   , timeSlice
   , notZero = dmz.util.isNotZero
   , calcRiskInitial
   , calcVulnerability
   , calcRiskReduced
   , formatResult
   , calcObjectiveRisk
   , calcObjectiveTxV
   , calcObjectiveThreat
   , calcObjectiveVulnerability
   , calcObjectiveConsequence
   , weightDegrees
   , linkReachable
   , allLinksFlow
   , isSink
   , addToNodeBetweennessCounter
   , addToLinkBetweennessCounter
   , findBetweenness
   , weightBetweenness
   , updateHeight
   , findHeight
   , weightHeight
   , weightContagious
   , weighObject
   , logDefenderTerm
   , logDefender
   , allocatePreventionBudget
   , rankObject
   , receiveRank
   , receiveHide
   , updateSimulatorFlag
   , updateObjectScalar
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

workFunc = function () {
   if (doGraphCount > 1) {
      doGraphCount -= 1;
   }
   else if (doGraphCount == 1) {
      updateObjectiveGraph();
      doGraphCount = 0;
   }
   if (doRankCount > 1) {
      doRankCount -= 1;
   }
   else if (doRankCount == 1) {
      receiveRank();
      doRankCount = 0;
   }
};

timeSlice = dmz.time.setRepeatingTimer(self, workFunc);

calcRiskInitial = function (object) {
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

calcVulnerability = function (object) {
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
   if (Gamma && notZero(Gamma) && Vulnerability && (Vulnerability > 0) &&
         Cost && (Cost > 0) && notZero(Cost) &&
         Allocation && notZero(Allocation)) {
      result = Vulnerability * Math.exp(-Gamma * Allocation / Cost);
   }
   dmz.object.scalar(object, VulnerabilityReducedHandle, result);
   return result;
};

calcRiskReduced = function (object) {
   var result = 0
     , Threat = dmz.object.scalar(object, ThreatHandle)
     , Vulnerability = calcVulnerability(object)
     , Consequence = dmz.object.scalar(object, ConsequenceHandle)
     ;
   if (Threat && Vulnerability && Consequence) {
      result = Threat * Vulnerability * Consequence;
   }
   dmz.object.scalar(object, RiskReducedHandle, result);
   return result;
};

formatResult = function (value) {
   return (value * 100).toFixed() + "%";
};

calcObjectiveRisk = function (object) {
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

calcObjectiveTxV = function (object) {
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
      dmz.object.text(object, LabelHandle, "T x V = " + formatResult(result));
   }
   return [result, orig];
};

calcObjectiveThreat = function (object) {
   var result = dmz.object.scalar(object, ThreatHandle)
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle, "Threat = " + formatResult(result));
   }
   return [result, result];
};

calcObjectiveVulnerability = function (object) {
   var result = dmz.object.scalar(object, VulnerabilityReducedHandle)
     , orig = dmz.object.scalar(object, VulnerabilityHandle)
     ;
   if (!result) {
      result = 0;
   }
   if (visible) {
      dmz.object.text(object, LabelHandle,
                       "Vulnerability = " + formatResult(result));
   }
   return [result, orig];
};

calcObjectiveConsequence = function (object) {
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

weightDegrees = {

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

linkReachable = function (link, flowState) {
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

allLinksFlow = function (superList, subList, flowState) {
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

isSink = function (object) {
   var sinkObjList = []
     , result = false
     , sub = dmz.object.subLinks(object, LinkHandle)
     , superLink = dmz.object.superLinks(object, LinkHandle)
     ;
   sinkObjList[0] = object;
   if (sub || superLink) {
      if (allLinksFlow(sinkObjList, sub, ReverseState) &&
          allLinksFlow(superLink, sinkObjList, ForwardState)) {
         result = true;
      }
   }
   return result;
};

addToNodeBetweennessCounter = function (object) {
   var value = dmz.object.addToCounter(object, BetweennessHandle);
   if (value > maxBetweenness) {
      maxBetweenness = value;
   }
};

addToLinkBetweennessCounter = function (link) {
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

findBetweenness = function (current, target, visited) {
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
            if (linkReachable(link, ForwardState)) {
               if (subs[sub] == target) {
                  found = true;
                  item.found = true;
                  addToNodeBetweennessCounter(target);
                  addToLinkBetweennessCounter(link);
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
            if (linkReachable(link, ReverseState)) {
               if (supers[superLink] == target) {
                  found = true;
                  item.found = true;
                  addToNodeBetweennessCounter(target);
                  addToLinkBetweennessCounter(link);
               }
               else if (!visited[supers[superLink]]) {
                  list.push({ object: supers[superLink], link: link, parent: item});
               }
            }
         });
      }
   });
   if (!found && list.length > 0) {
      findBetweenness(list, target, visited);
      for (place = list.length - 1; place >= 0; place -= 1) {
         item = list[place];
         if (item.found) {
            addToNodeBetweennessCounter(item.object);
            addToLinkBetweennessCounter(item.link);
            item.parent.found = true;
         }
      }
   }
};

weightBetweenness = {

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
                  findBetweenness(list, target, visited);
                  if (list[0]) {
                     addToNodeBetweennessCounter(root);
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

updateHeight = function (obj, level, visited) {
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

findHeight = function (sink) {
   var queue = []
     , visited = []
     , node
     , height
     , subList
     , superList
     , link
     ;
   queue.push(sink);
   updateHeight(sink, 1, visited);
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
               if (linkReachable(link, ReverseState)) {
                  updateHeight(link, height + 1, visited);
                  if (updateHeight(subList[key], height + 2, visited)) {
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
               if (linkReachable(link, ForwardState)) {
                  updateHeight(link, height + 1, visited);
                  if (updateHeight(superList[key], height + 2, visited)) {
                     queue.push(superList[key]);
                  }
               }
            }
         });
      }
   }
};

weightHeight = {

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
         if (isSink(objects[key])) {
            sinkFound = true;
            findHeight(objects[key]);
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

weightContagious = {

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

weighObject = function (object) {
   var value = 1;
   Object.keys(weightList).forEach(function (key) {
      value *= weightList[key].calc(object);
   });
   dmz.object.scalar(object, WeightHandle, value);
};

logDefenderTerm = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
      object.gamma;
   if (notZero(result)) {
      result = object.cost / result;
      if (notZero(object.gamma) && notZero(result)) {
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

logDefender = function (object) {
   var result = object.weight * object.threat * object.consequence * object.vul *
      object.gamma;
   if (notZero(result) && notZero(object.cost)) {
      result = Math.log(object.cost / result);
   }
   else {
      result = 0;
   }
   return result;
};

allocatePreventionBudget = function (handleList, budget, maxBudget, vinf) {
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
         A = A + logDefenderTerm(objectList[key]);
         if (notZero(objectList[key].gamma)) {
            B = B + (objectList[key].cost / objectList[key].gamma);
         }
      });
      totalAllocation = 0;
      logLamda = 0;
      if (notZero(B)) {
         logLamda = (-budget - A) / B;
      }
      Object.keys(objectList).forEach(function (key) {
         object = objectList[key];
         A = 0;
         if (notZero(objectList[key].gamma)) {
            A = object.cost / object.gamma;
         }
         B = logDefender(object);
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
         while (notZero(remainder) && (count <= size))  {
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

rankObject = function (object) {
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

receiveRank = function () {
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
      weighObject(objects[key]);
   });
   allocatePreventionBudget(objects, preventionBudget, maxPreventionBudget, vinf);
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
         calcRiskReduced(objects[key]);
         object = { handle: objects[key] };
         object.rank = rankObject(object.handle);
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

receiveHide = function () {
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

updateObjectiveGraph = function () {
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
            weighObject(objects[object]);
         });
         allocatePreventionBudget(objects, budget, maxPreventionBudget, vinf);
         result = 0;
         if (objective) {
            Object.keys(objects).forEach(function (key) {
               calcRiskReduced(objects[key]);
               array = objective(objects[key]);
               result += array[0];
            });
         }
         if (result > max) {
            max = result;
         }
         list[ix] = result;
      }
      if (max > 0) {
         for (ix = 0; ix <= barCount; ix += 1) {
            list[ix] = Math.ceil(list[ix] / max * 100);
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
         doRank();
      }
      else {
         receiveHide();
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
         doRank();
      }
   }
});

// function receive_response_budget
responseBudgetMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      responseBudget = data.number("Budget", 0);
      maxResponseBudget = data.number("Budget", 1);
      if (visible) {
         doRank();
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
         doRank();
      }
   }
});

updateObjectiveGraphMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      updateGraph = data.boolean("Boolean", 0);
      updateObjectiveGraph();
   }
});

dmz.object.create.observe(self, function (handle, objType, varity) {
   if (objType) {
      if (objType.isOfType(NodeType) || objType.isOfType(NodeLinkType)) {
         objects[handle] = handle;
         if (visible && objects[handle]) {
            doRank();
         }
         doGraph();
      }
   }
});

updateObjectScalar = function (handle) {
   if (visible && objects[handle]) {

      doRank();
   }
   calcRiskInitial(handle);
   doGraph();
};

dmz.object.scalar.observe(self, ThreatHandle, updateObjectScalar);
dmz.object.scalar.observe(self, VulnerabilityHandle, updateObjectScalar);
dmz.object.scalar.observe(self, PreventionCostHandle, updateObjectScalar);
dmz.object.scalar.observe(self, ConsequenceHandle, updateObjectScalar);
dmz.object.scalar.observe(self, DegreeHandle, updateObjectScalar);

updateSimulatorFlag = function (handle, attr, value) {
   if (value) {
      if (attr == WeightDegreesHandle) {
         weightList[WeightDegreesHandle] = weightDegrees;
      }
      else if (attr == WeightBetweennessHandle) {
         weightList[WeightBetweennessHandle] = weightBetweenness;
      }
      else if (attr == WeightHeightHandle) {
         weightList[WeightHeightHandle] = weightHeight;
      }
      else if (attr == WeightContagiousHandle) {
         weightList[WeightContagiousHandle] = weightContagious;
      }
      else if (attr == ObjectiveNoneHandle) {
         objective = calcObjectiveNone;
      }
      else if (attr == ObjectiveRiskHandle) {
         objective = calcObjectiveRisk;
      }
      else if (attr == ObjectiveTxVHandle) {
         objective = calcObjectiveTxV;
      }
      else if (attr == ObjectiveThreatHandle) {
         objective = calcObjectiveThreat;
      }
      else if (attr == ObjectiveVulnerabilityHandle) {
         objective = calcObjectiveVulnerability;
      }
      doGraph();
   }
   else if (weightList[attr]) {
      delete weightList[attr];
      doGraph();
   }
   if (visible) {
      doRank();
   }
};

dmz.object.flag.observe(self, WeightDegreesHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, WeightBetweennessHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, WeightHeightHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, WeightContagiousHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, ObjectiveNoneHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, ObjectiveRiskHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, ObjectiveTxVHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, ObjectiveThreatHandle, updateSimulatorFlag);
dmz.object.flag.observe(self, ObjectiveVulnerabilityHandle, updateSimulatorFlag);


dmz.object.destroy.observe(self, function (handle) {
   var updateRank = false;
   if (visible && objects[handle]) {
      updateRank = true;
   }
   delete objects[handle];
   if (updateRank) {
      doRank();
   }
   doGraph();
});

dmz.object.link.observe(self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) {
      doRank();
   }
   doGraph();
});

dmz.object.unlink.observe(self, LinkHandle, function (link, attr, Super, sub) {
   if (visible && objects[Super]) {
      doRank();
   }
   doGraph();
});

dmz.object.linkAttributeObject.observe(self, LinkHandle,
function (link, attr, Super, sub, object) {
   if (visible && object && objects[object]) {
      doRank();
   }
   doGraph();
});

dmz.object.state.observe(self, LinkFlowHandle, function (object) {
   if (visible && object && objects[object]) {
      doRank();
   }
   doGraph();
});
