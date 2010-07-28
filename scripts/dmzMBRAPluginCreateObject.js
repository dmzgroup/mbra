var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , data: require("dmz/runtime/data")
      , defs: require("dmz/runtime/definitions")
      , message: require("dmz/runtime/messaging")
      , undo: require("dmz/runtime/undo")
      }
   , message = dmz.message.create(
      self.config.string("message.name","CreateObjectMessage"))
   , editMessage = dmz.message.create(
      self.config.string("edit.name","EditObjectAttributesMessage"))
   ;

// Create a new node, place it, then send a message to open the Edit NA node window
// Once the window has been closed successfully, activate the node. If the user hit
// cancel, the node will not be activated.

// NOTE:
// Necessary to convert strings into new named handles before sending messages

message.subscribe(self, function receive (data) {

   var pos
     , undoHandle
     , handle
     , outData
     , nanpHandle
     ;
   if (dmz.data.isTypeOf (data)) {

      pos = data.vector ("position", 0);
      undoHandle = dmz.undo.startRecord ("Create Node");
      handle = dmz.object.create("na_node");

      if (!handle) { self.log.error ("Object Not Created!"); }
      else {
         dmz.object.position (handle, null, pos);
         outData = dmz.data.create ();
         outData.handle ("object", 0, handle);
         outData.handle ("created", 0, handle);
         nanpHandle = dmz.defs.createNamedHandle("NetworkAnalysisNodeProperties");
         editMessage.send (nanpHandle, outData);
      }

      if (dmz.object.isObject (handle)) {
         dmz.object.activate (handle);
         dmz.undo.stopRecord (undoHandle);
      } else { dmz.undo.abortRecord (undoHandle); }
   }
});
