#ifndef DMZI_PHONE_PLUGIN_CANVAS_OBJECT_DOT_H
#define DMZI_PHONE_PLUGIN_CANVAS_OBJECT_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHashTableHandleTemplate.h>

@class Node;
@class Edge;


namespace dmz {

   class iPhoneModuleCanvas;

   class iPhonePluginCanvasObject :
         public Plugin,
         public ObjectObserverUtil {

      public:
         iPhonePluginCanvasObject (const PluginInfo &Info, Config &local);
         ~iPhonePluginCanvasObject ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void remove_object_attribute (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Mask &AttrMask);

         virtual void link_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void unlink_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void update_link_attribute_object (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle,
            const UUID &AttributeIdentity,
            const Handle AttributeObjectHandle,
            const UUID &PrevAttributeIdentity,
            const Handle PrevAttributeObjectHandle);

         virtual void update_object_position (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Vector &Value,
            const Vector *PreviousValue);

         virtual void update_object_text (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const String &Value,
            const String *PreviousValue);

      protected:
         Boolean _find_config_from_type (Config &local, ObjectType &objType);
         
         void _init (Config &local);

         Log _log;
         Definitions _defs;
         iPhoneModuleCanvas *_canvasModule;
         String _canvasModuleName;
         Handle _defaultAttrHandle;
         Handle _linkAttrHandle;
         ObjectType _linkAttrObjectType;
         Handle _rankAttrHandle;
         HashTableHandleTemplate<Node> _objectTable;
         HashTableHandleTemplate<Edge> _linkTable;

      private:
         iPhonePluginCanvasObject ();
         iPhonePluginCanvasObject (const iPhonePluginCanvasObject &);
         iPhonePluginCanvasObject &operator= (const iPhonePluginCanvasObject &);
   };
};


#endif // DMZI_PHONE_PLUGIN_CANVAS_OBJECT_DOT_H

