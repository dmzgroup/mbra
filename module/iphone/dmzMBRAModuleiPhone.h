#ifndef DMZ_MBRA_MODULE_UI_IPHONE_DOT_H
#define DMZ_MBRA_MODULE_UI_IPHONE_DOT_H

#import <dmzInputEventMouse.h>
#import <dmziPhoneModuleCanvas.h>
#import <dmzRuntimeLog.h>
#import <dmzRuntimeMessaging.h>
#import <dmzRuntimePlugin.h>
#import <dmzTypesHashTableHandleTemplate.h>
//#include <dmzTypesMask.h>


@class MBRAController;
@class NAController;
@class CanvasView;


namespace dmz {

   class InputModule;
   class ObjectModule;

   class MBRAModuleiPhone :
         public Plugin,
//         public MessageObserver,
         public iPhoneModuleCanvas {

      public:
         static MBRAModuleiPhone *get_instance ();
            
         MBRAModuleiPhone (const PluginInfo &Info, Config &local);
         ~MBRAModuleiPhone ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);
            
            // Message Observer Interface
//         virtual void receive_message (
//             const Message &Msg,
//             const UInt32 MessageSendHandle,
//             const Handle TargetObserverHandle,
//             const Data *InData,
//             Data *outData);
         
         // iPhoneModuleCanvas Interface
         virtual UIView *get_view () const;
         virtual Boolean add_item (const Handle ObjectHandle, UIView *item);
         virtual UIView *lookup_item (const Handle ObjectHandle);
         virtual UIView *remove_item (const Handle ObjectHandle);

         // class Interface
         void set_mode (const NSInteger Mode);
         void calculate (const BOOL Value);

         void touches_began (NSSet *touches, UIEvent *event);
         void touches_moved (NSSet *touches, UIEvent *event);
         void touches_ended (NSSet *touches, UIEvent *event);
            
      protected:
         static MBRAModuleiPhone *_instance;
            
         void _handle_mouse_event (NSSet *touches, UIEvent *event);
         void _init (Config &local);

         Log _log;
         InputModule *_inputModule;
         String _inputModuleName;
//         ObjectModule *_objectModule;
//         String _objectModuleName;
         Handle _channel;
         UINavigationController *_navigationController;
         NAController *_naController;
         InputEventMouse _mouseEvent;
         UInt32 _buttonMask;
         HashTableHandleTemplate<UIView> _itemTable;
         Int32 _mode;
         Handle _addChannel;
         Handle _editChannel;
         Handle _linkChannel;
         Handle _deleteChannel;
         Message _calculateOnMessage;
         Message _calculateOffMessage;
         Handle _calculateTarget;
//         Boolean _created;
//         Handle _objectAttrHandle;
//         Handle _createdAttrHandle;
//         Handle _nameAttrHandle;
//         Handle _descriptionAttrHandle;
//         Handle _eliminationCostAttrHandle;
//         Handle _consequenceAttrHandle;
//         Handle _flowAttrHandle;
//         Message _editObjectMessage;
//         Mask _flowForwardMask;
//         Mask _flowReverseMask;
//         ObjectAttributeCalculator *_degreeCalc;
            
      private:
         MBRAModuleiPhone ();
         MBRAModuleiPhone (const MBRAModuleiPhone &);
         MBRAModuleiPhone &operator= (const MBRAModuleiPhone &);

   };
};


#endif // DMZ_MBRA_MODULE_UI_IPHONE_DOT_H
