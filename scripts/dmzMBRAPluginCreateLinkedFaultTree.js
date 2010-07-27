var dmz =
   { defs: require("dmz/runtime/definitions")
   , data: require("dmz/runtime/data")
   , object: require("dmz/components/object")
   , input: require("dmz/components/input")
   , message: require("dmz/runtime/messaging")
   }
   , message = dmz.message.create (
         self.config.string ("edit.name", "CreateLinkedFaultTreeMessage"))
   , LinkHandle = dmz.defs.createNamedHandle ("NA_Fault_Tree_Link")
   , NodeName = dmz.defs.createNamedHandle ("NA_Node_Name")
   , ActiveFTHandle = dmz.defs.createNamedHandle ("FT_Active_Fault_Tree")
   , FTNameHandle = dmz.defs.createNamedHandle ("FT_Name")
   ;

message.subscribe (self, function (data, type) {
   if (dmz.data.isTypeOf(data)) {
      var handle = data.handle ("object", 0);
      if (dmz.object.isObject (handle)) {
         var ft = null;
         var links = dmz.object.subLinks (handle, LinkHandle);
         if (links) { ft = links[0]; }
         else {
            var name = dmz.object.text (handle, NodeName);
            if (!name) { name = "Root"; }
            ft = dmz.object.create ("ft_component_root");
            dmz.object.text (ft, FTNameHandle, name);
            dmz.object.activate (ft);
            dmz.object.link (LinkHandle, handle, ft);
         }
         dmz.object.flag (ft, ActiveFTHandle, true);
         dmz.input.channel ("NetworkAnalysisChannel", false);
         dmz.input.channel ("FaultTreeChannel", true);
      }
   }
});
