var dmz =
      { object: require("dmz/components/object")
      , objectType: require("dmz/runtime/objectType")
      , defs: require("dmz/runtime/definitions")
      }
   , NodeType = dmz.objectType.lookup("na_node")
   , NodeLinkType = dmz.objectType.lookup("na_link_attribute")
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")
   , DegreeHandle = dmz.defs.createNamedHandle("NA_Node_Degrees")
   , list = []
   ;

var update_degrees = function (handle) {
   var count = 0
     , sub = dmz.object.subLinks(handle, LinkHandle)
     , superLinks
     ;
   if (sub) {
      count += sub.length;
   }
   superLinks = dmz.object.superLinks(handle, LinkHandle);
   if (superLinks) {
      count += superLinks.length;
   }
   dmz.object.scalar(handle, DegreeHandle, count);
};

dmz.object.create.observe(self, function (handle, objType, varity) {
   if (objType) {
      if (objType.isOfType(NodeType)) {
         list[handle] = true;
      }
      else if (objType.isOfType(NodeLinkType)) {
         dmz.object.scalar(handle, DegreeHandle, 1);
      }
   }
});

dmz.object.destroy.observe(self, function (handle) {
   delete list[handle];
});

dmz.object.link.observe(self, LinkHandle, function (link, attr, superLink, sub) {
   if (list[superLink]) {
      update_degrees(superLink);
   }
   if (list[sub]) {
      update_degrees(sub);
   }
});

dmz.object.unlink.observe(self, LinkHandle, function (link, attr, superLink, sub) {
   if (list[superLink]) {
      update_degrees(superLink);
   }
   if (list[sub]) {
      update_degrees(sub);
   }
});
