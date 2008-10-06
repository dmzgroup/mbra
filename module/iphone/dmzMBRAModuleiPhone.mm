#import "CanvasView.h"
#import "Constants.h"
#import "dmzAppDelegate.h"
#include <dmzInputConsts.h>
#include <dmzInputModule.h>
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#import "dmzMBRAModuleiPhone.h"
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#import <dmzRuntimePluginFactoryLinkSymbol.h>
#import <dmzRuntimePluginInfo.h>
#import "MBRAController.h"
#import "NAController.h"


dmz::MBRAModuleiPhone *localInstance (0);
dmz::MBRAModuleiPhone *dmz::MBRAModuleiPhone::_instance (0);


dmz::MBRAModuleiPhone::MBRAModuleiPhone (const PluginInfo &Info, Config &local) :
      Plugin (Info),
//      MessageObserver (Info),
      iPhoneModuleCanvas (Info),
      _log (Info),
      _inputModule (0),
      _inputModuleName (),
//      _objectModule (0),
//      _objectModuleName (),
      _channel (0),
      _navigationController (0),
      _naController (0),
      _mouseEvent (),
      _buttonMask (0),
      _itemTable (),
      _mode (kModeAdd),
      _addChannel (0),
      _editChannel (0),
      _linkChannel (0),
      _deleteChannel (0),
      _calculateOnMessage (),
      _calculateOffMessage (),
      _calculateTarget (0) {
//      _created (False),
//      _objectAttrHandle (0),
//      _createdAttrHandle (0),
//      _nameAttrHandle (0),
//      _descriptionAttrHandle (0),
//      _eliminationCostAttrHandle (0),
//      _consequenceAttrHandle (0),
//      _flowAttrHandle (0),
//      _flowForwardMask (),
//      _flowReverseMask (),
//      _degreeCalc (0) {
         
   if (!_instance) {
      
      _instance = this;
      localInstance = _instance;
   }

   _naController = [[NAController alloc] initWithNibName:@"NAController" bundle:nil];
      
   _navigationController = [[UINavigationController alloc]
      initWithRootViewController:_naController];
   
   dmzAppDelegate *app = [dmzAppDelegate shared_dmz_app];
   app.rootController = _navigationController;
         
   _init (local);
}


dmz::MBRAModuleiPhone::~MBRAModuleiPhone () {

//   HashTableHandleIterator it;
//   
//   UIView *item (_itemTable.get_first (it));
//   
//   while (item) {
//      
//      [item removeFromSuperview];
//      [item release];
//      item = _itemTable.get_next (it);
//   }
  
   _itemTable.clear ();
   
   [_naController release];
   [_navigationController release];
   
//   if (_degreeCalc) { delete _degreeCalc; _degreeCalc = 0; }
   
   if (localInstance) {
      
      _instance = 0;
      localInstance = 0;
   }   
}


dmz::MBRAModuleiPhone *
dmz::MBRAModuleiPhone::get_instance () { return _instance; }


// Plugin Interface
void
dmz::MBRAModuleiPhone::update_plugin_state (
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
dmz::MBRAModuleiPhone::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {
  
   if (Mode == PluginDiscoverAdd) {

//      if (!_objectModule) {
//         
//         _objectModule = ObjectModule::cast (PluginPtr, _objectModuleName);
//         
//         if (_objectModule && _degreeCalc) {
//            
//            _degreeCalc->store_object_module (_objectModule);
//         }
//      }
      
      if (!_inputModule) {
         
         _inputModule = InputModule::cast (PluginPtr, _inputModuleName);
         set_mode (_mode);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

//      if (_objectModule && (_objectModule == ObjectModule::cast (PluginPtr))) {
//         
//         _objectModule = 0;
//         if (_degreeCalc) { _degreeCalc->store_object_module (0); }
//      }
      
      if (_inputModule && (_inputModule == InputModule::cast (PluginPtr))) {
         
         _inputModule = 0;
      }
   }
}


// Message Observer Interface
//void
//dmz::MBRAModuleiPhone::receive_message (
//      const Message &Msg,
//      const UInt32 MessageSendHandle,
//      const Handle TargetObserverHandle,
//      const Data *InData,
//      Data *outData) {
//   
//_log.warn << "-=-=-=-=-=-=-=--= receive_mesage: " << Msg.get_name () << endl;
//   if (Msg == _editObjectMessage) {
//      
//      if (InData && _objectModule) {
//         
//         Handle objHandle (0);
//         
//         if (InData->lookup_handle (_objectAttrHandle, 0, objHandle)) {
//            
//            _created = False;
//            Handle createdHandle (0);
//            InData->lookup_handle (_createdAttrHandle, 0, createdHandle);
//            if (createdHandle) { _created = True; }
//            
//            if (_objectModule->is_object (objHandle)) {
//               
//               _log.warn << "-=-=-=-=-=-=-=--= edit_node: " << objHandle << endl;
////               _edit_node (objHandle);
//            }
////            else if (_objectModule->is_link (objHandle)) {
////               
////               _edit_link (objHandle);
////            }
//         }
//      }
//   }
//}


// iPhoneModuleCanvas Interface
UIView *
dmz::MBRAModuleiPhone::get_view () const {

   return (UIView *)_naController.canvas.view;
}


dmz::Boolean
dmz::MBRAModuleiPhone::add_item (const Handle ObjectHandle, UIView *item) {
   
   Boolean retVal (False);
   
   if (item) {
      
      retVal = _itemTable.store (ObjectHandle, item);
      
      if (retVal) {
         
         [item retain];
         
         [_naController.canvas.view addSubview:item];
      }
   }
   
   return retVal;
}


UIView *
dmz::MBRAModuleiPhone::lookup_item (const Handle ObjectHandle) {
   
   UIView *item (_itemTable.lookup (ObjectHandle));
   return item;
}


UIView *
dmz::MBRAModuleiPhone::remove_item (const Handle ObjectHandle) {
   
   UIView *item (_itemTable.remove (ObjectHandle));
   
   if (item) {
      
      [item removeFromSuperview];
      [item release];
   }
   
   return item;
}


// class Interface
#if 0
void
dmz::MBRAModuleiPhone::show_network_analysis (const UInt32 Index) {
   
//   [[_rootController navigationController] pushViewController:_naController animated:YES];
}


dmz::UInt32
dmz::MBRAModuleiPhone::get_na_count () {
   
   return 3;
}


NSString *
dmz::MBRAModuleiPhone::get_na_name (const UInt32 Index) {
   
   String name ("Analysis ");
   name << Index+1;
   
   return [NSString stringWithUTF8String:name.get_buffer ()];
}
#endif


void
dmz::MBRAModuleiPhone::set_mode (const NSInteger Mode) {
   
   Boolean addMode (False);
   Boolean editMode (False);
   Boolean linkMode (False);
   Boolean deleteMode (False);
   
   if (Mode == kModeAdd) { addMode = True; }
   else if (Mode == kModeEdit) { editMode = True; }
   else if (Mode == kModeLink) { linkMode  = True; }
   else if (Mode == kModeDelete) { deleteMode = True; }
   
   if (_inputModule) {
      
      _mode = Mode;
      
      _inputModule->set_channel_state (_addChannel, addMode);
      _inputModule->set_channel_state (_editChannel, editMode);
      _inputModule->set_channel_state (_linkChannel, linkMode);
      _inputModule->set_channel_state (_deleteChannel, deleteMode);
   }
}


void
dmz::MBRAModuleiPhone::calculate (const BOOL On) {
   
   if (On) {
   
      Data data;
      _calculateOnMessage.send (_calculateTarget, &data, 0);
   }
   else {
      
      Data data;
      _calculateOffMessage.send (_calculateTarget, &data, 0);
   }
}


void
dmz::MBRAModuleiPhone::touches_began (NSSet *touches, UIEvent *event) {

   _buttonMask |= 0x01 << 0;
   _handle_mouse_event (touches, event);
}


void
dmz::MBRAModuleiPhone::touches_moved (NSSet *touches, UIEvent *event) {

   
   _handle_mouse_event (touches, event);
}


void
dmz::MBRAModuleiPhone::touches_ended (NSSet *touches, UIEvent *event) {
      
   _buttonMask = 0;
   _handle_mouse_event (touches, event);
}


void
dmz::MBRAModuleiPhone::_handle_mouse_event (NSSet *touches, UIEvent *event) {

   if (_inputModule) {
      
      InputEventMouse event (_mouseEvent);
      
      UITouch *touch = [touches anyObject];
      
      UIView *view = _naController.canvas;
      
      CGPoint pointOnCanvas = [touch locationInView:view];
      CGPoint pointOnScreen = [view convertPoint:pointOnCanvas toView:nil];
           
//_log.warn << "pointOnCanvas: " << pointOnCanvas.x << " - " << pointOnCanvas.y << endl;
//_log.warn << "pointOnScreen: " << pointOnScreen.x << " - " << pointOnScreen.y << endl;
      
      event.set_mouse_position (pointOnScreen.x, pointOnScreen.y);
      event.set_mouse_screen_position (pointOnScreen.x, pointOnScreen.y);
      
      event.set_button_mask (_buttonMask);
      
      event.set_scroll_delta (0, 0);
      
      event.set_window_size (view.frame.size.width, view.frame.size.height);
      
      if (_mouseEvent.update (event)) {
         
//_log.warn << "send_mouse_event: " << _mouseEvent.get_source_handle () << endl;
         _inputModule->send_mouse_event (_mouseEvent);
      }
   }   
}


void
dmz::MBRAModuleiPhone::_init (Config &local) {

   RuntimeContext *context (get_plugin_runtime_context ());
   Definitions defs (get_plugin_runtime_context ());
   
//   _objectModuleName = config_to_string ("module.object.name", local);

   _inputModuleName = config_to_string ("module.input.name", local);
   
   _channel = config_to_named_handle (
      "channel.name",
     local,
     InputChannelDefaultName,
     context);
   
   _mouseEvent.set_source_handle (get_plugin_handle ());
   
   _addChannel = defs.create_named_handle (
      config_to_string ("mode.add", local, "NA_Create_Object_Channel"));

   _editChannel = defs.create_named_handle (
      config_to_string ("mode.edit", local, "NA_Default_Channel"));
   
   _linkChannel = defs.create_named_handle (
      config_to_string ("mode.link", local, "NA_Link_Objects_Channel"));
   
   _deleteChannel = defs.create_named_handle (
      config_to_string ("mode.delete", local, "NA_Destroy_Object_Channel"));
   
   _calculateOnMessage = config_create_message_type (
      "calculate.on",
      local,
      "NARankObjectsMessage",
      context,
      &_log);
   
   _calculateOffMessage = config_create_message_type (
      "calculate.off",
      local,
      "NAHideObjectsMessage",
      context,
      &_log);

   _calculateTarget = config_to_named_handle (
      "calculate.target",
      local,
      "dmzMBRAPluginNARanking",
      context);
   
//   _objectAttrHandle =
//      config_to_named_handle ("attribute.object.name", local, "object", context);
//   
//   _createdAttrHandle =
//      config_to_named_handle ("attribute.created.name", local, "created", context);
//   
//   _nameAttrHandle =
//      config_to_named_handle ("attribute.node.name", local, "NA_Node_Name", context);
//   
//   _descriptionAttrHandle = 
//      config_to_named_handle (
//         "attribute.node.description",
//         local,
//         "NA_Node_Description",
//         context);
//   
//   _eliminationCostAttrHandle =
//      config_to_named_handle (
//         "attribute.node.eliminationCost",
//         local,
//         "NA_Node_Elimination_Cost",
//         context);
//   
//   _consequenceAttrHandle =
//      config_to_named_handle (
//         "attribute.node.consequence",
//         local,
//         "NA_Node_Consequence",
//         context);
//   
//   _flowAttrHandle =
//      config_to_named_handle (
//         "attribute.node.flow",
//         local,
//         "NA_Link_Flow",
//         context);
//   
//   _editObjectMessage =
//      config_create_message_type (
//         "message.edit",
//         local,
//         "EditObjectAttributesMessage",
//         context);
//   
//   subscribe_to_message (_editObjectMessage);
//   
//   defs.lookup_state (
//      config_to_string ("mask.flowForward", local, "NA_Flow_Forward"),
//      _flowForwardMask);
//   
//   defs.lookup_state (
//      config_to_string ("mask.flowReverse", local, "NA_Flow_Reverse"),
//      _flowReverseMask);
//   
//   _degreeCalc = config_to_object_attribute_calculator ("degrees", local, context, &_log);   
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAModuleiPhone (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAModuleiPhone (Info, local);
}

};
