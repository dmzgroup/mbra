var dmz =
      { object: require("dmz/components/object")
      , defs: require("dmz/runtime/definitions")
      }
   , LinkHandle = dmz.defs.createNamedHandle("Node_Link")
   , objectList = []
   ;

// Keep track of links, link labels, and the objects that are represented by link labels
// Whenever a node is moved, adjust all of the tags attached to its links to maintain a
// correct position
// Whenever a node is destroyed, remove all of the extraneous links, link objects, and
// link tags

// NOTE:
// Remember, JS does not allow operator overloading. Vectors cannot be added, subtracted,
// multiplied, etc, with mathematical operators -- these operations MUST be done via
// vector class methods, or else JS will try to convert the vectors to strings and
// perform the string-type versions of those operations, generally resulting in a NaN.
// The C++ bindings won't throw an error if a bad string is used.

var update_object_position = function (objectHandle) {
   var list = objectList[objectHandle]
     , attrPos
     , superPos
     , subPos
     , link
     ;
   if (list) {
      Object.keys(list).forEach(function (index) {
         link = list[index];
         superPos = dmz.object.position(link.superLink);
         subPos = dmz.object.position(link.sub);
         if (superPos && subPos) {
            attrPos = superPos.add((subPos.subtract(superPos)).multiply(0.5));
            dmz.object.position(link.attr, null, attrPos);
         }
      });
   }
};

dmz.object.position.observe(self, update_object_position);

dmz.object.destroy.observe(self, function (handle) {
   var list = objectList[handle]
     , link
     ;
   if (list) {
      Object.keys(list).forEach(function (index) {
         link = list[index];
         if (objectList[link.superLink]) {
            delete objectList[link.superLink][link.linkHandle];
         }
         if (objectList[link.sub]) {
            delete objectList[link.sub][link.linkHandle];
         }
      });
      delete objectList[handle];
   }
});

dmz.object.linkAttributeObject.observe(self, LinkHandle,
function (linkHandle, AttrHandle, Super, Sub, AttrObj, PrevObj) {
   var link
     ;
   if (AttrObj) {
      link = { superLink: Super, sub: Sub, attr: AttrObj, linkHandle: linkHandle };
      if (!objectList[Super]) {
         objectList[Super] = [];
      }
      objectList[Super][linkHandle] = link;
      if (!objectList[Sub]) {
         objectList[Sub] = [];
      }
      objectList[Sub][linkHandle] = link;

      update_object_position(Super);
   }
   else if (objectList[Super] && objectList[Super][linkHandle] &&
                 objectList[Super][linkHandle].attr == PrevObj) {
      delete objectList[Super][linkHandle];
      delete objectList[Sub][linkHandle];
   }
   if (PrevObj && dmz.object.isObject(PrevObj)) {
      dmz.object.destroy(PrevObj);
   }
});
