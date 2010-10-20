var dmz =
      { object: require("dmz/components/object")
      , defs: require("dmz/runtime/definitions")
      , time: require("dmz/runtime/time")
      }
   , FTNameHandle = dmz.defs.createNamedHandle("FT_Name")
   , FTLinkHandle = dmz.defs.createNamedHandle("FT_Link")
   , FTPathHandle = dmz.defs.createNamedHandle("FT_Node_Path_Name")
   , work
   , list = {}
   , updatePath
   , findRoot
   ;

updatePath = function (path, handle) {
   var name = dmz.object.text(handle, FTNameHandle)
     , subList
     , newPath = path
     ;

   if (name) {
      if (path == "") {
         newPath = name;
      }
      else {
         newPath += "." + name;
      }
   }
   dmz.object.text(handle, FTPathHandle, newPath);
   subList = dmz.object.subLinks(handle, FTLinkHandle);
   if (subList) {
      Object.keys(subList).forEach(function (key) {
         updatePath(newPath, subList[key]);
      });
   }
};

findRoot = function (handle) {
   var result = handle
     , superList = dmz.object.superLinks(handle, FTLinkHandle)
     ;
   if (superList) {
      result = findRoot(superList[0]);
   }
   return result;
};

work = dmz.time.setRepeatingTimer(self, function () {
   var roots
     , root
     ;
   if (list) {
      roots = {};
      Object.keys(list).forEach(function (key) {
         root = findRoot(list[key]);
         roots[root] = root;
      });
      Object.keys(roots).forEach(function (key) {
         updatePath("", roots[key]);
      });
      list = false;
   }
});

dmz.object.text.observe(self, FTNameHandle, function (handle) {
   if (!list) {
      list = {};
   }
   list[handle] = handle;
});

dmz.object.link.observe(self, FTLinkHandle, function (link, attr, Super, sub) {
   if (!list) {
      list = {};
   }
   list[Super] = Super;
   list[sub] = sub;
});

dmz.object.unlink.observe(self, FTLinkHandle, function (link, attr, Super, sub) {
   if (list) {
      if (list[Super]) {
         delete list[Super];
      }
      if (list[sub]) {
         delete list[sub];
      }
   }
});
