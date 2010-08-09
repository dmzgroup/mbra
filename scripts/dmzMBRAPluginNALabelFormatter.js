var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , data: require("dmz/runtime/data")
      , defs: require("dmz/runtime/definitions")
      , message: require("dmz/runtime/messaging")
      }
   , NodeType = dmz.objectType.lookup("na_node")
   , message = dmz.message.create(
                  self.config.string("toggle-message.name", "ToggleNodeLabelMessage"))
   , LabelHandle = dmz.defs.createNamedHandle("NA_Node_Objective_Label")
   , NodeName = dmz.defs.createNamedHandle("NA_Node_Name")
   , NodeLabel = dmz.defs.createNamedHandle("NA_Node_Label")
   , objects = {}
   , toggle = true
   ;

// Update text fields for nodes and links

var updateObject = function (obj) {
   var value = "";
   if (toggle) {
      value = obj.name;
      if (obj.objective && obj.objective !== "") {
         value = value + "\n" + obj.objective;
      }
   }
   dmz.object.text(obj.handle, NodeLabel, value);
};

dmz.object.create.observe(self, function (handle, objType) {
   if (objType.isOfType(NodeType)) {
      objects[handle] = { handle: handle, name: ""};
   }
});
   
dmz.object.destroy.observe(self, function (handle) {
   delete objects[handle];
});

dmz.object.text.observe(self, NodeName, function (handle, attr, value) {
   var obj = objects[handle];
   if (obj) {
      obj.name = value;
      updateObject(obj);
   }
});

dmz.object.text.observe(self, LabelHandle, function (handle, attr, value) {
   var obj = objects[handle];
   if (obj) {
      obj.objective = value;
      updateObject(obj);
   }
});

message.subscribe(self, function (data) {
   if (dmz.data.isTypeOf(data)) {
      toggle = data.boolean("toggle", 0);
      Object.keys(objects).forEach(function (key) {
         updateObject(objects[key]);
      });
   }
});
