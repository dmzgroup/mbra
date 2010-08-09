var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , defs: require("dmz/runtime/definitions")
      , data: require("dmz/runtime/data")
      , message: require("dmz/runtime/messaging")
      }
   , NodeType = dmz.objectType.lookup("na_node")
   , nanpHandle = dmz.defs.createNamedHandle("NetworkAnalysisNodeProperties")
   , nalpHandle = dmz.defs.createNamedHandle("NetworkAnalysisLinkProperties")
   //, NodeType = dmz.objectType.lookup("na_node")
   , MsgType = dmz.message.create(
      self.config.string("edit.name","EditObjectAttributesMessage"))
   ;

// Open Edit NA node window

// NOTE:
// Necessary to convert strings into new named handles before sending messages

MsgType.subscribe (self,function (data) {
   var handle
     , nanpHandle
     , nalpHandle
     ;
   if (dmz.data.isTypeOf (data)) {
      var handle = data.handle("object", 0);
      if (dmz.object.isObject (handle)) {
         if (dmz.object.type(handle).isOfType(NodeType)) {
            MsgType.send(nanpHandle, data);
         }
         else {
            MsgType.send(nalpHandle, data);
         }
      }
      else if (dmz.object.isLink (handle)) {
         MsgType.send(nalpHandle, data);
      }
   }
});

