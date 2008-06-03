#include "dmzMBRAPluginFaultTreeBuilder.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzRenderModulePick.h>
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>
#include "ui_dmzMBRAThreatProperties.h"
#include "ui_dmzMBRAComponentProperties.h"


dmz::MBRAPluginFaultTreeBuilder::MBRAPluginFaultTreeBuilder (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      MessageObserver (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _appState (Info),
      _defs (Info),
      _undo (Info),
      _canvasModule (0),
      _canvasModuleName (),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _root (0),
      _rootType (),
      _defaultAttrHandle (0),
      _objectAttrHandle (0),
      _linkAttrHandle (0),
      _logicAttrHandle (0),
      _nameAttrHandle (0),
      _eliminationCostAttrHandle (0),
      _consequenceAttrHandle (0),
      _threatAttrHandle (0),
      _vulnerabilityAttrHandle (0),
      _componentAddMessage (),
      _componentEditMessage (),
      _componentDeleteMessage (),
      _threatAddMessage (),
      _threatEditMessage (),
      _threatDeleteMessage (),
      _logicAndMessage (),
      _logicOrMessage (),
      _componentType (),
      _threatType (),
      _logicType (),
      _logicAndMask (),
      _logicOrMask (),
      _linkTable () {

   _init (local);
}


dmz::MBRAPluginFaultTreeBuilder::~MBRAPluginFaultTreeBuilder () {

   _linkTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginFaultTreeBuilder::discover_plugin (const Plugin *PluginPtr) {

   if (!_canvasModule) {

      _canvasModule = QtModuleCanvas::cast (PluginPtr, _canvasModuleName);
   }
   
   if (!_mainWindowModule) {

      _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::remove_plugin (const Plugin *PluginPtr) {

   if (_canvasModule && (_canvasModule == QtModuleCanvas::cast (PluginPtr))) {
         
      _canvasModule = 0;
   }
   
   if (_mainWindowModule && (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {
         
      _mainWindowModule = 0;
   }
}


// Message Observer Interface
void
dmz::MBRAPluginFaultTreeBuilder::receive_message (
      const MessageType &Msg,
      const UInt32 MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (InData) {
   
      Handle obj;

      if (InData->lookup_handle (_objectAttrHandle, 0, obj)) {
         
         if (Msg == _componentAddMessage) { _component_add (obj); }
         else if (Msg == _componentEditMessage) { _component_edit (obj); }
         else if (Msg == _componentDeleteMessage) { _component_delete (obj); }
         else if (Msg == _threatAddMessage) { _threat_add (obj); }
         else if (Msg == _threatEditMessage) { _threat_edit (obj); }
         else if (Msg == _threatDeleteMessage) { _threat_delete (obj); }
         else if (Msg == _logicAndMessage) { _logic_and (obj); }
         else if (Msg == _logicOrMessage) { _logic_or (obj); }
      }
   }
}


// Object Observer Interface
void
dmz::MBRAPluginFaultTreeBuilder::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_exact_type (_rootType)) {

      _root = ObjectHandle;
         
      _log.debug << "Found Fault Tree Root: " << _root << endl;
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   if (ObjectHandle == _root) {
      
      _root = 0;
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      Int32 *count (_linkTable.lookup (SuperHandle));
         
      if (!count) {
            
         count = new Int32 (0);
            
         if (!_linkTable.store (SuperHandle, count)) { delete count; count = 0; }
      }
         
      if (count) {
         
         (*count)++;

         if (*count == 2) {
            
            // Make sure we aren't loading or undoing
            if (_appState.is_mode_normal ()) {

               _create_logic (SuperHandle);
            }
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {
      
      Int32 *count (_linkTable.lookup (SuperHandle));
         
      if (count) {
         
         (*count)--;

         if (*count == 1) {
            
            _delete_logic (SuperHandle);
         }
         else if ((*count) == 0) {
            
            _linkTable.remove (SuperHandle);
            
            delete count; count = 0;
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_store_object_module (ObjectModule &objMod) {

}


void
dmz::MBRAPluginFaultTreeBuilder::_remove_object_module (ObjectModule &objMod) {

}


void 
dmz::MBRAPluginFaultTreeBuilder::_component_add (const Handle Parent) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Parent) {
      
      const Handle UndoHandle (_undo.start_record ("Add Component"));
      
      Handle object (objMod->create_object (_componentType, ObjectLocal));
      
      if (object) {

         _component_edit (object);
         objMod->activate_object (object);
         objMod->link_objects (_linkAttrHandle, Parent, object);
      }
      
      _undo.stop_record (UndoHandle);
   }
}


dmz::Boolean
dmz::MBRAPluginFaultTreeBuilder::_component_edit (const Handle Object) {
   
   Boolean result (False);
   
   ObjectModule *objMod (get_object_module ());
   
   if (_mainWindowModule && objMod && Object) {

      String text;
      objMod->lookup_text (Object, _nameAttrHandle, text);

      QDialog dialog (_mainWindowModule->get_widget ());
      
// #ifdef Q_WS_MAC
//       dialog.setWindowFlags (Qt::Sheet);
// #endif

      Ui::componentForm ui;
      ui.setupUi (&dialog);

      ui.nameLineEdit->setText (text.get_buffer ());

      if (dialog.exec () == QDialog::Accepted) {

         const Handle UndoHandle (_undo.start_record ("Edit Component"));
         
         QString name = ui.nameLineEdit->text ();
         
         text.flush ();
         if (!name.isEmpty ()) { text = qPrintable (name); }
         
         objMod->store_text (Object, _nameAttrHandle, text);
         
         _undo.stop_record (UndoHandle);
         
         result = True;
      }
   }
   
   return result;
}


void 
dmz::MBRAPluginFaultTreeBuilder::_component_delete (const Handle Object) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Object) {

      String undoName ("Delete Component");
      
      if (Object == _root) { undoName = "Reset Fault Tree"; }
      
      const Handle UndoHandle (_undo.start_record (undoName));
      
      // delete all objects linked to this Object
      
      HandleContainer children;
      objMod->lookup_sub_links (Object, _linkAttrHandle, children);

      if (Object == _root) {

         objMod->store_text (_root, _nameAttrHandle, "Fault Tree Root");
      }
      
      Handle current (children.get_first ());
      
      while (current) {
         
         _component_delete (current);
         
         current = children.get_next ();
      }
      
      // then delete this Object
      
      if (Object != _root) {

         objMod->destroy_object (Object);
      }
      
      _undo.stop_record (UndoHandle);
   }  
}


void 
dmz::MBRAPluginFaultTreeBuilder::_threat_add (const Handle Parent) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Parent) {
      
      const Handle UndoHandle (_undo.start_record ("Add Threat"));
      
      Handle object (objMod->create_object (_threatType, ObjectLocal));
      
      if (object) {
         
         objMod->store_scalar (object, _threatAttrHandle, 1.0);
         objMod->store_scalar (object, _vulnerabilityAttrHandle, 0.0);

         _threat_edit (object);

         objMod->activate_object (object);
         objMod->link_objects (_linkAttrHandle, Parent, object);
      }
      
      _undo.stop_record (UndoHandle);
   }   
}


dmz::Boolean 
dmz::MBRAPluginFaultTreeBuilder::_threat_edit (const Handle Object) {
   
   Boolean result (False);
   
   if (_mainWindowModule && Object) {
      
      ThreatStruct ts;
      _threat_get (Object, ts);
   
      QDialog dialog (_mainWindowModule->get_widget ());

// #ifdef Q_WS_MAC
//       dialog.setWindowFlags (Qt::Sheet);
// #endif

      Ui::threatForm ui;
      ui.setupUi (&dialog);

      ui.nameLineEdit->setText (ts.name);
      ui.eliminationCostSpinBox->setValue (ts.eliminationCost);
      ui.consequenceSpinBox->setValue (ts.consequence);
      ui.threatSpinBox->setValue (ts.threat * 100);
      ui.vulnerabilitySpinBox->setValue (ts.vulnerability * 100);
      
      if (ts.eliminationCost && _vulnerabilityCalc) {
         
      }
      
      if (dialog.exec () == QDialog::Accepted) {
         
         ts.name = ui.nameLineEdit->text ();
         ts.eliminationCost = ui.eliminationCostSpinBox->value ();
         ts.consequence = ui.consequenceSpinBox->value ();
         ts.threat = ui.threatSpinBox->value () / 100.0;
         ts.vulnerability = ui.vulnerabilitySpinBox->value () / 100.0;

         _threat_update (Object, ts);
         
         result = True;
      }
   }
   
   return result;
}


void
dmz::MBRAPluginFaultTreeBuilder::_threat_get (const Handle Object, ThreatStruct &ts) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {
      
      String text;
      
      objMod->lookup_text (Object, _nameAttrHandle, text);
      ts.name = text.get_buffer ();
      
      objMod->lookup_scalar (
         Object,
         _eliminationCostAttrHandle,
         ts.eliminationCost);
         
      objMod->lookup_scalar (Object, _consequenceAttrHandle, ts.consequence);
      objMod->lookup_scalar (Object, _threatAttrHandle, ts.threat);
      objMod->lookup_scalar (Object, _vulnerabilityAttrHandle, ts.vulnerability);
   }
}


void
dmz::MBRAPluginFaultTreeBuilder::_threat_update (
      const Handle Object,
      const ThreatStruct &Ts) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      const Handle UndoHandle (_undo.start_record ("Edit Threat"));

      objMod->store_text (Object, _nameAttrHandle, qPrintable (Ts.name));
      
      objMod->store_scalar (
         Object,
         _eliminationCostAttrHandle,
         Ts.eliminationCost);
      
      objMod->store_scalar (Object, _consequenceAttrHandle, Ts.consequence);
      objMod->store_scalar (Object, _threatAttrHandle, Ts.threat);
      objMod->store_scalar (Object, _vulnerabilityAttrHandle, Ts.vulnerability);

      _undo.stop_record (UndoHandle);
   }   
}


void 
dmz::MBRAPluginFaultTreeBuilder::_threat_delete (const Handle Object) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Object) {
      
      const Handle UndoHandle (_undo.start_record ("Delete Threat"));
      
      objMod->destroy_object (Object);
      
      _undo.stop_record (UndoHandle);
   }
}


void 
dmz::MBRAPluginFaultTreeBuilder::_logic_and (const Handle Object) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Object) {
      
      Mask state;
      if (objMod->lookup_state (Object, _defaultAttrHandle, state)) {
         
         if (!state.contains (_logicAndMask)) {
            
            const Handle UndoHandle (_undo.start_record ("Create AND gate"));
            
            state |= _logicAndMask;
            state &= ~_logicOrMask;

            objMod->store_state (Object, _defaultAttrHandle, state);
            
            _undo.stop_record (UndoHandle);
         }
      }
   }
}


void 
dmz::MBRAPluginFaultTreeBuilder::_logic_or (const Handle Object) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Object) {
      
      Mask state;
      if (objMod->lookup_state (Object, _defaultAttrHandle, state)) {
         
         if (!state.contains (_logicOrMask)) {
            
            const Handle UndoHandle (_undo.start_record ("Create OR gate"));
            
            state &= ~_logicAndMask;
            state |= _logicOrMask;
            
            objMod->store_state (Object, _defaultAttrHandle, state);
            
            _undo.stop_record (UndoHandle);
         }
      }
   }
}


void 
dmz::MBRAPluginFaultTreeBuilder::_create_logic (const Handle Parent) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Parent) {
      
      Handle object (objMod->create_object (_logicType, ObjectLocal));
      
      if (object) {
         
         objMod->store_state (object, _defaultAttrHandle, _logicOrMask);
         objMod->activate_object (object);
         
         objMod->link_objects (_logicAttrHandle, Parent, object);
      }
   }
}


void 
dmz::MBRAPluginFaultTreeBuilder::_delete_logic (const Handle Parent) {
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod && Parent) {
      
      HandleContainer logic;
      objMod->lookup_sub_links (Parent, _logicAttrHandle, logic);
      
      if (logic.get_count ()) {
         
         objMod->destroy_object (logic.get_first ());
      }
   }
}


void 
dmz::MBRAPluginFaultTreeBuilder::_init (Config &local) {

   RuntimeContext *context (get_plugin_runtime_context ());

   _canvasModuleName = config_to_string ("module.canvas.name", local);
   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);
   
   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask | ObjectDestroyMask);
   
   _objectAttrHandle = config_to_named_handle (
      "attribute.object.name", local, "object", context);
   
   _linkAttrHandle = activate_object_attribute (
      config_to_string ("attribute.logic.name", local, "FT_Link"),
      ObjectLinkMask | ObjectUnlinkMask);
   
   _logicAttrHandle = config_to_named_handle (
      "attribute.logic.name", local, "FT_Logic_Link", context);
      
   _nameAttrHandle =
      config_to_named_handle ("attribute.threat.name", local, "FT_Name", context);

   _eliminationCostAttrHandle =
      config_to_named_handle (
         "attribute.threat.eliminationCost",
         local,
         "FT_Threat_Elimination_Cost",
         context);

   _consequenceAttrHandle =
      config_to_named_handle (
         "attribute.threat.consequence",
         local,
         "FT_Threat_Consequence",
         context);
         
   _threatAttrHandle =
      config_to_named_handle (
         "attribute.threat.value",
         local,
         "FT_Threat_Value",
         context);

   _vulnerabilityAttrHandle =
      config_to_named_handle (
         "attribute.vulnerability.value",
         local,
         "FT_Vulnerability_Value",
         context);

   _componentAddMessage = config_create_message_type (
      "message.component.add",
      local,
      "FTComponentAddMessage",
      context);
   
   _componentEditMessage = config_create_message_type (
      "message.component.edit",
      local,
      "FTComponentEditMessage",
      context);
   
   _componentDeleteMessage = config_create_message_type (
      "message.component.delete",
      local,
      "FTComponentDeleteMessage",
      context);

   _threatAddMessage = config_create_message_type (
      "message.threat.add",
      local,
      "FTThreatAddMessage",
      context);

   _threatEditMessage = config_create_message_type (
      "message.threat.edit",
      local,
      "FTThreatEditMessage",
      context);

   _threatDeleteMessage = config_create_message_type (
      "message.threat.delete",
      local,
      "FTThreatDeleteMessage",
      context);

   _logicAndMessage = config_create_message_type (
      "message.logic.and",
      local,
      "FTLogicAndMessage",
      context);

   _logicOrMessage = config_create_message_type (
      "message.logic.or",
      local,
      "FTLogicOrMessage",
      context);
   
   subscribe_to_message (_componentAddMessage);
   subscribe_to_message (_componentEditMessage);
   subscribe_to_message (_componentDeleteMessage);
   
   subscribe_to_message (_threatAddMessage);
   subscribe_to_message (_threatEditMessage);
   subscribe_to_message (_threatDeleteMessage);
   
   subscribe_to_message (_logicAndMessage);
   subscribe_to_message (_logicOrMessage);

   _defs.lookup_object_type (
      config_to_string ("root.type", local, "ft_component_root"), _rootType);

   _defs.lookup_object_type (
      config_to_string ("type.component", local, "ft_component"),
      _componentType);

   _defs.lookup_object_type (
      config_to_string ("type.threat", local, "ft_threat"),
      _threatType);

   _defs.lookup_object_type (
      config_to_string ("type.logic", local, "ft_logic"),
      _logicType);

   _defs.lookup_state (
      config_to_string ("state.logic.and", local, "FT_Logic_And"),
      _logicAndMask);

   _defs.lookup_state (
      config_to_string ("state.logic.or", local, "FT_Logic_Or"),
      _logicOrMask);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFaultTreeBuilder (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFaultTreeBuilder (Info, local);
}

};
