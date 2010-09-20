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
   , EliminationHandle = dmz.defs.createNamedHandle("FT_Threat_Elimination_Cost")
   , ConsequenceHandle = dmz.defs.createNamedHandle("FT_Threat_Consequence")
   , ThreatHandle = dmz.defs.createNamedHandle("FT_Threat_Value")
   , VulnerabilityHandle = dmz.defs.createNamedHandle("FT_Vulnerability_Value")
   , VulnerabilityReducedHandle = dmz.defs.createNamedHandle(
         "FT_Vulnerability_Reduced_Value")
   , RiskHandle = dmz.defs.createNamedHandle("FT_Risk_Value")
   , RiskReducedHandle = dmz.defs.createNamedHandle("FT_Risk_Reduced_Value")
   , VulnerabilitySumHandle = dmz.defs.createNamedHandle("FT_Vulnerability_Sum_Value")
   , VulnerabilitySumReducedHandle = dmz.defs.createNamedHandle(
      "FT_Vulnerability_Sum_Reduced_Value")
   , RiskSumHandle = dmz.defs.createNamedHandle("FT_Risk_Sum_Value")
   , RiskSumReducedHandle = dmz.defs.createNamedHandle("FT_Risk_Sum_Reduced_Value")
   , AllocationHandle = dmz.defs.createNamedHandle("FT_Threat_Allocation")
   , FTLinkHandle = dmz.defs.createNamedHandle("FT_Link")
   , FTLogicLinkHandle = dmz.defs.createNamedHandle("FT_Logic_Link")
   , FTActiveFaultTree = dmz.defs.createNamedHandle("FT_Active_Fault_Tree")
   , BudgetHandle = dmz.defs.createNamedHandle("FT_Budget")

   , AndState = 1
   , OrState = 2
   , XOrState = 3
   , AndMask = dmz.defs.lookupState("FT_Logic_And")
   , OrMask = dmz.defs.lookupState("FT_Logic_Or")
   , XOrMask = dmz.defs.lookupState("FT_Logic_XOr")
   , LogicMask = AndMask.or(OrMask.or(XOrMask))
   , ThreatType = dmz.objectType.lookup("ft_threat")
   , ComponentType = dmz.objectType.lookup("ft_component")
   , ComponentRootType = dmz.objectType.lookup("ft_component_root")
   , simMessage = dmz.message.create(
      self.config.string("simulator-message.name", "FTSimulatorMessage"))
   , budgetMessage = dmz.message.create(
      self.config.string("budget-message.name", "FTBudgetMessage"))
   , vinfinityMessage = dmz.message.create(
        self.config.string(
           "v-infinity-message.name",
           "FTVulnerabilityInfinityMessage"))
   , index = []
   , objects = {}
   , budget = 0
   , maxBudget = 0
   , vinf = 0.05
   , reset = null
   , root = null
   , risk = 0
   , work = null
   , haveSetTimer = false
   , traverseDepth = 0
   , buildIndex
   , traverseFaultTree
   , notZero = dmz.util.isNotZero
   , logDefenderTerm
   , logDefender
   , stopWork
   , round = Math.round
   , getLogicState
   , newObject
   , updateVulnerabilityReduced
   , buildIndex
   , riskSub
   , riskXor
   , vulnerabilityAnd
   , vulnerabilityOr
   , vulnerabilityXor
   , traverseFaultTree
   , logDefenderTerm
   , logDefender
   , stopPlugin

   , Level = []
   , AllLevel

   , startWork
   ;

(function () {
   Level[0] = dmz.defs.lookupState("FT_Threat_Level_0");
   Level[1] = dmz.defs.lookupState("FT_Threat_Level_1");
   Level[2] = dmz.defs.lookupState("FT_Threat_Level_2");
   Level[3] = dmz.defs.lookupState("FT_Threat_Level_3");
   Level[4] = dmz.defs.lookupState("FT_Threat_Level_4");
   Level[5] = dmz.defs.lookupState("FT_Threat_Level_5");
   Level[6] = dmz.defs.lookupState("FT_Threat_Level_6");
   Level[7] = dmz.defs.lookupState("FT_Threat_Level_7");
   Level[8] = dmz.defs.lookupState("FT_Threat_Level_8");
   Level[9] = dmz.defs.lookupState("FT_Threat_Level_9");
   Level[10] = dmz.defs.lookupState("FT_Threat_Level_10");
   AllLevel = Level[0].or(Level[1]).or(Level[2]).or(Level[3]).or(Level[4])
      .or(Level[5]).or(Level[6]).or(Level[7]).or(Level[8]).or(Level[9]).or(Level[10]);
}());

startWork = function () {
   var vulnerability
     , A
     , B
     , totalAllocation
     , logLamda
     , C
     , D
     , scale
     , object
     , max
     , size
     , remainder
     , count
     ;
   index = [];
   buildIndex(root);
   index.forEach(function (key) {
      dmz.object.scalar(key.handle, AllocationHandle, 0);
      key.gamma = 0;
   });
   if (root) {
      risk = 0;
      vulnerability = traverseFaultTree(root);
      if (!vulnerability) {
         vulnerability = 0;
      }
      dmz.object.scalar(root, RiskSumHandle, risk);
      dmz.object.scalar(root, RiskSumReducedHandle, risk);
      dmz.object.scalar(root, VulnerabilitySumHandle, vulnerability);
      dmz.object.scalar(root, VulnerabilitySumReducedHandle, vulnerability);
   }
   if (notZero(budget)) {
      A = 0;
      B = 0;
      index.forEach(function (key) {
         if (key.vulnerability <= 0) {
            key.vulnerability = 1;
         }
         key.gamma = -Math.log(vinf / key.vulnerability);
         if (key.gamma < 0) {
            key.gamma = 0;
         }
         A = A + logDefenderTerm(key);
         if (notZero(key.gamma)) {
            B = B + (key.cost / key.gamma);
         }
      });
      totalAllocation = 0;
      logLamda = 0;
      if (notZero(B)) {
         logLamda = (-budget - A) / B;
      }
      index.forEach(function (key) {
         C = 0;
         if (notZero(key.gamma)) {
            C = key.cost / key.gamma;
         }
         D = logDefender(key);
         key.allocation = -C * (logLamda + D);
         if (key.allocation < 0) {
            key.allocation = 0;
         }
         if (key.allocation > key.cost) {
            key.allocation = key.cost;
         }
         totalAllocation += key.allocation;
      });
      scale = 1;
      if (totalAllocation < budget) {
         remainder = budget - totalAllocation;
         for (count = 0; (count < index.length) && notZero(remainder); count += 1) {
            object = index[count];
            max = object.cost - object.allocation;
            if (max > remainder) {
               max = remainder;
               remainder = 0;
            }
            else {
               remainder -= max;
            }
            object.allocation += max;
         }
      }
      else {
         scale = budget / totalAllocation;
      }
      totalAllocation = 0;
      index.forEach(function (key) {
         key.allocation *= scale;
         totalAllocation += key.allocation;
         dmz.object.scalar(key.handle, AllocationHandle, key.allocation);
      });
      if (root) {
         risk = 0;
         traverseDepth = 0;
         vulnerability = traverseFaultTree(root);
         dmz.object.scalar(root, RiskSumReducedHandle, risk);
         dmz.object.scalar(root, VulnerabilitySumReducedHandle, vulnerability);
      }
   }
   if (!haveSetTimer) {
      haveSetTimer = true;
      work = dmz.time.setRepeatingTimer(self, function () {
         var source
           , target
           , sourceAllocation
           , targetAllocation
           , maxAddition
           , addition
           , riskOrig
           , vulnerabilityOrig
           , vulnerability
           ;
         if (reset) {
            startWork();
         }
         size = index.length;
         if (size > 1) {
            source = dmz.util.randomInt(0, size - 1);
            target = source;
            count = 0;
            while ((count < 12) && (source == target)) {
               count += 1;
               target = dmz.util.randomInt(0, size - 1);
            }
            source = index[source];
            target = index[target];
            if (source && target && source.handle != target.handle) {
               sourceAllocation = source.allocation;
               targetAllocation = target.allocation;
               maxAddition = target.cost - target.allocation;
               if (maxAddition > source.allocation) {
                  maxAddition = source.allocation;
               }
               if (maxAddition >= 0.01) {
                  addition = 0.01 + ((maxAddition - 0.01) * Math.random());
                  dmz.object.scalar(
                     source.handle,
                     AllocationHandle,
                     source.allocation - addition);
                  dmz.object.scalar(
                     target.handle,
                     AllocationHandle,
                     target.allocation + addition);
                  riskOrig = dmz.object.scalar(root, RiskSumReducedHandle);
                  vulnerabilityOrig = dmz.object.scalar(
                     root,
                     VulnerabilitySumReducedHandle);
                  risk = 0;
                  traverseDepth = 0;
                  vulnerability = traverseFaultTree(root);
                  if (riskOrig < risk) {
                     dmz.object.scalar(
                        source.handle,
                        AllocationHandle,
                        sourceAllocation);
                     dmz.object.scalar(
                        target.handle,
                        AllocationHandle,
                        targetAllocation);
                  }
                  else {
                     dmz.object.scalar(root, RiskSumReducedHandle, risk);
                     dmz.object.scalar(
                        root,
                        VulnerabilitySumReducedHandle,
                        vulnerability);
                  }
               }
            }
         }
      });
   }
   reset = null;
}

stopWork = function () {
   reset = null;
   dmz.time.cancelTimer(self, work);
   haveSetTimer = false;
}

getLogicState = function (node) {
   var logicTable = dmz.object.subLinks(node, FTLogicLinkHandle)
     , logic = OrState
     , logicState
     ;
   if (logicTable) {
      logicState = dmz.object.state(logicTable[1]);
      if (logicState) {
         if (logicState.contains(AndMask)) {
            logic = AndState;
         }
         else if (logicState.contains(XOrMask)) {
            logic = XOrState;
         }
      }
   }
   return logic;
}

newObject = function (handle) {

   return {
      handle: handle,
      cost: 0,
      threat: 0,
      vulnerability: 0,
      consequence: 0,
      allocation: 0,
      gamma: 0,
      vreduced: 0
   };
}

updateVulnerabilityReduced = function (object) {
   var value
     , state
     ;
   if (object) {
      value = 0;
      if (object.cost > 0) {
         value = object.vulnerability *
            Math.exp(-object.gamma * object.allocation / object.cost);
      }
      object.vreduced = value;
      dmz.object.scalar(
         object.handle,
         RiskHandle,
         object.vulnerability * object.consequence * object.threat);
      dmz.object.scalar(
         object.handle,
         RiskReducedHandle,
         value * object.consequence * object.threat);
      dmz.object.scalar(object.handle, VulnerabilityReducedHandle, value);
      state = dmz.object.state(object.handle);
      if (!state) {
         state = dmz.mask.create();
      }
      state = state.unset(AllLevel);
      if (object.vulnerability > 0) {
         if (value < 0) {
            value = 0;
         }
         value = round((value / object.vulnerability) * 10);
         if (value < 0) {
            value = 0;
         }
         else if (value > 10) {
            value = 10;
         }
         state = state.set(Level[value]);
      }
      dmz.object.state(object.handle, null, state);
   }
}

dmz.object.create.observe(self, function (objHandle, objType) {
   var consequenceOld
     , eliminationOld
     , unitState
     , stateMultiplier = 1
     ;
   if (objType.isOfType(ThreatType)) {
      objects[objHandle] = newObject(objHandle);
		reset = true;
   } else if (objType.isOfType(ComponentType)) {
      reset = true;
   }
});

dmz.object.destroy.observe(self, function (objHandle) {
   if (objHandle != null) {
      if (objects[objHandle]) {
         delete objects[objHandle];
         reset = true;
      }
      else if (dmz.object.type(objHandle) &&
            dmz.object.type(objHandle).isOfType(ComponentType)) {
         reset = true;
      }
      if (objHandle == root) {
         root = null;
      }
   }
});

dmz.object.state.observe(self, function (objHandle, attrHandle, value, prevValue) {
   if (value && prevValue) {
      if (!(value.and(LogicMask)).equal(prevValue.and(LogicMask))) {
         reset = true;
      }
	}
});

dmz.object.link.observe(self, FTLinkHandle, function (link, attr, Super, sub) {
   if (objects[Super] || objects[sub]) {
      reset = true;
   }
});

dmz.object.unlink.observe(self, FTLinkHandle, function (link, attr, Super, sub) {
   if (objects[Super] || objects[sub]) {
      reset = true;
   }
});

dmz.object.flag.observe(self, FTActiveFaultTree, function (object, attrHandle, value) {
   if (value) {
      root = object;
      reset = true;
   }
});

dmz.object.scalar.observe(self, EliminationHandle,
function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].cost = value;
      updateVulnerabilityReduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, ConsequenceHandle,
function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].consequence = value;
      updateVulnerabilityReduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, ThreatHandle, function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].threat = value;
      updateVulnerabilityReduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, VulnerabilityHandle,
function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].vulnerability = value;
      updateVulnerabilityReduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, AllocationHandle,
function (objHandle, attrHandle, value) {
	var object = objects[objHandle];
   if (object) {
		object.allocation = value;
      updateVulnerabilityReduced(objects[objHandle]);
	}
});

buildIndex = function (node) {
   var nodeList = null
     , otype
     , ref
     ;
   if (node) {
      nodeList = dmz.object.subLinks(node, FTLinkHandle);
   }
   if (nodeList) {
      nodeList.forEach(function (key) {
         otype = dmz.object.type(key);
         if (otype.isOfType(ThreatType)) {
            ref = objects[key];
            if (ref) {
               index.push(ref);
            }
         }
         else if (otype.isOfType(ComponentType)) {
            buildIndex(key);
         }
      });
   }
}

riskSub = function (objects) {
   var result = 0;
   objects.forEach(function (key) {
      result += (key.threat * key.vreduced * key.consequence);
   });
   return result;
}

riskXor = function (objects) {
   var result = 0
     , value
     , current
     ;
   objects.forEach(function (ikey) {
      value = 1;
      current = ikey;
      objects.forEach(function (jkey) {
         if (ikey != jkey) {
            value *= (1 - (jkey.threat * jkey.vreduced));
         }
         else {
            value *= jkey.vreduced;
         }
      });
      result = result + (value * current.consequence);
   });
   return result;
}

vulnerabilityAnd = function (subv) {
   var result = 1;
   subv.forEach(function (key) {
      result *= key.vt;
   });
   return result;
}

vulnerabilityOr = function (subv) {
   var result = 0;
   if (subv.length == 1) {
      result = subv[0].vt;
   }
   else if (subv.length > 1) {
      result = 1;
      subv.forEach(function (key) {
         result *= (1 - key.vt);
      });
      result = 1 - result;
   }
   return result;
}

vulnerabilityXor = function (subv) {
   var result = 0
     , product
     ;
   subv.forEach(function (ikey) {
      product = 1;
      subv.forEach(function (jkey) {
         if (jkey != ikey) {
            product *= (1 - jkey.vt);
         }
         else {
            product *= jkey.vreduced;
         }
      });
      result += product;
   });
   return result;
}

traverseFaultTree = function (node) {
   var result = null
     , nodeList = dmz.object.subLinks(node, FTLinkHandle)
     , threatList
     , subv
     , otype
     , ref
     , value
     , op
     ;
   if (!nodeList) {
      nodeList = [];
   }
   if (nodeList.length > 0) {
      threatList = [];
      subv = [];
      nodeList.forEach(function (key) {
         otype = dmz.object.type(key);
         if (otype.isOfType(ThreatType)) {
            ref = objects[key];
            if (ref) {
               subv.push({ vt: ref.threat * ref.vreduced, vreduced: ref.vreduced });
               threatList.push(ref);
            }
         }
         else if (otype.isOfType(ComponentType)) {
            value = traverseFaultTree(key);
            if (value) {
               subv.push({ vt: value, vreduced: value });
            }
         }
      });
      op = getLogicState(node);
      result = 0;
      switch (op) {
         case AndState:
            risk += riskSub(threatList);
            result = vulnerabilityAnd(subv);
            break;
         case OrState:
            risk += riskSub(threatList);
            result = vulnerabilityOr(subv);
            break;
         case XOrState:
            risk += riskXor(threatList);
            result = vulnerabilityXor(subv);
            break;
         default: self.log.error("Unknown logic operator: " + op); break;
      }
   }
   return result;
};

logDefenderTerm = function (object) {
   var result = object.threat * object.consequence * object.vulnerability *
      object.gamma;
   if (result > 0) {
      result = object.cost / result;
      if ((object.gamma > 0) && (result > 0)) {
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
}

logDefender = function (object) {
   var result = object.threat * object.consequence * object.vulnerability *
      object.gamma;
   if (result > 0) {
      result = Math.log(object.cost / result);
   }
   else {
      result = 0;
   }
   return result;
}

simMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
         startWork();
      }
      else {
         stopWork();
      }
   }
});

budgetMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      budget = data.number(BudgetHandle, 0);
      if (!budget) {
         budget = 0;
      }
      maxBudget = data.number(BudgetHandle, 1);
      if (!maxBudget) {
         maxBudget = 0;
      }
      reset = true;
   }
});

vinfinityMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      vinf = data.number("value", 0);
      if (!vinf || (vinf <= 0)) {
         vinf = 0.05;
      }
      reset = true;
   }
});

stopPlugin = function () {
   dmz.time.cancelTimer(self, work);
   haveSetTimer = false;
}
