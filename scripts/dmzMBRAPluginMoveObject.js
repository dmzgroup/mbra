var dmz =
   { objectType: require("dmz/runtime/objectType")
   , defs: require("dmz/runtime/definitions")
   , data: require("dmz/runtime/data")
   , object: require("dmz/components/object")
   , message: require("dmz/runtime/messaging")
   , undo: require("dmz/runtime/undo")
   }
   , NodeType = dmz.objectType.lookup ("na_node")
   , selectMessage = dmz.message.create (
         self.config.string("message.select.name", "SelectMoveObjectMessage"))
   , unselectMessage = dmz.message.create (
         self.config.string("message.unselect.name","UnselectMoveObjectMessage"))
   , moveMessage = dmz.message.create (
         self.config.string ("message.move.name", "MoveSelectedObjectMessage"))
   , MoveNode = dmz.defs.createNamedHandle ("Move Node")
   , firstMove = false
   , Handle = null
   , Offset = null
   ;

selectMessage.subscribe (self, function (data, type) {
   firstMove = true;
   if (dmz.data.isTypeOf (data)) {
      Handle = data.handle ("object", 0);
      var pos = data.vector ("position", 0);
      if (Handle && pos) {
         var ObjType = dmz.object.type (Handle);
         if (ObjType && ObjType.isOfType (NodeType)) {
            var cpos = dmz.object.position (Handle);
            if (cpos) { Offset = cpos.subtract(pos); }
            else { Offset = dmz.vector.create (); }
         } else { Handle = null; }
      }
   }
});

unselectMessage.subscribe (self, function () { Handle = null; Offset = null;});

moveMessage.subscribe (self, function (data, type) {
   if (Handle && dmz.data.isTypeOf (data)) {
      var undoHandle = null;
      if (firstMove) {
         undoHandle = dmz.undo.startRecord (MoveNode);
         firstMove = false;
      }
      var pos = data.vector ("position", 0);
      if (Handle && pos) {
         dmz.object.position (Handle, null, pos.add(Offset));
      }
      if (undoHandle) { dmz.undo.stopRecord (undoHandle); }
   }
});
