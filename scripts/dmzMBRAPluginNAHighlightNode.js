var dmz =
      { defs: require("dmz/runtime/definitions")
      , data: require("dmz/runtime/data")
      , object: require("dmz/components/object")
      , message: require("dmz/runtime/messaging")
      , mask: require("dmz/types/mask")
      }
   , HighlightState = dmz.defs.lookupState("NA_Node_Highlight")
   , Message = dmz.message.create(self.config.string("message.name", "MouseMoveEvent"))
   , Handle = 0
   ;

// Highlight nodes as the mouse goes over them.

// NOTE:
// State.set() and state.unset() do not perform operations on the current object!
// They return new objects with the modified values.
// To evaluate a mask, use the state.bool() method.
// state.set () replaces all other states.

Message.subscribe(self, function (data) {
   var handle
     , state
     , prev
     ;
   if (dmz.data.isTypeOf(data)) {
      handle = data.handle("object", 0);

      if (handle && dmz.object.isLink(handle)) {
         handle = dmz.object.linkAttributeObject(handle);
      }

      if (handle && dmz.object.isObject(handle)) {
         state = dmz.object.state(handle);
         if (!state) {
            state = dmz.mask.create();
         }
         state = state.or(HighlightState);
         dmz.object.state(handle, null, state);
      }

      if (handle !== Handle) {
         if (Handle) {
            prev = Handle;
            Handle = handle;
            if (dmz.object.isObject(prev)) {
               state = dmz.object.state(prev);
               if (state.bool()) {
                  state = state.unset(HighlightState);
                  dmz.object.state(prev, null, state);
               }
            }
         } else {
            Handle = handle;
         }
      }
   }
});

dmz.object.state.observe(self, function (object, attribute, value) {
   var state
     ;
   if (value.contains(HighlightState) && (Handle !== object)) {
      state = dmz.object.state(object);
      if (state.bool()) {
         state = state.unset(HighlightState);
         dmz.object.state(object, null, state);
      }
   }
});
