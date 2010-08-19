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
   , ConsequenceHandle = dmz.defs.createNamedHandle("NA_Node_Consequence")
   , ThreatHandle = dmz.defs.createNamedHandle("NA_Node_Threat")
   , VulnerabilityHandle = dmz.defs.createNamedHandle("NA_Node_Vulnerability")

   , LinkFlowHandle = dmz.defs.createNamedHandle("NA_Link_Flow")
   , ForwardFlowState = dmz.defs.lookupState("NA_Flow_Forward")
   , ReverseFlowState = dmz.defs.lookupState("NA_Flow_Reverse")
   , FlowStateBoth = ForwardFlowState.or(ReverseFlowState)

   , CascadeBarNumberHandle = dmz.defs.createNamedHandle("NA_Cascade_Bar_Number")
   , CascadeBarValueHandle = dmz.defs.createNamedHandle("NA_Cascade_Bar_Value")
   , CascadeFailUpstreamState = dmz.defs.lookupState("NA_Cascade_Fail_Upstream")
   , CascadeFailDownstreamState = dmz.defs.lookupState("NA_Cascade_Fail_Downstream")
   , CascadeFailBothState = CascadeFailDownstreamState.or(CascadeFailUpstreamState)
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")

   , NodeType = dmz.objectType.lookup("na_node")
   , NodeLinkType = dmz.objectType.lookup("na_link_attribute")

   , cascadeTrialCount = 0
   , cascadePDF = null
   , normalizedCascadePDF = null
   , cascadeEP = null
   , cdf = []
   , objectArray = []

   , simulateMessage = dmz.message.create(
        self.config.string("simulate-message.name", "NASimulateMessage"))
   , simulateDirectionMessage = dmz.message.create(
        self.config.string(
           "simulate-direction-message.name",
           "NASimulateDirectionMessage"))
   , simulateDelayMessage = dmz.message.create(
      self.config.string("simulate-delay-message.name", "NASimulateDelayMessage"))
   , simulateIterCountMessage = dmz.message.create(
        self.config.string(
           "simulate-itercount-message.name",
           "NASimulateIterCountMessage"))
   , simulateLinksMessage = dmz.message.create(
        self.config.string(
           "simulate-allow--links-message.name",
           "NASimulateLinksMessage"))
   , cleanupMessage = dmz.message.create("CleanupObjectsMessage")

   , objectList = {}
   , linkObjectList = {}

   , dataReset = true
   , firstRun = true
   , bars = []
   , barCount = 100
   , failureType = CascadeFailBothState
   , updateGraphDelay = 500
   , allowLinks = true

   , cascadeFailureSimulation
   , dataUpdated
   , objectLinkList
   , cascadeInit
   , cascadeCDF
   , checkObjectCascadeFail
   , updateCascadeGraph


   , startTime = dmz.time.getSystemTime()
   ;

dmz.object.create.observe(self, function (handle, objType) {
   if (objType) {
      if (objType.isOfType(NodeType) || objType.isOfType(NodeLinkType)) {
         dataReset = true;
      }
   }
});

dataUpdated = function (objHandle) {
   if (objHandle && (objectList[objHandle] || linkObjectList[objHandle])) {
      dataReset = true;
   }
};

dmz.object.scalar.observe(self, ConsequenceHandle, dataUpdated);
dmz.object.scalar.observe(self, ThreatHandle, dataUpdated);
dmz.object.scalar.observe(self, VulnerabilityHandle, dataUpdated);
dmz.object.state.observe(self, LinkFlowHandle, dataUpdated);

dmz.object.destroy.observe(self, function (handle) {
   var list = objectList[handle]
     ;
   if (list) {
      delete objectList[handle];
      dataReset = true;
   }
});

dmz.object.linkAttributeObject.observe(self, LinkHandle,
function (linkHandle, AttrHandle, Super, Sub, AttrObj, PrevObj) {
   var link
     ;
   if (AttrObj) {
      link = { superLink: Super, sub: Sub, attr: AttrObj, handle: linkHandle };
      if (!objectList[Super]) {
         objectList[Super] = {};
      }
      objectList[Super][linkHandle] = link;
      if (!objectList[Sub]) {
         objectList[Sub] = {};
      }
      objectList[Sub][linkHandle] = link;
      linkObjectList[AttrObj] = link;
   }
   else if (objectList[Super] && objectList[Super][linkHandle] &&
         objectList[Super][linkHandle].attr == PrevObj) {
      delete linkObjectList[PrevObj];
      delete objectList[Super][linkHandle];
      delete objectList[Sub][linkHandle];

   }
   if (PrevObj && dmz.object.isObject(PrevObj)) {
      dmz.object.destroy(PrevObj);
   }
   dataReset = true;
});

objectLinkList = function (objHandle) {
   var list = objectList[objHandle]
     , result = []
     ;
   if (list) {
      Object.keys(list).forEach(function (index) {
         result.push(list[index]);
      });
   }
   return result;
};

cascadeInit = function () {
   var sumTV = 0
     , pdf = []
     , threat
     , vuln
     , count
     , handle
     , key
     , ix
     ;

   simulateIterCountMessage.send(dmz.data.wrapNumber(0));
   if (firstRun) {

      for (ix = 0; ix <  barCount; ix += 1) {
         bars[ix] = dmz.object.create("na_cascade_bar");
         dmz.object.counter(bars[ix], CascadeBarNumberHandle, ix + 1);
         dmz.object.counter(bars[ix], CascadeBarValueHandle, 0);
         dmz.object.activate(bars[ix]);
      }
      firstRun = false;
   }

   cascadeTrialCount = 0;
   cascadeEP = [];
   cascadePDF = [];
   for (count = 0; count <= 100; count += 1) {
      cascadeEP[count] = 0;
      cascadePDF[count] = 0;
   }

   cdf = [];
   objectArray = [];
   count = 0;
   Object.keys(objectList).forEach(function (key) {
      handle = parseInt(key);
      threat = dmz.object.scalar(handle, ThreatHandle);
      vuln = dmz.object.scalar(handle, VulnerabilityHandle);
      pdf[count] = threat * vuln;
      sumTV += pdf[count];
      objectArray[count] = handle;
      count += 1;
   });

   if (allowLinks) {
      Object.keys(linkObjectList).forEach(function (key) {
         threat = dmz.object.scalar(linkObjectList[key].attr, ThreatHandle);
         vuln = dmz.object.scalar(linkObjectList[key].attr, VulnerabilityHandle);
         pdf[count] = threat * vuln;
         sumTV += pdf[count];
         objectArray[count] = linkObjectList[key];
         count += 1;
      });
   }

   cdf[0] = pdf[0] / sumTV;
   for (key = 1; key < pdf.length; key += 1) {
      cdf[key] = cdf[key - 1] + (pdf[key] / sumTV);
   }

   dataReset = false;
};

cascadeCDF = function () {
   var key
     , random = Math.random()
     , result = null
     ;

   if (cdf.length == 1) {
      result = objectArray[0];
   }

   for (key = 0; (result === null) && (key < cdf.length); key += 1) {
      if (random <= cdf[key]) {
         result = objectArray[key];
      }
   }
   return result;
};

checkObjectCascadeFail = function (objectHandle) {
   return Math.random() <= (dmz.object.scalar(objectHandle, VulnerabilityHandle) *
                           dmz.object.scalar(objectHandle, ThreatHandle));
};

cascadeFailureSimulation = function () {
   var totalConsequences = 0
     , failedConsequences = 0
     , initFailure
     , list = []
     , visited = {}
     , current
     , link
     , linkList
     , counter
     , linkState
     ;

   if (dataReset) {
      cascadeInit();
   }

   initFailure = cascadeCDF();

   if (initFailure && !objectList[initFailure]) {
      failedConsequences += dmz.object.scalar(initFailure.attr, ConsequenceHandle);
      linkState = dmz.object.state(initFailure.attr, LinkFlowHandle);
      if (!linkState) {
         linkState = FlowStateBoth;
      }
      if (failureType.and(CascadeFailDownstreamState).bool()) {

         if (ForwardFlowState.and(linkState).bool()) {

            if (checkObjectCascadeFail(initFailure.sub)) {

               list.push(initFailure.sub);
            }

            visited[initFailure.sub] = true;
         }
         else if (ReverseFlowState.and(linkState).bool()) {

            if (checkObjectCascadeFail(initFailure.superLink)) {
               list.push(initFailure.superLink);
            }
            visited[initFailure.superLink] = true;
         }
      }
      else if (failureType.and(CascadeFailUpstreamState).bool()) {

         if (ForwardFlowState.and(linkState).bool()) {

            if (checkObjectCascadeFail(initFailure.superLink)) {
               list.push(initFailure.superLink);
            }
            visited[initFailure.superLink] = true;
         }
         else if (ReverseFlowState.and(linkState).bool()) {

            if (checkObjectCascadeFail(initFailure.sub)) {
               list.push(initFailure.sub);
            }
            visited[initFailure.sub] = true;
         }
      }
      visited[initFailure.attr] = true;
   }
   else if (initFailure) {
      list.push(initFailure);
      visited[initFailure] = true;
   }

   while (list.length > 0) {
      current = list.shift();
      linkList = objectLinkList(current);

      Object.keys(linkList).forEach(function (linkobj) {
         link = linkList[linkobj];

         if (!visited[link.attr]) {
            visited[link.attr] = true;
            if (!allowLinks || checkObjectCascadeFail(link.attr)) {
               if (allowLinks) {
                  failedConsequences += dmz.object.scalar(link.attr, ConsequenceHandle);
               }
               linkState = dmz.object.state(link.attr, LinkFlowHandle);
               if (!linkState) {
                  linkState = FlowStateBoth;
               }

               if (failureType.and(CascadeFailDownstreamState).bool()) {

                  if (ForwardFlowState.and(linkState).bool() &&
                      link.superLink == current && !visited[link.sub]) {

                     if (checkObjectCascadeFail(link.sub)) {
                        list.push(link.sub);
                     }
                     visited[link.sub] = true;
                  }
                  else if (ReverseFlowState.and(linkState).bool() &&
                           link.sub == current && !visited[link.superLink]) {

                     if (checkObjectCascadeFail(link.superLink)) {
                        list.push(link.superLink);
                     }
                     visited[link.superLink] = true;
                  }
               }
               else if (failureType.and(CascadeFailUpstreamState).bool()) {

                  if (ForwardFlowState.and(linkState).bool() &&
                      link.sub == current && !visited[link.superLink]) {

                     if (checkObjectCascadeFail(link.superLink)) {
                        list.push(link.superLink);
                     }
                     visited[link.superLink] = true;
                  }
                  else if (ReverseFlowState.and(linkState).bool() &&
                           link.superLink == current && !visited[link.sub]) {

                     if (checkObjectCascadeFail(link.sub)) {
                        list.push(link.sub);
                     }
                     visited[link.sub] = true;
                  }
               }
            }
         }
      });

      failedConsequences += dmz.object.scalar(current, ConsequenceHandle);
   }

   Object.keys(objectList).forEach(function (key) {
      totalConsequences += dmz.object.scalar(parseInt(key), ConsequenceHandle);
   });

   if (allowLinks) {
      Object.keys(objectLinkList).forEach(function (key) {
         totalConsequences += dmz.object.scalar(parseInt(key), ConsequenceHandle)
      });
   }

   if (totalConsequences > 0) {
      failedConsequences = Math.round(failedConsequences / totalConsequences * 100);
   }
   else {
      failedConsequences = 0;
   }

   cascadePDF[failedConsequences] += 1;
   cascadeTrialCount += 1;

   cascadeEP[100] = cascadePDF[100] / cascadeTrialCount;
   for (counter = 99; counter >= 1; counter -= 1) {
      cascadeEP[counter] = (cascadePDF[counter] / cascadeTrialCount) +
                           cascadeEP[counter + 1];
   }

   if ((cascadeTrialCount % updateGraphDelay) === 0) {
      updateCascadeGraph();
   }
};

updateCascadeGraph = function () {
   var ix;

   simulateIterCountMessage.send(dmz.data.wrapNumber(cascadeTrialCount));
   for (ix = 0; ix < barCount; ix += 1) {
      dmz.object.counter(bars[ix], CascadeBarValueHandle, cascadeEP[ix + 1] * 100);
   }
};

simulateMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
         dmz.time.setRepeatingTimer(self, cascadeFailureSimulation);
         //cascadeFailureSimulation();
      }
      else if (!firstRun) {
         dmz.time.cancleTimer(self, cascadeFailureSimulation);
         updateCascadeGraph();
      }
   }
});

simulateDelayMessage.subscribe(self, function (data) {
   updateGraphDelay = dmz.data.unwrapNumber(data);
});

simulateLinksMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      allowLinks = dmz.data.unwrapBoolean(data);
      dataReset = true;
   }
});

simulateDirectionMessage.subscribe(self, function (data) {
   var stateString;
   if (dmz.data.isTypeOf(data)) {
      stateString = data.string("String", 0);
      if (stateString === "Upstream") {
         failureType = CascadeFailUpstreamState;
      }
      else if (stateString === "Downstream") {
         failureType = CascadeFailDownstreamState;
      }
      else {
         failureType = CascadeFailBothState;
      }
      dataReset = true;
   }
});
