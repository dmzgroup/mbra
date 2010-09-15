var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , defs: require("dmz/runtime/definitions")
      , data: require("dmz/runtime/data")
      , mask: require("dmz/types/mask")
      , nmmatrix: require("nmmatrix")
      , nvector: require("nvector")
      , message: require("dmz/runtime/messaging")
      , util: require("dmz/types/util")
      , time: require("dmz/runtime/time")
      }
   , ConsequenceHandle = dmz.defs.createNamedHandle("NA_Node_Consequence")
   , ThreatHandle = dmz.defs.createNamedHandle("NA_Node_Threat")
   , ReducedVulnerabilityHandle = dmz.defs.createNamedHandle(
        "NA_Node_Vulnerability_Reduced")

   , VulnerabilityHandle = dmz.defs.createNamedHandle("NA_Node_Vulnerability")

   , LinkFlowHandle = dmz.defs.createNamedHandle("NA_Link_Flow")
   , ForwardFlowState = dmz.defs.lookupState("NA_Flow_Forward")
   , ReverseFlowState = dmz.defs.lookupState("NA_Flow_Reverse")
   , FlowStateBoth = ForwardFlowState.or(ReverseFlowState)

   , BarNumberHandle = dmz.defs.createNamedHandle("NA_Simulation_Bar_Number")
   , BarValueHandle = dmz.defs.createNamedHandle("NA_Simulation_Bar_Value")
   , CascadeFailUpstreamState = dmz.defs.lookupState("NA_Cascade_Fail_Upstream")
   , CascadeFailDownstreamState = dmz.defs.lookupState("NA_Cascade_Fail_Downstream")
   , CascadeFailBothState = CascadeFailDownstreamState.or(CascadeFailUpstreamState)
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")

   , NodeType = dmz.objectType.lookup("na_node")
   , NodeLinkType = dmz.objectType.lookup("na_link_attribute")

   , GraphType =
        { CASCADE:
             { count: 0
             , pdf: []
             , function: function () { self.log.warn ("CASCADE GRAPH FUNCTION"); }
             }
        , FLOW:
             { count: 0
             , pdf: []
             , delay: 500
             , function: function () { self.log.warn ("FLOW GRAPH FUNCTION"); }
             , sinkList: {}
             , sourceList: {}
             }
        }

   , GraphActive = false
   , currentType = GraphType.CASCADE
   , cdf = []
   , objectArray = []
   , dataReset = true

   , simulateMessage = dmz.message.create(
        self.config.string("simulate-message.name", "NASimulateMessage"))
   , simulationTypeMessage = dmz.message.create(
        self.config.string("simulation-type-message.name", "NAGraphSimulationType"))
   , errorMessage = dmz.message.create(
        self.config.string("simulation-error-message.name", "SimulationErrorMessage"))
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

   , firstRun = true
   , bars = []
   , barCount = 100
   , failureType = CascadeFailBothState
   , updateGraphDelay = 500
   , allowLinks = true
   , GraphHasSource = false

   , cascadeFailureSimulation
   , dataUpdated
   , graphInit
   , objectFromCDF
   , checkObjectCascadeFail
   , updateGraph
   , flowSimulation
   , graphHasBiLinks
   , graphHasSink
   , objectLinkList
   , isSink
   , allLinksFlow
   , calculateNetworkFlow
   , calculateCapacityMatrix
   , calculateFractionMatrix
   ;


(function () {
   var ix;
   for (ix = 0; ix < barCount; ix += 1) {
      bars[ix] = dmz.object.create("na_simulation_bar");
      dmz.object.counter(bars[ix], BarNumberHandle, ix + 1);
      dmz.object.counter(bars[ix], BarValueHandle, 0);
      dmz.object.activate(bars[ix]);
   }
}());

dmz.object.create.observe(self, function (handle, objType) {
   if (objType) {
      if (objType.isOfType(NodeType) || objType.isOfType(NodeLinkType)) {
         dataReset = true;
      }
   }
});


dataUpdated = function (objHandle, attrHandle, val) {
   if (objHandle && (objectList[objHandle] || linkObjectList[objHandle])) {
      dataReset = true;
      if (linkObjectList[objHandle]) {
         if (attrHandle === ConsequenceHandle) {
            linkObjectList[objHandle].consequence = val;
         }
         if (attrHandle === LinkFlowHandle) {
            linkObjectList[objHandle].state = val;
         }
      }
   }
};

dmz.object.scalar.observe(self, ConsequenceHandle, dataUpdated);
dmz.object.scalar.observe(self, ThreatHandle, dataUpdated);
dmz.object.scalar.observe(self, ReducedVulnerabilityHandle, dataUpdated);
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
      link =
         { superLink: Super
         , sub: Sub
         , attr: AttrObj
         , handle: linkHandle
         , state: dmz.object.state(AttrObj, LinkFlowHandle)
         , consequence: dmz.object.scalar(AttrObj, ConsequenceHandle)
         };
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

graphInit = function () {
//cascadeInit = function () {
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

   GraphHasSource = false;

   GraphType.CASCADE.count = 0;
   GraphType.CASCADE.pdf = [];
   GraphType.FLOW.count = 0;
   GraphType.FLOW.pdf = [];

   for (count = 0; count <= 100; count += 1) {
      GraphType.CASCADE.pdf[count] = 0;
      GraphType.FLOW.pdf[count] = 0;
   }

   cdf = [];
   objectArray = [];
   count = 0;
   Object.keys(objectList).forEach(function (key) {
      handle = parseInt(key);
      threat = dmz.object.scalar(handle, ThreatHandle);
      vuln = dmz.object.scalar(handle, ReducedVulnerabilityHandle);
      if (!vuln) { vuln = dmz.object.scalar(handle, VulnerabilityHandle); }
      pdf[count] = threat * vuln;
      sumTV += pdf[count];
      objectArray[count] = handle;
      count += 1;
   });

   if (allowLinks) {
      Object.keys(linkObjectList).forEach(function (key) {
         threat = dmz.object.scalar(linkObjectList[key].attr, ThreatHandle);
         vuln = dmz.object.scalar(linkObjectList[key].attr, ReducedVulnerabilityHandle);
         if (!vuln) { vuln = dmz.object.scalar(handle, VulnerabilityHandle); }
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

objectFromCDF = function () {
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
   var vuln = dmz.object.scalar(objectHandle, ReducedVulnerabilityHandle) ?
              dmz.object.scalar(objectHandle, ReducedVulnerabilityHandle) :
              dmz.object.scalar(objectHandle, VulnerabilityHandle)
     , result = Math.random() <= (vuln * dmz.object.scalar(objectHandle, ThreatHandle));
   return ((result === true) || (result === false)) ? result : false;
};

GraphType.CASCADE.function = function () {
//cascadeFailureSimulation = function () {
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
      graphInit();
   }

   initFailure = objectFromCDF();
   if (initFailure && typeof(initFailure) == "object") {
      visited[initFailure.attr] = true;
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
      if (failureType.and(CascadeFailUpstreamState).bool()) {

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
   }
   else if (initFailure && objectList[initFailure]) {
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
               if (failureType.and(CascadeFailUpstreamState).bool()) {

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
      Object.keys(linkObjectList).forEach(function (key) {
         totalConsequences += dmz.object.scalar(parseInt(key), ConsequenceHandle)
      });
   }

   if (totalConsequences > 0) {
      failedConsequences = Math.round(failedConsequences / totalConsequences * 100);
   }
   else {
      failedConsequences = 0;
   }

   GraphType.CASCADE.pdf[failedConsequences] += 1;
   GraphType.CASCADE.count += 1;

   if ((GraphType.CASCADE.count % updateGraphDelay) === 0) {
      updateGraph(GraphType.CASCADE);
   }

};

graphHasBiLinks = function () {

   var link
     , state
     , result = false
     ;

   Object.keys(linkObjectList).forEach(function (key) {
      link = linkObjectList[key];
      state = dmz.object.state(link.attr, LinkFlowHandle);
      if (state) {
         if (state.and(FlowStateBoth).bool()) {
            result = true;
         }
      }
   });

   return result;
}

graphHasSink = function (matrix) {
   var result = false
     , n = Object.keys(objectList).length
     , colIter
     , rowIter
     , rowSum
     ;

   for (colIter = 0; (colIter < n) && (!result); colIter += 1) {
      rowSum = 0;
      for (rowIter = colIter + 1; (rowIter < n) && (rowSum === 0); rowIter += 1) {
         rowSum += matrix.m[colIter][rowIter];
      }
      if (rowSum === 0) { result = true; }
   }

   return result;
};

calculateCapacityMatrix = function () {
   var matrix = dmz.nmmatrix.create(Object.keys(objectList).length)
     , isSource
     , isDownHillFrom
     , keyList = Object.keys(objectList)
     , iParent
     , parentHandle
     , iChild
     , childHandle
     , link
     ;

   isSource = function (node) {
      var link
        , linkState
        , result = true
        ;

      Object.keys(linkObjectList).forEach(function (index) {
         link = linkObjectList[index];
         linkState = link.state;
         if (((link.superLink === node) && linkState.and(ReverseFlowState).bool()) ||
             ((link.sub === node) && linkState.and(ForwardFlowState).bool())) {

            result = false;
         }
      });

      if (result) {
         GraphHasSource = true;
      }

      return result;
   };

   isDownHillFrom = function (child, parent) {
      var result = null
        , link
        , linkState
        ;

//      self.log.warn ("isDownHill", linkObjectList, Object.keys(linkObjectList));
      Object.keys(linkObjectList).forEach(function (index) {
         link = linkObjectList[index];
         linkState = link.state;
         //self.log.warn (link.superLink, link.sub, parent, child);
         if (((link.superLink === parent) && (link.sub === child) &&
                  (linkState.and(ForwardFlowState).bool()))
               || ((link.superLink === child) &&
                   (link.sub === parent) &&
                   (linkState.and(ReverseFlowState).bool()))) {

            //self.log.error(link.consequence, "DOWNHILL");

            result = link;
         }
      });

      return result;
   };

   for (iParent = 0; iParent < keyList.length; iParent += 1) {
      parentHandle = parseInt(keyList[iParent]);
      if (isSource(parentHandle)) {
         matrix.setElement(
            iParent,
            iParent,
            dmz.object.scalar(parentHandle, ConsequenceHandle));
      }
      for (iChild = 0; iChild < keyList.length; iChild += 1) {
         childHandle = parseInt(keyList[iChild]);
         if (iChild != iParent) {
            link = isDownHillFrom(childHandle, parentHandle);
//            self.log.warn(link, link ? true : false);
            if (link) {
//               self.log.warn(link.consequence);
               matrix.setElement(
                  iChild,
                  iParent,
                  link.consequence);
            }
         }
      }
   }

   return matrix;
};

calculateFractionMatrix = function (capacityMatrix) {

   var n = Object.keys(objectList).length
     , matrix = dmz.nmmatrix.create(n)
     , rowSum
     , rowIter
     , colIter
     ;

   for (rowIter = 0; rowIter < n; rowIter += 1) {
      rowSum = 0;
      for (colIter = 0; colIter < n; colIter += 1) {
         if (rowIter != colIter) {
            rowSum += capacityMatrix.m[colIter][rowIter];
         }
      }
      self.log.warn (rowIter, rowSum);
      if (rowSum > 0) {
         for (colIter = 0; colIter < n; colIter += 1) {
            if (capacityMatrix.m[rowIter][rowIter] > 0) {
               matrix.setElement(rowIter, rowIter, 1);
            }
            if (rowIter != colIter) {
               matrix.setElement(
                  colIter,
                  rowIter,
                  capacityMatrix.m[colIter][rowIter] / rowSum);
               //matrix.m[colIter][rowIter] = capacityMatrix.m[colIter][rowIter] / rowSum;
            }
         }
      }
   }

   return matrix;
};

calculateNetworkFlow = function (capacityMatrix, fractionMatrix) {
   var vec = capacityMatrix.diagonal()
     , trans = fractionMatrix.transpose()
     , ccount
     , rcount
     , sum
     , n = vec.length
     , tvec = dmz.nvector.create(vec.length)
     , ix
     ;

   for (ix = 0; ix < n; ix += 1) {
      for (rcount = 0; rcount < n; rcount += 1) {
         sum = 0;
         for (ccount = 0; ccount < n; ccount += 1) {
            sum += Math.min (
               capacityMatrix.m[rcount][ccount],
               trans.m[ccount][rcount] * vec.v[ccount]);
         }
         tvec.setElement(rcount, sum);
      }
      vec = tvec.copy();
      self.log.warn (ix, vec);
   }

   return vec;

};

GraphType.FLOW.function = function () {

   var error = null
     , capacityMatrix
     , fractionMatrix
     , origFlow
     , newFlow
     ;

   self.log.error ("FLOW function");
   if (dataReset) {
      graphInit();
   }

   capacityMatrix = calculateCapacityMatrix();
//   self.log.warn ("capacity matrix:\n", capacityMatrix);

   fractionMatrix = calculateFractionMatrix(capacityMatrix);

//   self.log.warn ("\n\n" + fractionMatrix);
//   self.log.warn ("\n\n" + fractionMatrix.transpose());

   if (!graphHasSink(capacityMatrix)) {
      error = dmz.data.wrapString("Error: Graph must have a \"sink\" node.");
   }
   else if (!GraphHasSource) {
      error = dmz.data.wrapString("Error: Graph must have a \"source\" node.");
   }
   else if (graphHasBiLinks()) {
      error = dmz.data.wrapString("Error: Graph may not contain bidirectional links.");
   }
   if (error) {
      errorMessage.send(error);
   }
   else {
      origFlow = calculateNetworkFlow(capacityMatrix, fractionMatrix);
      self.log.warn ("orig flow:", origFlow);

      // Loop to test node failures
      // Loop to test link failures
      // For each tested, increment exceedence in correct location
      // Set count = total number of nodes and links

   }

   self.log.error ("END FLOW function");

};

updateGraph = function (graphType) {

   var ix
     , counter
     , ep = []
     ;

   for (counter = 0; counter <= 100; counter += 1)  { ep[counter] = 0; }

   ep[100] = graphType.pdf[100] / graphType.count;
   for (counter = 99; counter >= 1; counter -= 1) {
      self.log.warn (graphType.pdf[counter], graphType.count, ep[counter + 1]);
      ep[counter] = (graphType.pdf[counter] / graphType.count) + ep[counter + 1];
   }

   simulateIterCountMessage.send(dmz.data.wrapNumber(graphType.count));
   for (ix = 0; ix < barCount; ix += 1) {
      self.log.warn ((ix+1), (ep[ix+1]*100));
      dmz.object.counter(bars[ix], BarValueHandle, ep[ix + 1] * 100);
   }
};

simulateMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
//         dmz.time.setRepeatingTimer(self, cascadeFailureSimulation);
         if (currentType == GraphType.CASCADE) {
            dmz.time.setRepeatingTimer(self, GraphType.CASCADE.function);
         }
         else {
            dmz.time.setTimer(self, GraphType.FLOW.function);
         }

         GraphActive = true;
//         cascadeFailureSimulation();
      }
      else if (GraphActive) {
//         dmz.time.cancleTimer(self, cascadeFailureSimulation);
         dmz.time.cancleTimer(self, currentType.function);
         GraphActive = false;
         if (currentType == GraphType.CASCADE) {
            updateGraph(currentType);
         }
      }
   }
});

simulationTypeMessage.subscribe(self, function (data) {
   var state
     ;

   if (dmz.data.isTypeOf(data)) {
      self.log.warn ("type:", data.boolean("Boolean", 0), dmz.data.unwrapBoolean(data));
      if (!data.boolean("Boolean", 0)) {
         self.log.warn ("setting to FLOW");
         if (GraphActive) {
            dmz.time.cancleTimer(self, GraphType.CASCADE.function);
            dmz.time.setTimer(self, GraphType.FLOW.function);
         }
         currentType = GraphType.FLOW;
      }
      else {
         self.log.warn ("setting to CASCADE");
         if (GraphActive) {
            dmz.time.cancleTimer(self, GraphType.FLOW.function);
            dmz.time.setRepeatingTimer(self, GraphType.CASCADE.function);
         }
         currentType = GraphType.CASCADE;
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
