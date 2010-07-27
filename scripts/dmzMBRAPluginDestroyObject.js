var dmz =
   { object: require("dmz/components/object")
   , objectType: require("dmz/runtime/objectType")
   , data: require("dmz/runtime/data")
   , defs: require("dmz/runtime/definitions")
   , message: require("dmz/runtime/messaging")
   , undo: require("dmz/runtime/undo")
   }
   , NodeType = dmz.objectType.lookup ("na_node")
   , NodeLinkType = dmz.defs.createNamedHandle("Node_Link")
   , message = dmz.message.create (
         self.config.string("message.name", "DestroyObjectMessage"))
   , objects = []
   ;

message.subscribe (self, function (data) {
   if (dmz.data.isTypeOf (data)) {
      var handle = data.handle ("object", 0);
      if (handle) {
         var link = objects[handle];
         if (link) { handle = link; }
         var objType = dmz.object.type (handle);
         if (objType && objType.isOfType (NodeType) && dmz.object.isObject (handle)) {
            var undoHandle = dmz.undo.startRecord ("Delete Node");
            dmz.object.destroy (handle);
            dmz.undo.stopRecord (undoHandle);
         } else if (dmz.object.isLink (handle)) {
            var undoHandle = dmz.undo.startRecord ("Unlink Nodes");
            dmz.object.unlink (handle);
            dmz.undo.stopRecord (undoHandle);
         }
      } else { self.log.error ("Got null handle!"); }
   }
});

dmz.object.destroy.observe (self, function (object) { delete objects[object]; });

dmz.object.linkAttributeObject.observe (self, NodeLinkType,
function (link, attr, Super, sub, object, prev) {
   if (object && dmz.object.type (object).isOfType (NodeLinkType)) {
      objects[object] = link;
   }
});
