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
      self.config.string("simulate-direction-message.name",
                            "NASimulateDirectionMessage"))
   , simulateDelayMessage = dmz.message.create(
      self.config.string("simulate-delay-message.name", "NASimulateDelayMessage"))
   , simulateIterCountMessage = dmz.message.create(
      self.config.string("simulate-itercount-message.name",
                         "NASimulateIterCountMessage"))

   , objectList = []
   , linkObjectList = []

   , dataReset = true
   , continueCascade = false
   , firstRun = true
   , bars = []
   , barCount = 100
   , failureType = CascadeFailBothState
   , updateGraphDelay = 500

   , cascade_failure_simulation


   , startTime = dmz.time.getSystemTime();
   ;

dmz.object.create.observe(self, function(handle, objType) {
   if (objType) {
      if (objType.isOfType(NodeType) || objType.isOfType(NodeLinkType)) {
         dataReset = true;
      }
   }
});

dmz.object.destroy.observe(self, function (handle) {
   var list = objectList[handle]
     , link
     ;
   if (list) {
      Object.keys(list).forEach(function (index) {
         link = list[index];
         if (objectList[link.superLink]) {
            delete objectList[link.superLink][link];
         }
         if (objectList[link.sub]) {
            delete objectList[link.sub][link];
         }
         if (linkObjectList[link.attr]) {
            delete linkObjectList[link.attr];
         }
      });
      dataReset = true;
   }
});

dmz.object.linkAttributeObject.observe(self, LinkHandle,
function (linkHandle, AttrHandle, Super, Sub, AttrObj, PrevObj) {
   var link
     ;
   if (AttrObj) {
      link = { superLink: Super, sub: Sub, attr: AttrObj };
      if (!objectList[Super]) {
         objectList[Super] = [];
      }
      objectList[Super][linkHandle] = link;
      if (!objectList[Sub]) {
         objectList[Sub] = [];
      }
      objectList[Sub][linkHandle] = link;
      linkObjectList[AttrObj] = link;
   }
   else if (objectList[Super] && objectList[Super][linkHandle] &&
                 objectList[Super][linkHandle].attr == PrevObj) {
      delete objectList[Super][linkHandle];
      delete objectList[Sub][linkHandle];
   }
   if (PrevObj && dmz.object.isObject(PrevObj)) {
      dmz.object.destroy(PrevObj);
   }
   dataReset = true;
});

var object_subList = function (objHandle) {
   var list = objectList[objHandle]
     , result = []
     ;
   if (list) {
      Object.keys(list).forEach(function (index) {
         //self.log.warn("Adding to list: " + list[index].sub);
         result.push(list[index]);
      });
   }
   return result;
};

var object_linkList = function (objHandle) {
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

var cascade_init = function () {
   var sumTV = 0
     , pdf = []
     , threat
     , vuln
     , count
     , handle
     , key
     , ix
     ;

   if (firstRun) {
      for (ix = 0; ix <  barCount; ix += 1) {
         bars[ix] = dmz.object.create("na_cascade_bar");
         dmz.object.counter(bars[ix],CascadeBarNumberHandle,ix+1);
         dmz.object.counter(bars[ix],CascadeBarValueHandle,1);
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

   Object.keys(linkObjectList).forEach(function (key) {
      threat = dmz.object.scalar(linkObjectList[key].attr, ThreatHandle);
      vuln = dmz.object.scalar(linkObjectList[key].attr, VulnerabilityHandle);
      pdf[count] = threat * vuln;
      sumTV += pdf[count];
      objectArray[count] = linkObjectList[key];
      count += 1;
   });

   cdf[0] = pdf[0] / sumTV;
   for (key = 1; key < pdf.length; key += 1) {
      cdf[key] = cdf[key - 1] + (pdf[key] / sumTV);
   }

   dataReset = false;
};

var cascade_cdf = function () {
   var key
     , random = Math.random()
     , result = null
     ;

   if (cdf.length == 1) {
      result = objectArray[0];
   }

   for (key = 0; result == null && key < cdf.length; key += 1) {
      if (random <= cdf[key]) {
         result = objectArray[key];
      }
   }
   return result;
};

var check_object_cascade_fail = function (objectHandle) {
   return Math.random() <= (dmz.object.scalar(objectHandle, VulnerabilityHandle) *
                           dmz.object.scalar(objectHandle, ThreatHandle));
};

cascade_failure_simulation = function() {
   var totalConsequences = 0
     , failedConsequences = 0
     , consequence = 0
     , initFailure
     , consequencePercentage
     , list = []
     , visited = {}
     , current
     , link
     , linkList
     , prevIndex
     , curr
     , counter
     , field
     , linkState
     ;

     if (dataReset) {
        cascade_init();
     }

     initFailure = cascade_cdf();


     if (initFailure && !objectList[initFailure]) {
//        self.log.error("Is a link");
        failedConsequences += dmz.object.scalar(initFailure.attr, ConsequenceHandle);
        linkState = dmz.object.state(initFailure.attr, LinkFlowHandle);

        if (failureType.and(CascadeFailDownstreamState).bool()) {

           if (ForwardFlowState.and(linkState).bool()) {

              if (check_object_cascade_fail(initFailure.sub)) {
                 list.push(initFailure.sub);
              }
              visited[initFailure.sub] = true;
           }
           else if (ReverseFlowState.and(linkState).bool()) {

              if (check_object_cascade_fail(initFailure.superLink)) {
                 list.push(initFailure.superLink);
              }
              visited[initFailure.superLink] = true;
           }
        }
        else if (failureType.and(CascadeFailUpstreamState).bool()) {

           if (ForwardFlowState.and(linkState).bool()) {

              if (check_object_cascade_fail(initFailure.superLink)) {
                 list.push(initFailure.superLink);
              }
              visited[initFailure.superLink] = true;
           }
           else if (ReverseFlowState.and(linkState).bool()) {

              if (check_object_cascade_fail(initFailure.sub)) {
                 list.push(initFailure.sub);
              }
              visited[initFailure.sub] = true;
           }
        }
        visited[initFailure.attr] = true;
     }

     else if (initFailure) {
//        self.log.error("Is a node");
        list.push(initFailure);
        visited[initFailure] = true;
     }

//     self.log.warn ("list.length = " + list.length);

     while (list.length > 0) {
        current = list.shift();
        linkList = object_linkList(current);

        Object.keys(linkList).forEach(function (linkobj) {
           link = linkList[linkobj];
           if (!visited[link.attr]) {
              visited[link.attr] = true;
              if (check_object_cascade_fail(link.attr)) {
                 failedConsequences += dmz.object.scalar(link.attr, ConsequenceHandle);
//                 self.log.warn("link: " + link.attr + " state: " + dmz.object.state(link.attr,LinkFlowHandle));
                 linkState = dmz.object.state(link.attr, LinkFlowHandle);

//                 self.log.warn ("linkState: " + ForwardFlowState.and(linkState).bool() +
//                                " " + ReverseFlowState.and(linkState).bool());
                 if (failureType.and(CascadeFailDownstreamState).bool()) {

                    if (ForwardFlowState.and(linkState).bool() &&
                        link.superLink == current && !visited[link.sub]) {

                       if (check_object_cascade_fail(link.sub)) {
                          list.push(link.sub);
                       }
                       visited[link.sub] = true;
                    }
                    else if (ReverseFlowState.and(linkState).bool() &&
                             link.sub == current && !visited[link.superLink]) {

                       if (check_object_cascade_fail(link.superLink)) {
                          list.push(link.superLink);
                       }
                       visited[link.superLink] = true;
                    }
                 }
                 else if (failureType.and(CascadeFailUpstreamState).bool()) {

                    if (ForwardFlowState.and(linkState).bool() &&
                        link.sub == current && !visited[link.superLink]) {

                       if (check_object_cascade_fail(link.superLink)) {
                          list.push(link.superLink);
                       }
                       visited[link.superLink] = true;
                    }
                    else if (ReverseFlowState.and(linkState).bool() &&
                             link.superLink == current && !visited[link.sub]) {

                       if (check_object_cascade_fail(link.sub)) {
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

     Object.keys(visited).forEach(function (key) {
        totalConsequences += dmz.object.scalar(parseInt(key), ConsequenceHandle);
//        self.log.warn("total = " + totalConsequences + " key: " + key);
     });

//     self.log.warn("failed: " + failedConsequences + " total: " + totalConsequences);

     if (totalConsequences > 0) {
        failedConsequences = Math.round(failedConsequences / totalConsequences * 100);
     }
     else {
        failedConsequences = 0;
     }


     cascadePDF[failedConsequences] += 1;
//     Object.keys(cascadePDF).forEach(function (arg) {self.log.warn("pdf["+arg+"] = " + cascadePDF[arg]);});

     cascadeTrialCount += 1;
//     for (counter = 0; counter < cascadePDF.length; counter += 1) {
//        normalizedCascadePDF[counter] = cascadePDF[counter] / cascadeTrialCount;
//     }

     cascadeEP[100] = cascadePDF[100] / cascadeTrialCount;
     for (counter = 99; counter >= 0; counter -= 1) {
           cascadeEP[counter] = (cascadePDF[counter] / cascadeTrialCount) +
                                cascadeEP[counter + 1];
     }

     if (cascadeTrialCount < updateGraphDelay ||
         (cascadeTrialCount % updateGraphDelay == 0)) {
        update_cascade_graph();
     }
};

var update_cascade_graph = function () {
  var ix
    ;

  simulateIterCountMessage.send(dmz.data.wrapNumber(cascadeTrialCount));
//  self.log.warn ("cascadeTrialCount: " + cascadeTrialCount + " : " + (dmz.time.getSystemTime() - startTime));
  for (ix = 0; ix < barCount; ix += 1) {
     dmz.object.counter(bars[ix],CascadeBarValueHandle,cascadeEP[ix]*100);
//     self.log.warn((ix) + ": " + (cascadeEP[ix]*100));
//     dmz.object.activate(bars[ix]);
  }
};

// Temporarily start/stop based on calculation button
simulateMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
         dmz.time.setRepeatingTimer(self, cascade_failure_simulation);
         //cascade_failure_simulation();
      }
      else if (!firstRun){
         dmz.time.cancleTimer(self, cascade_failure_simulation);
         update_cascade_graph();
      }
   }
});

simulateDelayMessage.subscribe(self, function (data) {
   updateGraphDelay = dmz.data.unwrapNumber(data);
});

simulateDirectionMessage.subscribe(self, function (data) {
   var stateString;
   if (dmz.data.isTypeOf(data)) {
      stateString = data.string("String", 0);
      if (stateString == "Upstream") { failureType = CascadeFailUpstreamState; }
      else if (stateString == "Downstream") { failureType = CascadeFailDownstreamState; }
      else { failureType = CascadeFailBothState; }
      dataReset = true;
      dmz.time.cancleTimer(self, cascade_failure_simulation);
      dmz.time.setRepeatingTimer(self, cascade_failure_simulation);
   }
});
