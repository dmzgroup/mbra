#include "dmziPhonePluginCanvasObject.h"
#include <dmziPhoneModuleCanvas.h>
#include <dmziPhoneCanvasConsts.h>
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>
#include <dmzTypesMatrix.h>
#import "Node.h"
#import "Edge.h"


dmz::iPhonePluginCanvasObject::iPhonePluginCanvasObject (
      const PluginInfo &Info, Config &local) :
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _defs (Info, &_log),
      _canvasModule (0),
      _canvasModuleName (),
      _defaultAttrHandle (0),
      _rankAttrHandle (0),
      _objectTable (),
      _linkTable () {

   _init (local);
}


dmz::iPhonePluginCanvasObject::~iPhonePluginCanvasObject () {

   _objectTable.empty ();
   _linkTable.empty ();
}


// Plugin Interface
void
dmz::iPhonePluginCanvasObject::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::iPhonePluginCanvasObject::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvasModule) {
         
         _canvasModule = iPhoneModuleCanvas::cast (PluginPtr, _canvasModuleName);
      }      
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvasModule && (_canvasModule == iPhoneModuleCanvas::cast (PluginPtr))) {
         
         _objectTable.empty ();
         _canvasModule = 0;
      }
   }
}


// Object Observer Interface
void
dmz::iPhonePluginCanvasObject::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {
   
   Config data;
   ObjectType currentType (Type);
   
   if (_find_config_from_type (data, currentType)) {
      
      String name (currentType.get_name ());
      name << "." << ObjectHandle;
      
//      ObjectStruct *os (new ObjectStruct (ObjectHandle));
      
      Node *item = [[Node alloc] init];
      item.tag = ObjectHandle;
      
      ObjectModule *objMod (get_object_module ());
      
      if (objMod) {
         
         Vector pos;
         Matrix ori;
         
         objMod->lookup_position (ObjectHandle, _defaultAttrHandle, pos);
         objMod->lookup_orientation (ObjectHandle, _defaultAttrHandle, ori);
         
         [item setPos:CGPointMake (pos.get_x (), pos.get_z ())];
      }
      
      _objectTable.store (ObjectHandle, item);
      
      if (_canvasModule) {
      
         _canvasModule->add_item (ObjectHandle, item);
      }
   }
}


void
dmz::iPhonePluginCanvasObject::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   Node *item (_objectTable.remove (ObjectHandle));
   
   if (item) {

      if (_canvasModule) { _canvasModule->remove_item (ObjectHandle); }
      
      [item release];
      item = nil;
   }
}


void
dmz::iPhonePluginCanvasObject::remove_object_attribute (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &AttrMask) {
 
   if (AttributeHandle == _rankAttrHandle) {
      
      Node *item (_objectTable.lookup (ObjectHandle));
      
      if (item) {
         
         [item setRank:0];
      }
   }
}


void
dmz::iPhonePluginCanvasObject::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {
     
   if (AttributeHandle == _linkAttrHandle) {
      
      Node *superItem (_objectTable.lookup (SuperHandle));
      Node *subItem (_objectTable.lookup (SubHandle));
      
      Edge *link = [[Edge alloc] initWithSourceNode:superItem destNode:subItem];
      
      _linkTable.store (LinkHandle, link);
      
      if (_canvasModule) {
         
         _canvasModule->add_item(LinkHandle, link);
         
         UIView *view (_canvasModule->get_view ());
         
         if (view) {
            
            [view sendSubviewToBack:link];
         }
      }
   }
}


void
dmz::iPhonePluginCanvasObject::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {
      
      Edge *link (_linkTable.remove (LinkHandle));
      
      if (link) {
         
         [link removeFromSourceAndDest];
         [link removeFromSuperview];
         [link release];
         link = nil;
      }
   }
}


void
dmz::iPhonePluginCanvasObject::update_link_attribute_object (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle,
      const UUID &AttributeIdentity,
      const Handle AttributeObjectHandle,
      const UUID &PrevAttributeIdentity,
      const Handle PrevAttributeObjectHandle) {


}


void
dmz::iPhonePluginCanvasObject::update_object_position (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Vector &Value,
      const Vector *PreviousValue) {

   if (AttributeHandle == _defaultAttrHandle) {
      
      Node *item (_objectTable.lookup (ObjectHandle));
      
      if (item) {
            
         [item setPos:CGPointMake (Value.get_x (), Value.get_z ())];
      }
   }
}


void
dmz::iPhonePluginCanvasObject::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   if (AttributeHandle == _rankAttrHandle) {
      
      Node *item (_objectTable.lookup (ObjectHandle));
      
      if (item) {
          
         UInt32 rank (string_to_uint32 (Value)); 
         [item setRank:rank];
      }
   }
}


dmz::Boolean
dmz::iPhonePluginCanvasObject::_find_config_from_type (
      Config &local,
      ObjectType &objType) {
   
   const String Name (get_plugin_name ());
   
   Boolean found (objType.get_config ().lookup_all_config_merged (Name, local));
   
   if (!found) {
      
      ObjectType currentType (objType);
      currentType.become_parent ();
      
      while (currentType && !found) {
         
         if (currentType.get_config ().lookup_all_config_merged (Name, local)) {
            
            found = True;
            objType = currentType;
         }
         
         currentType.become_parent ();
      }
   }
   
   return found;
}


void
dmz::iPhonePluginCanvasObject::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _canvasModuleName = config_to_string ("module.canvas.name", local);
   
   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask |
      ObjectDestroyMask |
      ObjectPositionMask |
      ObjectOrientationMask);
   
   _linkAttrHandle = activate_object_attribute (
      ObjectAttributeNodeLinkName,
      ObjectLinkMask |
      ObjectUnlinkMask |
      ObjectLinkAttributeMask);

   const String TypeName (
      config_to_string ("linkAttributeObjectType.name", local, "na_link_attribute"));
   
   defs.lookup_object_type (TypeName, _linkAttrObjectType);
   
   _rankAttrHandle = activate_object_attribute (
      config_to_string ("rank.attribute", local, "NA_Node_Rank"),
      ObjectTextMask | ObjectRemoveAttributeMask);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmziPhonePluginCanvasObject (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::iPhonePluginCanvasObject (Info, local);
}

};
