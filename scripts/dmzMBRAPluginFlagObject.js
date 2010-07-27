var dmz =
   { objectType: require("dmz/runtime/objectType")
   , defs: require("dmz/runtime/definitions")
   , data: require("dmz/runtime/data")
   , object: require("dmz/components/object")
   , message: require("dmz/runtime/messaging")
   , undo: require("dmz/runtime/undo")
   }
   , NodeType = dmz.objectType.lookup ("na_node")
   , FlagState = dmz.defs.lookupState ("NA_Node_Flagged")
   , message = dmz.message.create(self.config.string("edit.name","FlagObjectMessage"))
   ;


message.subscribe (self, function (data, type) {

   if (dmz.data.isTypeOf (data)) {

      var sendMessage = dmz.defs.createNamedHandle("Flag Node");
      var handle = data.handle ("object", 0);

      if (handle && dmz.object.isLink (handle)) {
         sendMessage = dmz.defs.createNamedHandle("Flag Link");
         handle = dmz.object.linkAttributeObject (handle);
      }

      if (handle && dmz.object.isObject (handle)) {
         var UndoHandle = dmz.undo.startRecord (sendMessage);
         var state = dmz.object.state (handle);
         if (!state) { state = dmz.mask.create (); }
         if (state.contains (FlagState)) { state = state.unset (FlagState); }
         else { state = state.or(FlagState); }
         dmz.object.state (handle, null, state);
         dmz.undo.stopRecord (UndoHandle);
      }

   }

});

