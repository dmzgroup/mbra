var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , defs: require("dmz/runtime/definitions")
      , data: require("dmz/runtime/data")
      , undo: require("dmz/runtime/undo")
      , message: require("dmz/runtime/messaging")
      }
   , firstHandle = dmz.object.create("na_tool_link_node")
   , secondHandle = dmz.object.create("na_tool_link_node")
   , startMessage = dmz.message.create(
      self.config.string("message.first.name", "FirstLinkObjectMessage"))
   , updateMessage = dmz.message.create(
      self.config.string("message.update.name", "UpdateLinkPositionMessage"))
   , endMessage = dmz.message.create(
      self.config.string("message.second.name", "SecondLinkObjectMessage"))
   , failedMessage = dmz.message.create(
      self.config.string("message.failed.name", "FailedLinkObjectsMessage"))
   , editMessage = dmz.message.create(
      self.config.string("message.edit.name", "EditObjectAttributesMessage"))
   , NodeType = dmz.objectType.lookup("na_node")
   , LinkHandle = null
   , Handle = null
   , NodeLink = dmz.defs.createNamedHandle("Node_Link")
   , undoLinkRecord = "Link Nodes"
   ;

(function () {
   if (firstHandle) {
      dmz.object.activate(firstHandle);
   }
   if (secondHandle) {
      dmz.object.activate(secondHandle);
   }
}());

startMessage.subscribe(self, function (data) {
   var handle
     , pos
     ;
   if (LinkHandle) {
      dmz.object.unlink(LinkHandle);
      LinkHandle = null;
   }
   Handle = null;
   if (dmz.data.isTypeOf(data)) {
      handle = data.handle("object", 0);
      if (handle && dmz.object.isObject(handle) &&
           dmz.object.type(handle).isOfType(NodeType)) {

         Handle = handle;
      }
      if (Handle && firstHandle && secondHandle) {
         pos = dmz.object.position(Handle);
         if (pos) {
            dmz.object.position(firstHandle, null, pos);
            dmz.object.position(secondHandle, null, pos);
            LinkHandle = dmz.object.link(NodeLink, firstHandle, secondHandle);
         }
      }
   }
});

updateMessage.subscribe(self, function (data) {
   var pos
     ;
   if (dmz.data.isTypeOf(data)) {
      pos = data.vector("position", 0);
      if (pos && secondHandle) {
         dmz.object.position(secondHandle, null, pos);
      }
   }
});

endMessage.subscribe(self, function (data) {
   var handle
     , undoHandle
     , linkHandle
     , attrObj
     , outData
     , nalpHandle
     ;
   if (LinkHandle) {
      dmz.object.unlink(LinkHandle);
      LinkHandle = null;
   }
   if (dmz.data.isTypeOf(data)) {
      handle = data.handle("object", 0);
      if (handle) {
         if (handle == Handle) {
            Handle = null;
            handle = null;
         }
         else if (dmz.object.isObject(handle) &&
               dmz.object.type(handle).isOfType(NodeType) && Handle) {
            undoHandle = dmz.undo.startRecord(undoLinkRecord);
            linkHandle = dmz.object.link(NodeLink, Handle, handle);
            attrObj = null;
            if (linkHandle) {
               attrObj = dmz.object.create("na_link_attribute");
               dmz.object.linkAttributeObject(linkHandle, attrObj);
               outData = dmz.data.create();
               outData.handle("object", 0, linkHandle);
               outData.handle("created", 0, linkHandle);
               nalpHandle = dmz.defs.createNamedHandle("NetworkAnalysisLinkProperties");
               editMessage.send(nalpHandle, outData);
            }
            if (dmz.object.isLink(linkHandle)) {
               if (attrObj && dmz.object.isObject(attrObj)) {
                  dmz.object.activate(attrObj);
               }
               dmz.undo.stopRecord(undoHandle);
            }
            else {
               if (attrObj && dmz.object.isObject(attrObj)) {
                  dmz.object.destroy(attrObj);
               }
               dmz.undo.abortRecord(undoHandle);
            }
         }
      }
   }
   Handle = null;
});

failedMessage.subscribe(self, function () {
   if (LinkHandle) {
      dmz.object.unlink(LinkHandle);
      LinkHandle = null;
   }
   Handle = null;
});
