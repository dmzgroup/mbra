#ifndef DMZ_MBRA_PLUGIN_FAULT_TREE_BUILDER_DOT_H
#define DMZ_MBRA_PLUGIN_FAULT_TREE_BUILDER_DOT_H

#include <dmzApplicationState.h>
#include <dmzObjectCalc.h>
#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <dmzTypesMask.h>
#include <QtCore/QString>


namespace dmz {

   class QtModuleCanvas;
   class QtModuleMainWindow;


   class MBRAPluginFaultTreeBuilder :
         public Plugin,
         public MessageObserver,
         public ObjectObserverUtil {

      public:
         MBRAPluginFaultTreeBuilder (const PluginInfo &Info, Config &local);
         ~MBRAPluginFaultTreeBuilder ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Message Observer Interface
         void receive_message (
            const Message &Msg,
            const UInt32 MessageSendHandle,
            const Handle TargetObserverHandle,
            const Data *InData,
            Data *outData);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

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

      protected:
         struct ThreatStruct {

            QString name;
            Float64 eliminationCost;
            Float64 consequence;
            Float64 threat;
            Float64 vulnerability;

            ThreatStruct () :
               eliminationCost (0.0),
               consequence (0.0),
               threat (0.0),
               vulnerability (0.0) {;}
         };

         virtual void _store_object_module (ObjectModule &objMod);
         virtual void _remove_object_module (ObjectModule &objMod);

         void _component_add (const Handle Parent);
         Boolean _component_edit (const Handle Object);
         void _component_delete (const Handle Object);

         void _threat_add (const Handle Parent);
         Boolean _threat_edit (const Handle Object);
         void _threat_get (const Handle ObjectHandle, ThreatStruct &ts);
         void _threat_update (const Handle ObjectHandle, const ThreatStruct &ts);
         void _threat_delete (const Handle Object);

         void _logic_and (const Handle Object);
         void _logic_or (const Handle Object);

         void _create_logic (const Handle Parent);
         void _delete_logic (const Handle Parent);

         void _init (Config &local);

         Log _log;
         ApplicationStateWrapper _appState;
         Definitions _defs;
         Undo _undo;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         QtModuleCanvas *_canvasModule;
         String _canvasModuleName;
         Handle _root;
         ObjectType _rootType;
         Handle _defaultAttrHandle;
         Handle _objectAttrHandle;
         Handle _linkAttrHandle;
         Handle _logicAttrHandle;
         Handle _nameAttrHandle;
         Handle _eliminationCostAttrHandle;
         Handle _consequenceAttrHandle;
         Handle _threatAttrHandle;
         Handle _vulnerabilityAttrHandle;
         ObjectAttributeCalculator *_vulnerabilityCalc;
         Message _componentAddMessage;
         Message _componentEditMessage;
         Message _componentDeleteMessage;
         Message _threatAddMessage;
         Message _threatEditMessage;
         Message _threatDeleteMessage;
         Message _logicAndMessage;
         Message _logicOrMessage;
         ObjectType _componentType;
         ObjectType _threatType;
         ObjectType _logicType;
         Mask _logicAndMask;
         Mask _logicOrMask;
         HashTableHandleTemplate<Int32> _linkTable;

      private:
         MBRAPluginFaultTreeBuilder ();
         MBRAPluginFaultTreeBuilder (const MBRAPluginFaultTreeBuilder &);
         MBRAPluginFaultTreeBuilder &operator= (const MBRAPluginFaultTreeBuilder &);
   };
};

#endif // DMZ_MBRA_PLUGIN_FAULT_TREE_BUILDER_DOT_H
