#ifndef DMZ_MBRA_PLUGIN_NODE_PROPERTIES_DOT_H
#define DMZ_MBRA_PLUGIN_NODE_PROPERTIES_DOT_H

#include <dmzObjectCalc.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesMask.h>
#include <QtCore/QString>

class QMainWindow;

namespace dmz {

   class ObjectModule;
   class QtModuleMainWindow;

         
   class MBRAPluginNodeProperties :
         public Plugin,
         public MessageObserver {

      public:
         MBRAPluginNodeProperties (const PluginInfo &Info, Config &local);
         ~MBRAPluginNodeProperties ();

         // Plugin Interface
         virtual void discover_plugin (const Plugin *PluginPtr);
         virtual void start_plugin ();
         virtual void stop_plugin ();
         virtual void shutdown_plugin ();
         virtual void remove_plugin (const Plugin *PluginPtr);

         // Message Observer Interface
         void receive_message (
            const MessageType &Msg,
            const UInt32 MessageSendHandle,
            const Handle TargetObserverHandle,
            const Data *InData,
            Data *outData);

      protected:
         struct NodeStruct {
            
            QString name;
            QString description;
            Float64 eliminationCost;
            Float64 consequence;
            QString degree;
            
            NodeStruct () : eliminationCost (0.0), consequence (0.0) {;}
         };
         
         struct LinkStruct {
         
            QString name;
            Float64 eliminationCost;
            Float64 consequence;
            QString flow;
            
            LinkStruct () : eliminationCost (0.0), consequence (0.0) {;}
         };
         
         void _edit_node (const Handle ObjectHandle);
         void _edit_link (const Handle LinkHandle);
         
         void _get_node (const Handle ObjectHandle, NodeStruct &ns);
         void _get_link (const Handle LinkHandle, LinkStruct &ls);
         
         void _update_node (const Handle ObjectHandle, const NodeStruct &Ns);
         void _update_link (const Handle LinkHandle, const LinkStruct &Ls);
         
         void _init (Config &local);

         Log _log;
         Undo _undo;
         Definitions _defs;
         ObjectModule *_objectModule;
         String _objectModuleName;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Handle _objectAttrHandle;
         Handle _nameAttrHandle;
         Handle _descriptionAttrHandle;
         Handle _eliminationCostAttrHandle;
         Handle _consequenceAttrHandle;
         Handle _flowAttrHandle;
         MessageType _editObjectMessageType;
         Mask _flowForwardMask;
         Mask _flowReverseMask;
         ObjectAttributeCalculator *_degreeCalc;
         
      private:
         MBRAPluginNodeProperties ();
         MBRAPluginNodeProperties (const MBRAPluginNodeProperties &);
         MBRAPluginNodeProperties &operator= (const MBRAPluginNodeProperties &);

   };
};

#endif // DMZ_MBRA_PLUGIN_NODE_PROPERTIES_DOT_H
