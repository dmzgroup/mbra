#ifndef DMZ_IPHONE_PLUGIN_NODE_PROPERTIES_DOT_H
#define DMZ_IPHONE_PLUGIN_NODE_PROPERTIES_DOT_H

#include <dmzObjectCalc.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesMask.h>

@class NAEditController;


namespace dmz {

   class ObjectModule;


   class iPhonePluginNodeProperties :
         public Plugin,
         public MessageObserver {

      public:
         struct NodeStruct {
            
            String name;
            Float64 eliminationCost;
            Float64 consequence;
            
            NodeStruct () : name (), eliminationCost (0.0), consequence (0.0) {;}
         };
            
         iPhonePluginNodeProperties (const PluginInfo &Info, Config &local);
         ~iPhonePluginNodeProperties ();

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

         // Class Interface
         void get_node (const Handle ObjectHandle, NodeStruct &ns);
         void update_node (const Handle ObjectHandle, const NodeStruct &Ns);
            
      protected:
         struct LinkStruct {

            String name;
            Float64 eliminationCost;
            Float64 consequence;
            String flow;

            LinkStruct () : eliminationCost (0.0), consequence (0.0) {;}
         };

         void _edit_node (const Handle ObjectHandle);
         void _edit_link (const Handle LinkHandle);

         void _get_link (const Handle LinkHandle, LinkStruct &ls);

         void _update_link (const Handle LinkHandle, const LinkStruct &Ls);

         void _init (Config &local);

         Log _log;
         Definitions _defs;
         ObjectModule *_objectModule;
         String _objectModuleName;
         Boolean _created;
         Handle _objectAttrHandle;
         Handle _createdAttrHandle;
         Handle _nameAttrHandle;
         Handle _descriptionAttrHandle;
         Handle _eliminationCostAttrHandle;
         Handle _consequenceAttrHandle;
         Handle _flowAttrHandle;
         Message _editObjectMessage;
         Mask _flowForwardMask;
         Mask _flowReverseMask;
         ObjectAttributeCalculator *_degreeCalc;
         NAEditController *_editController;

      private:
         iPhonePluginNodeProperties ();
         iPhonePluginNodeProperties (const iPhonePluginNodeProperties &);
         iPhonePluginNodeProperties &operator= (const iPhonePluginNodeProperties &);
   };
};

#endif // DMZ_IPHONE_PLUGIN_NODE_PROPERTIES_DOT_H
