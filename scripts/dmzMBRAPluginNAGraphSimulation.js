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
   , ConsequenceReducedHandle = dmz.defs.createNamedHandle(
        "NA_Node_Consequence_Reduced")
   , currentConsequenceHandle = ConsequenceHandle
   , FlowConsequenceHandle = dmz.defs.createNamedHandle("NA_Node_Flow_Consequence")
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
   , LabelHandle = dmz.defs.createNamedHandle("NA_Node_Name")

   , GraphType =
        { CASCADE:
             { count: 0
             , pdf: []
             , function: function () { self.log.warn ("CASCADE GRAPH FUNCTION"); }
             , dataReset: true
             }
        , FLOW:
             { count: 0
             , pdf: []
             , delay: 500
             , function: function () { self.log.warn ("FLOW GRAPH FUNCTION"); }
             , origFlow: 0
             , dataReset: true
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
   , calculationTypeMessage = dmz.message.create(
         self.config.string("simulator-message.name", "CalcSimulationType"))
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
           "simulate-allow-links-message.name",
           "NASimulateLinksMessage"))
   , cleanupMessage = dmz.message.create("CleanupObjectsMessage")

   , objectList = {}
   , linkObjectList = {}
   , sinkList = []

   , firstRun = true
   , bars = []
   , barCount = 100
   , failureType = CascadeFailBothState
   , updateGraphDelay = 500
   , allowLinks = true
   , GraphHasSource = false
   , flowError

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
   , isSource
   , isSink
   , allLinksFlow
   , calculateNetworkFlow
   , calculateCapacityMatrix
   , calculateFractionMatrix
   , removeNodeFromCapacityMatrix
   , removeLinkFromCapacityMatrix
   , graphHasSource

   ;


(function () {
   var ix;
   errorMessage.send(dmz.data.wrapString(""));
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
         GraphType.FLOW.dataReset = true;
         GraphType.CASCADE.dataReset = true;
      }
   }
});


dataUpdated = function (objHandle, attrHandle, val) {
   if (objHandle && (objectList[objHandle] || linkObjectList[objHandle])) {
      dataReset = true;
      if ((attrHandle !== ThreatHandle) && (attrHandle !== VulnerabilityHandle)) {
         GraphType.FLOW.dataReset = true;
      }
      GraphType.CASCADE.dataReset = true;

      if (attrHandle === ConsequenceReducedHandle) {
         currentConsequenceHandle = ConsequenceReducedHandle;
      }

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
dmz.object.scalar.observe(self, ConsequenceReducedHandle, dataUpdated);
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
      GraphType.FLOW.dataReset = true;
      GraphType.CASCADE.dataReset = true;
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
         , consequence: dmz.object.scalar(AttrObj, currentConsequenceHandle)
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
   GraphType.FLOW.dataReset = true;
   GraphType.CASCADE.dataReset = true;
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

   if (allowLinks || (currentType === GraphType.FLOW)) {
      Object.keys(linkObjectList).forEach(function (key) {
         linkObjectList[key].fromIndex = -1;
         linkObjectList[key].toIndex = -1;
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

   if (GraphType.CASCADE.dataReset) {
      if (dataReset) { graphInit(); }
      GraphType.CASCADE.dataReset = false;
   }

   initFailure = objectFromCDF();
   if (initFailure && typeof(initFailure) == "object") {
      visited[initFailure.attr] = true;
      failedConsequences += dmz.object.scalar(initFailure.attr, currentConsequenceHandle);
      linkState = dmz.object.state(initFailure.attr, LinkFlowHandle);
      if (!linkState) { linkState = FlowStateBoth; }
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
                  failedConsequences += link.consequence;
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

                     if (checkObjectCascadeFail(link.sub)) { list.push(link.sub); }
                     visited[link.sub] = true;
                  }
               }
            }
         }
      });

      failedConsequences += dmz.object.scalar(current, currentConsequenceHandle);
   }

   Object.keys(objectList).forEach(function (key) {
      totalConsequences += dmz.object.scalar(parseInt(key), currentConsequenceHandle);
   });

   if (allowLinks) {
      Object.keys(linkObjectList).forEach(function (key) {
         totalConsequences += dmz.object.scalar(parseInt(key), currentConsequenceHandle);
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
         if (state.and(FlowStateBoth).equal(FlowStateBoth)) {
            result = true;
         }
      }
   });

   return result;
}

graphHasSink = function (matrix, hasFailedObject) {
   var n = Object.keys(objectList).length
     , colIter
     , rowIter
     , rowSum
     , counter = 0
     ;

   if (!hasFailedObject) { sinkList = []; }

   for (rowIter = 0; rowIter < n; rowIter += 1) {
      rowSum = 0;
      for (colIter = 0; (colIter < n) && (rowSum === 0); colIter += 1) {
         if (colIter != rowIter) { rowSum += matrix.m[colIter][rowIter]; }
      }
      if (rowSum === 0) {
         counter += 1;
         if (!hasFailedObject) { sinkList.push(rowIter); }
      }
   }
   return counter > 0;
};

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

   if (result) { GraphHasSource = true; }
   return result;
};

calculateCapacityMatrix = function () {
   var matrix = dmz.nmmatrix.create(Object.keys(objectList).length)
     , isDownHillFrom
     , keyList = Object.keys(objectList)
     , iParent
     , parentHandle
     , iChild
     , childHandle
     , link
     ;

   isDownHillFrom = function (child, parent) {
      var result = null
        , link
        , linkState
        ;

      Object.keys(linkObjectList).forEach(function (index) {
         link = linkObjectList[index];
         linkState = link.state;
         if (((link.superLink === parent) && (link.sub === child) &&
                  (linkState.and(ForwardFlowState).bool()))
               || ((link.superLink === child) &&
                   (link.sub === parent) &&
                   (linkState.and(ReverseFlowState).bool()))) {

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
            if (link) {
               link.fromIndex = iParent;
               link.toIndex = iChild;
               matrix.setElement(iChild, iParent, link.consequence);
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
         if (rowIter != colIter) { rowSum += capacityMatrix.m[colIter][rowIter]; }
      }
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
            }
         }
      }
   }

   return matrix;
};

graphHasSource = function (capacityMatrix) {
   var vec = capacityMatrix.diagonal()
     , counter = 0
     , result = false
     ;

   for (counter = 0; (counter < vec.length) && !result; counter += 1) {
      if (vec.v[counter] > 0) { result = true; }
   }

   GraphHasSource = result;
   return result;
}

calculateNetworkFlow = function (capacityMatrix, currFail) {
   var vec = capacityMatrix.diagonal()
     , fractionMatrix = calculateFractionMatrix(capacityMatrix)
     , trans = fractionMatrix.transpose()
     , ccount
     , rcount
     , sum
     , n = vec.length
     , tvec = dmz.nvector.create(vec.length)
     , ix
     , result = 0
     ;

//   self.log.warn ("currFail:", dmz.object.text(parseInt(currFail), LabelHandle), currFail);
//   self.log.warn ("capacityMatrix:", capacityMatrix);
//   self.log.warn ("fractionMatrix:", fractionMatrix);
//   self.log.warn ("transposeMatrix:", trans);

   if (!graphHasSink(capacityMatrix, currFail)) {
      if (!currFail) {
         errorMessage.send(
            dmz.data.wrapString("Error: Graph must have a \"sink\" node."));
         self.log.error("Error: Graph must have a \"sink\" node.");
         result = -1;
      }
   }
   else if (!graphHasSource(capacityMatrix)) {
      if (!currFail) {
         errorMessage.send(
            dmz.data.wrapString("Error: Graph must have a \"source\" node."));
         self.log.error("Error: Graph must have a \"source\" node.");
         result = -1;
      }
   }
   else {
      for (ix = 0; ix < n; ix += 1) {
//         self.log.warn ("vec["+ix+"]:", vec);
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
      }

//      self.log.warn ("final vec:", vec);
      sum = 0;
      sinkList.forEach(function (sinkIndex) {
         if (sinkIndex != currFail) { sum += vec.v[sinkIndex]; }
      });
      result = sum;
   }

   return result;
};

removeNodeFromCapacityMatrix = function (capacityMatrix, handle) {

   var counter
     , keyList = Object.keys(objectList)
     ;

   if (isSource(handle)) {
      for (counter = 0; counter < keyList.length; counter += 1) {
         if (keyList[counter] == handle) {
            capacityMatrix.setElement(counter, counter, 0);
         }
      }
   }

   Object.keys(objectList[handle]).forEach(function (index) {
      removeLinkFromCapacityMatrix(capacityMatrix, objectList[handle][index]);
   });


};

removeLinkFromCapacityMatrix = function (capacityMatrix, link) {
   var keyList = Object.keys(objectList)
     ;

   if ((link.fromIndex !== -1) && (link.toIndex !== -1)) {
      capacityMatrix.setElement(link.toIndex, link.fromIndex, 0);
   }
   else {
      self.log.error(
         "Trying to remove link with unset from / to:",
         link.fromIndex,
         link.toIndex);
   }
};

GraphType.FLOW.calculate = function () {

   var capacityMatrix
     , tempMatrix
     , origFlow
     , newFlow
     ;

     errorMessage.send(dmz.data.wrapString(""));
     flowError = null;
   if (GraphType.FLOW.dataReset) {
      if (dataReset) { graphInit(); }

      if (graphHasBiLinks()) {
         errorMessage.send(
            dmz.data.wrapString(
               "Error: Graph may not contain bidirectional links."));
         self.log.error("Error: Graph may not contain bidirectional links.");
      }
      else {
         capacityMatrix = calculateCapacityMatrix();
         GraphType.FLOW.origFlow = calculateNetworkFlow(capacityMatrix);
         if (GraphType.FLOW.origFlow > -1) {

//            self.log.warn("origFlow:", GraphType.FLOW.origFlow);
            // Loop to test node failures
            Object.keys(objectList).forEach(function (key) {
               var tempMatrix = capacityMatrix.copy()
                 , newFlow = 0
                 ;

               removeNodeFromCapacityMatrix(tempMatrix, parseInt(key));
               newFlow = calculateNetworkFlow(tempMatrix, parseInt(key));
//               self.log.warn(dmz.object.text(parseInt(key), LabelHandle), "New:", newFlow, "Delta:", GraphType.FLOW.origFlow - newFlow)

               if (newFlow > GraphType.FLOW.origFlow) {
                  newFlow = GraphType.FLOW.origFlow;
                  errorMessage.send(
                     dmz.data.wrapString(
                        "Warning: Graph contains link(s) or node(s) with negative flow"
                        + " consequence."));
               }
               dmz.object.scalar(
                  parseInt(key),
                  FlowConsequenceHandle,
                  (GraphType.FLOW.origFlow - newFlow));

            });

            // Loop to test link failures
            Object.keys(linkObjectList).forEach(function (key) {
               var tempMatrix = capacityMatrix.copy()
                 , newFlow = 0
                 ;

               removeLinkFromCapacityMatrix(tempMatrix, linkObjectList[key]);
               newFlow = calculateNetworkFlow(tempMatrix, linkObjectList[key]);
//               self.log.warn(dmz.object.text(linkObjectList[key].attr, LabelHandle), "New:", newFlow, "Delta:", GraphType.FLOW.origFlow - newFlow)
               if (newFlow > GraphType.FLOW.origFlow) {
                  newFlow = GraphType.FLOW.origFlow;
                  errorMessage.send(
                     dmz.data.wrapString(
                        "Warning: Graph contains link(s) or node(s) with negative flow"
                        + " consequence."));
               }
               dmz.object.scalar(
                  linkObjectList[key].attr,
                  FlowConsequenceHandle,
                  (GraphType.FLOW.origFlow - newFlow));
            });
         }
      }
      GraphType.FLOW.dataReset = false;
   }
};

GraphType.FLOW.function = function () {
   var failure
     , consequence = 0
     ;

   if (GraphType.FLOW.dataReset) { GraphType.FLOW.calculate(); }

   failure = objectFromCDF();
   if (failure.attr) {
      consequence = dmz.object.scalar(parseInt(failure.attr), FlowConsequenceHandle);
   }
   else { consequence = dmz.object.scalar(parseInt(failure), FlowConsequenceHandle); }

   if (GraphType.FLOW.origFlow > 0) {
      consequence = Math.round(consequence / GraphType.FLOW.origFlow * 100);
   }
   else { consequence = 0; }

   GraphType.FLOW.pdf[consequence] += 1;
   GraphType.FLOW.count += 1;
   if ((GraphType.FLOW.count % updateGraphDelay) === 0) { updateGraph(GraphType.FLOW); }
};

updateGraph = function (graphType) {

   var ix
     , counter
     , ep = []
     ;

   for (counter = 0; counter <= 100; counter += 1)  { ep[counter] = 0; }

   ep[100] = graphType.pdf[100] / graphType.count;
   for (counter = 99; counter >= 1; counter -= 1) {
//      self.log.warn (graphType.pdf[counter], graphType.count, ep[counter + 1]);
      ep[counter] = (graphType.pdf[counter] / graphType.count) + ep[counter + 1];
   }

   simulateIterCountMessage.send(dmz.data.wrapNumber(graphType.count));
   for (ix = 0; ix < barCount; ix += 1) {
      dmz.object.counter(bars[ix], BarValueHandle, ep[ix + 1] * 100);
   }
};

simulateMessage.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      if (data.boolean("Boolean", 0)) {
         dmz.time.setRepeatingTimer(self, currentType.function);
         GraphActive = true;
      }
      else if (GraphActive) {
         dmz.time.cancelTimer(self, currentType.function);
         GraphActive = false;
         updateGraph(currentType);
      }
   }
});

simulationTypeMessage.subscribe(self, function (data) {

   if (dmz.data.isTypeOf(data)) {
      if (!data.boolean("Boolean", 0)) {
         if (GraphActive) {
            dmz.time.cancelTimer(self, GraphType.CASCADE.function);
            dmz.time.setRepeatingTimer(self, GraphType.FLOW.function);
         }

         currentType = GraphType.FLOW;
         if (!allowLinks) {
            dataReset = true;
            GraphType.FLOW.dataReset = true;
            GraphType.CASCADE.dataReset = true;
         }
      }
      else {
         if (GraphActive) {
            dmz.time.cancelTimer(self, GraphType.FLOW.function);
            dmz.time.setRepeatingTimer(self, GraphType.CASCADE.function);
         }
         currentType = GraphType.CASCADE;
         if (!allowLinks) {
            dataReset = true;
            GraphType.FLOW.dataReset = true;
            GraphType.CASCADE.dataReset = true;
         }
      }
   }
});

calculationTypeMessage.subscribe(self, function (data) {

   if (dmz.data.isTypeOf(data)) {

      if (!data.boolean("Boolean", 0)) {

         if (GraphType.FLOW.dataReset) { GraphType.FLOW.calculate(); }
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
      GraphType.FLOW.dataReset = true;
      GraphType.CASCADE.dataReset = true;
   }
});

simulateDirectionMessage.subscribe(self, function (data) {
   var stateString;
   if (dmz.data.isTypeOf(data)) {
      stateString = data.string("String", 0);
      if (stateString === "Upstream") { failureType = CascadeFailUpstreamState; }
      else if (stateString === "Downstream") {
         failureType = CascadeFailDownstreamState;
      }
      else { failureType = CascadeFailBothState; }
      dataReset = true;
      GraphType.FLOW.dataReset = true;
      GraphType.CASCADE.dataReset = true;
   }
});
