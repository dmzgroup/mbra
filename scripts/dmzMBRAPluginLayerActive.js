var dmz =
   { defs: require("dmz/runtime/definitions")
   , object: require("dmz/components/object")
   }
   , ActiveHandle = dmz.defs.createNamedHandle ("Layer_Active")
   , active = null
   ;


dmz.object.flag.observe (self, ActiveHandle, function (handle, attr, value) {
   if (value) {
      if (active && active != handle) {
         dmz.object.flag (active, ActiveHandle, false);
      }
      active = handle;
   } else if (active == handle) { active = null; }
});
