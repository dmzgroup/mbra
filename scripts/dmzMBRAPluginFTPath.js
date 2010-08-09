var dmz =
      { object: require("dmz/components/object")
      , defs: require("dmz/runtime/definitions")
      , time: require("dmz/runtime/time")
      }
   , FTNameHandle = dmz.defs.createNamedHandle("FT_Name")
   , FTLinkHandle = dmz.defs.createNamedHandle("FT_Link")
   , FTPathHandle = dmz.defs.createNamedHandle("FT_Node_Path_Name")
   , iparis = iparis
   , pairs = pairs
   , work
   , list = {}
   ;

var update_path = function (path, handle) {
   var name = dmz.object.text(handle, FTNameHandle)
     , subList
     ;
   if (name) {
      if (path == "") {
         path = name;
      }
      else {
         path += "." + name;
      }
   }
   dmz.object.text(handle, FTPathHandle, path);
   subList = dmz.object.subLinks(handle, FTLinkHandle);
   if (subList) {
      Object.keys(list).forEach(function (key) {
         update_path(path, subList[key]);
      });
   }
};

var find_root = function (handle) {
   var result = handle
     , superList = dmz.object.superLinks(handle, FTLinkHandle);
   if (superList) {
      result = find_root(superList[0]);
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
         root = find_root(list[key]);
         roots[root] = root;
      });
      Object.keys(roots).forEach(function (key) {
         update_path("", roots[key]);
      });
      list = null;
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
