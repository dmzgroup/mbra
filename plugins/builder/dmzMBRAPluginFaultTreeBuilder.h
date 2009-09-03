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
#include <dmzTypesHandleContainer.h>
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

         virtual void update_object_state (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Mask &Value,
            const Mask *PreviousValue);

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
         void _component_edit (const Handle Object, const Boolean Created);
         void _component_delete (const Handle Object);

         void _threat_add (const Handle Parent);
         void _threat_edit (const Handle Object, const Boolean Created);
         void _threat_delete (const Handle Object);

         void _logic_and (const Handle Object);
         void _logic_or (const Handle Object);

         void _create_logic (const Handle Parent);
         void _delete_logic (const Handle Parent);

         Handle _clone_component (const Handle Object, ObjectModule &objMod);
         void _empty_clip_board (const Handle NewClipBoard = 0);

         void _set_component_hide_state (
            const Handle Obj,
            const Boolean Value,
            ObjectModule &objMod);

         void _cut (const Handle Parent);
         void _copy (const Handle Parent);
         void _paste (const Handle Parent);
         void _create_from_flagged ();

         void _init (Config &local);

         Log _log;
         ApplicationStateWrapper _appState;
         Definitions _defs;
         Undo _undo;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         QtModuleCanvas *_canvasModule;
         String _canvasModuleName;
         Handle _defaultAttrHandle;
         Handle _objectDataHandle;
         Handle _createdDataHandle;
         Handle _linkAttrHandle;
         Handle _naLinkAttrHandle;
         Handle _logicAttrHandle;
         Handle _hideAttrHandle;
         Handle _naNameAttrHandle;
         Handle _ftNameAttrHandle;
         Handle _activeFTAttrHandle;
         Handle _clipBoardHandle;
         Handle _clipBoardAttrHandle;
         Handle _componentEditTarget;
         Handle _threatEditTarget;
         Int32 _cloneDepth;
         Message _componentAddMessage;
         Message _componentEditMessage;
         Message _componentDeleteMessage;
         Message _threatAddMessage;
         Message _threatEditMessage;
         Message _threatDeleteMessage;
         Message _logicAndMessage;
         Message _logicOrMessage;
         Message _cutMessage;
         Message _copyMessage;
         Message _pasteMessage;
         Message _createFromFlaggedMessage;
         ObjectType _rootType;
         ObjectType _componentType;
         ObjectType _threatType;
         ObjectType _logicType;
         ObjectType _clipBoardType;
         Mask _logicAndMask;
         Mask _logicOrMask;
         Mask _flaggedMask;
         HashTableHandleTemplate<Int32> _linkTable;
         HandleContainer _flaggedNodes;

      private:
         MBRAPluginFaultTreeBuilder ();
         MBRAPluginFaultTreeBuilder (const MBRAPluginFaultTreeBuilder &);
         MBRAPluginFaultTreeBuilder &operator= (const MBRAPluginFaultTreeBuilder &);
   };
};

#endif // DMZ_MBRA_PLUGIN_FAULT_TREE_BUILDER_DOT_H
