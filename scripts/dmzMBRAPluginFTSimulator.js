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

   , MoneyUnitHandle = dmz.defs.createNamedHandle("FT_Monetary_Units")
   , MoneyUnitTrillionsState = dmz.defs.lookupState("FT_Money_Unit_Trillions")
   , MoneyUnitBillionsState = dmz.defs.lookupState("FT_Money_Unit_Billions")
   , MoneyUnitMillionsState = dmz.defs.lookupState("FT_Money_Unit_Millions")
   , MoneyUnitThousandsState = dmz.defs.lookupState("FT_Money_Unit_Thousands")
   , MoneyUnitExactValueState = dmz.defs.lookupState("FT_Money_Unit_Exact_Value")

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
   , vinfinityMessage = dmz.message.create(self.config.string(
      "v-infinity-message.name", "FTVulnerabilityInfinityMessage"))
   , index = []
   , objects = []
   , budget = 0
   , maxBudget = 0
   , vinf = 0.05
   , reset = null
   , root = null
   , risk = 0
   , work = null
   , haveSetTimer = false
   , traverseDepth = 0
   , build_index
   , traverse_fault_tree
   , not_zero
   , log_defender_term
   , log_defender
   ;

var Level = [];
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
var AllLevel = Level[0].or(Level[1].or(Level[2].or(Level[3].or(Level[4].or(Level[5] +
            Level[6].or(Level[7].or(Level[8].or(Level[9].or(Level[10])))))))));

var get_money_unit_from_state = function (state) {
   var result = 1;
   if (state.and(MoneyUnitTrillionsState).bool()) {
      result = 1000000000000;
   }
   else if (state.and(MoneyUnitBillionsState).bool()) {
      result = 1000000000;
   }
   else if (state.and(MoneyUnitMillionsState).bool()) {
      result = 1000000;
   }
   else if (state.and(MoneyUnitThousandsState).bool()) {
      result = 1000;
   }
   return result;
}

function start_work() {
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
   build_index(root);
   Object.keys(index).forEach(function (key) {
      dmz.object.scalar(index[key].handle, AllocationHandle, 0);
      index[key].gamma = 0;
   });
   if (root) {
      risk = 0;
      vulnerability = traverse_fault_tree(root);
      if (!vulnerability) {
         vulnerability = 0;
      }
      dmz.object.scalar(root, RiskSumHandle, risk);
      dmz.object.scalar(root, RiskSumReducedHandle, risk);
      dmz.object.scalar(root, VulnerabilitySumHandle, vulnerability);
      dmz.object.scalar(root, VulnerabilitySumReducedHandle, vulnerability);
   }
   if (not_zero(budget)) {
      A = 0;
      B = 0;
      Object.keys(index).forEach(function (key) {
         if (index[key].vulnerability <= 0) {
            index[key].vulnerability = 1;
         }
         index[key].gamma = -Math.log(vinf / index[key].vulnerability);
         if (index[key].gamma < 0) {
            index[key].gamma = 0;
         }
         A = A + log_defender_term(index[key]);
         if (not_zero(index[key].gamma)) {
            B = B + (index[key].cost / index[key].gamma);
         }
      });
      totalAllocation = 0;
      logLamda = 0;
      if (not_zero(B)) {
         logLamda = (-budget - A) / B;
      }
      Object.keys(index).forEach(function (key) {
         C = 0;
         if (not_zero(index[key].gamma)) {
            C = index[key].cost / index[key].gamma;
         }
         D = log_defender(index[key]);
         index[key].allocation = -C * (logLamda + D);
         if (index[key].allocation < 0) {
            index[key].allocation = 0;
         }
         if (index[key].allocation > index[key].cost) {
            index[key].allocation = index[key].cost;
         }
         totalAllocation += index[key].allocation;
      });
      scale = 1;
      if (totalAllocation < budget) {
         remainder = budget - totalAllocation;
         for (count = 0; count < index.length && not_zero(remainder); count += 1) {
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
      Object.keys(index).forEach(function (key) {
         index[key].allocation *= scale;
         totalAllocation += index[key].allocation;
         dmz.object.scalar(index[key].handle, AllocationHandle, index[key].allocation);
      });
      if (root) {
         risk = 0;
         traverseDepth = 0;
         vulnerability = traverse_fault_tree(root);
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
            start_work();
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
                  vulnerabilityOrig = dmz.object.scalar(root,
                                                       VulnerabilitySumReducedHandle);
                  risk = 0;
                  traverseDepth = 0;
                  vulnerability = traverse_fault_tree(root);
                  if (riskOrig < risk) {
                     dmz.object.scalar(source.handle, AllocationHandle,
                                        sourceAllocation);
                     dmz.object.scalar(target.handle, AllocationHandle,
                                        targetAllocation);
                  }
                  else {
                     dmz.object.scalar(root, RiskSumReducedHandle, risk);
                     dmz.object.scalar(
                        root, VulnerabilitySumReducedHandle, vulnerability);
                  }
               }
            }
         }
      });
   }
   reset = null;
}

function stop_work() {
   reset = null;
   dmz.time.cancleTimer(self, work);
   haveSetTimer = false;
}


var not_zero = function (value) {
   return dmz.util.isNotZero(value);
};

function round(value) {
   return Math.round(value);
}

function get_logic_state(node) {
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

function new_object(handle) {

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

function update_vulnerability_reduced(object) {
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
      objects[objHandle] = new_object(objHandle);
//      consequenceOld = dmz.object.scalar(objHandle, ConsequenceHandle);
//      eliminationOld = dmz.object.scalar(objHandle, EliminationHandle);
//      unitState = dmz.object.state(objHandle, MoneyUnitHandle);
//      if (unitState) {
//         stateMultiplier = get_money_unit_from_state(unitState);
//      }
//      dmz.object.scalar(objHandle, ConsequenceHandle, consequenceOld * stateMultiplier);
//      dmz.object.scalar(objHandle, EliminationHandle,
//                        eliminationOld * stateMultiplier);
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
      update_vulnerability_reduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, ConsequenceHandle,
function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].consequence = value;
      update_vulnerability_reduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, ThreatHandle, function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].threat = value;
      update_vulnerability_reduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, VulnerabilityHandle,
function (objHandle, attrHandle, value) {
   if (objects[objHandle]) {
		objects[objHandle].vulnerability = value;
      update_vulnerability_reduced(objects[objHandle]);
		reset = true;
	}
});

dmz.object.scalar.observe(self, AllocationHandle,
function (objHandle, attrHandle, value) {
	var object = objects[objHandle];
   if (object) {
		object.allocation = value;
      update_vulnerability_reduced(objects[objHandle]);
	}
});

function build_index(node) {
   var nodeList = null
     , otype
     , ref
     ;
   if (node) {
      nodeList = dmz.object.subLinks(node, FTLinkHandle);
   }
   if (nodeList) {
      Object.keys(nodeList).forEach(function (key) {
         otype = dmz.object.type(nodeList[key]);
         if (otype.isOfType(ThreatType)) {
            ref = objects[nodeList[key]];
            if (ref) {
               index.push(ref);
            }
         }
         else if (otype.isOfType(ComponentType)) {
            build_index(nodeList[key]);
         }
      });
   }
}

function risk_sub(objects) {
   var result = 0;
   Object.keys(objects).forEach(function (key) {
      result += (objects[key].threat * objects[key].vreduced *
                 objects[key].consequence);
   });
   return result;
}

function risk_xor(objects) {
   var result = 0
     , value
     , current
     ;
   Object.keys(objects).forEach(function (ikey) {
      value = 1;
      current = objects[ikey];
      Object.keys(objects).forEach(function (jkey) {
         if (ikey != jkey) {
            value *= (1 - (objects[jkey].threat * objects[jkey].vreduced));
         }
         else {
            value *= objects[jkey].vreduced;
         }
      });
      result = result + (value * current.consequence);
   });
   return result;
}

function vulnerability_and(subv) {
   var result = 1;
   Object.keys(subv).forEach(function (key) {
      result *= subv[key].vt;
   });
   return result;
}

function vulnerability_or(subv) {
   var result = 0;
   if (subv.length == 1) {
      result = subv[0].vt;
   }
   else if (subv.length > 1) {
      result = 1;
      Object.keys(subv).forEach(function (key) {
         result *= (1 - subv[key].vt);
      });
      result = 1 - result;
   }
   return result;
}

function vulnerability_xor(subv) {
   var result = 0
     , product
     ;
   Object.keys(subv).forEach(function (ikey) {
      product = 1;
      Object.keys(subv).forEach(function (jkey) {
         if (jkey != ikey) {
            product *= (1 - subv[jkey].vt);
         }
         else {
            product *= subv[jkey].vreduced;
         }
      });
      result += product;
   });
   return result;
}

var traverse_fault_tree = function (node) {
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
      Object.keys(nodeList).forEach(function (key) {
         otype = dmz.object.type(nodeList[key]);
         if (otype.isOfType(ThreatType)) {
            ref = objects[nodeList[key]];
            if (ref) {
               subv.push({ vt: ref.threat * ref.vreduced, vreduced: ref.vreduced });
               threatList.push(ref);
            }
         }
         else if (otype.isOfType(ComponentType)) {
            value = traverse_fault_tree(nodeList[key]);
            if (value) {
               subv.push({ vt: value, vreduced: value });
            }
         }
      });
      op = get_logic_state(node);
      result = 0;
      switch (op) {
         case AndState:
            risk += risk_sub(threatList);
            result = vulnerability_and(subv);
            break;
         case OrState:
            risk += risk_sub(threatList);
            result = vulnerability_or(subv);
            break;
         case XOrState:
            risk += risk_xor(threatList);
            result = vulnerability_xor(subv);
            break;
         default: self.log.error("Unknown logic operator: " + op); break;
      }
   }
   return result;
};

function log_defender_term(object) {
   var result = object.threat * object.consequence * object.vulnerability *
      object.gamma;
   if (result > 0) {
      result = object.cost / result;
      if (object.gamma > 0 && result > 0) {
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

function log_defender(object) {
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
         start_work();
      }
      else {
         stop_work();
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

function stop_plugin() {
   dmz.time.cancleTimer(self, work); haveSetTimer = false;
}

dmz.object.state.observe(self, MoneyUnitHandle,
function (object, attr, newState, prevState) {
//   var prevMultiplier = 1
//     , newMultiplier = 1
//     , currentConsequence
//     , currentEliminationCost
//     ;
//   if (object && objects[object]) {
//      if (prevState != null) {
//         prevMultiplier = get_money_unit_from_state(prevState);
//      }
//      if (newState) {
//         newMultiplier = get_money_unit_from_state(newState);
//      }
//      currentConsequence = dmz.object.scalar(object, ConsequenceHandle);
//      currentEliminationCost = dmz.object.scalar(object, EliminationHandle);

//      dmz.object.scalar(object, ConsequenceHandle,
//                        currentConsequence * prevMultiplier / newMultiplier);
//      dmz.object.scalar(object, EliminationHandle,
//                        currentEliminationCost * prevMultiplier / newMultiplier);

//   }
});
