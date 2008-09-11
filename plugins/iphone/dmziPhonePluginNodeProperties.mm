#import "dmzAppDelegate.h"
#include "dmziPhonePluginNodeProperties.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#import "NAEditController.h"


dmz::iPhonePluginNodeProperties::iPhonePluginNodeProperties (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      MessageObserver (Info),
      _log (Info),
      _defs (Info, &_log),
      _objectModule (0),
      _objectModuleName (),
      _created (False),
      _objectAttrHandle (0),
      _createdAttrHandle (0),
      _nameAttrHandle (0),
      _descriptionAttrHandle (0),
      _eliminationCostAttrHandle (0),
      _consequenceAttrHandle (0),
      _flowAttrHandle (0),
      _flowForwardMask (),
      _flowReverseMask (),
      _degreeCalc (0),
      _editController (nil) {

   _init (local);
}


dmz::iPhonePluginNodeProperties::~iPhonePluginNodeProperties () {

   if (_degreeCalc) { delete _degreeCalc; _degreeCalc = 0; }
   
   if (_editController) { [_editController release]; _editController = nil; }
}


// Plugin Interface
void
dmz::iPhonePluginNodeProperties::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_objectModule) {

         _objectModule = ObjectModule::cast (PluginPtr, _objectModuleName);

         if (_objectModule && _degreeCalc) {

            _degreeCalc->store_object_module (_objectModule);
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_objectModule && (_objectModule == ObjectModule::cast (PluginPtr))) {

         _objectModule = 0;
         if (_degreeCalc) { _degreeCalc->store_object_module (0); }        
      }
   }
}


// Message Observer Interface
void
dmz::iPhonePluginNodeProperties::receive_message (
      const Message &Msg,
      const UInt32 MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (Msg == _editObjectMessage) {

      if (InData && _objectModule) {

         Handle objHandle (0);

         if (InData->lookup_handle (_objectAttrHandle, 0, objHandle)) {

            _created = False;
            Handle createdHandle (0);
            InData->lookup_handle (_createdAttrHandle, 0, createdHandle);
            if (createdHandle) { _created = True; }

            if (_objectModule->is_object (objHandle)) {

               _edit_node (objHandle);
            }
            else if (_objectModule->is_link (objHandle)) {

               _edit_link (objHandle);
            }
         }
      }
   }
}


void
dmz::iPhonePluginNodeProperties::_edit_node (const Handle ObjectHandle) {

   if (_objectModule) {

//      NodeStruct ns;
//      _get_node (ObjectHandle, ns);

      // QDialog dialog (_mainWindowModule->get_widget ());
      // 
      // Ui::nodeForm ui;
      // ui.setupUi (&dialog);
      // 
      // ui.nameLineEdit->setText (ns.name);
      // ui.descriptionTextEdit->setPlainText (ns.description);
      // ui.eliminationCostSpinBox->setValue (ns.eliminationCost);
      // ui.consequenceSpinBox->setValue (ns.consequence);
      // ui.degreeLabel->setText (ns.degree);

      if (!_editController) {
         
         _editController = [[NAEditController alloc] initWithNibName:@"NAEditController" bundle:nil];
      }
      
      UINavigationController *nc = [[UINavigationController alloc] initWithRootViewController:_editController];
      
      [_editController nodeToEdit:ObjectHandle plugin:this];
      
      dmzAppDelegate *app = [dmzAppDelegate shared_dmz_app];
      [app.rootController presentModalViewController:nc animated:YES];
      
      [nc release];
      
      if (1 /*dialog.exec () == QDialog::Accepted*/) {

         // ns.name = ui.nameLineEdit->text ();
         // ns.description = ui.descriptionTextEdit->toPlainText ();
         // ns.eliminationCost = ui.eliminationCostSpinBox->value ();
         // ns.consequence = ui.consequenceSpinBox->value ();

//         _update_node (ObjectHandle, ns);
      }
//      else if (_created) { _objectModule->destroy_object (ObjectHandle); }
   }
}


void
dmz::iPhonePluginNodeProperties::_edit_link (const Handle LinkHandle) {

   if (_objectModule) {

      LinkStruct ls;
      _get_link (LinkHandle, ls);

      // QDialog dialog (_mainWindowModule->get_widget ());
      // 
      // Ui::linkForm ui;
      // ui.setupUi (&dialog);
      // 
      // ui.nameLineEdit->setText (ls.name);
      // ui.eliminationCostSpinBox->setValue (ls.eliminationCost);
      // ui.consequenceSpinBox->setValue (ls.consequence);
      // 
      // Int32 index = ui.flowDirectionComboBox->findText (ls.flow, Qt::MatchContains);
      // if (index < 0) { index = 0; }
      // ui.flowDirectionComboBox->setCurrentIndex (index);

      Handle attrHandle, superHandle, subHandle;
      String superName, subName;

      _objectModule->lookup_linked_objects (
         LinkHandle,
         attrHandle,
         superHandle,
         subHandle);

      _objectModule->lookup_text (superHandle, _nameAttrHandle, superName);
      _objectModule->lookup_text (subHandle, _nameAttrHandle, subName);

      // ui.fromLabel->setText (superName.get_buffer ());
      // ui.toLabel->setText (subName.get_buffer ());

      if (1 /*dialog.exec () == QDialog::Accepted*/) {

         // ls.name = ui.nameLineEdit->text ();
         // ls.eliminationCost = ui.eliminationCostSpinBox->value ();
         // ls.consequence = ui.consequenceSpinBox->value ();
         // ls.flow = ui.flowDirectionComboBox->currentText ();

         _update_link (LinkHandle, ls);
      }
      else if (_created) { _objectModule->unlink_objects (LinkHandle); }
   }
}


void
dmz::iPhonePluginNodeProperties::get_node (const Handle ObjectHandle, NodeStruct &ns) {

   if (_objectModule) {

      String text;
      _objectModule->lookup_text (ObjectHandle, _nameAttrHandle, text);      
      ns.name = text;

      _objectModule->lookup_text (ObjectHandle, _descriptionAttrHandle, text.flush ());
//      ns.description = text.get_buffer ();

      _objectModule->lookup_scalar (
         ObjectHandle,
         _eliminationCostAttrHandle,
         ns.eliminationCost);

      _objectModule->lookup_scalar (ObjectHandle, _consequenceAttrHandle, ns.consequence);

//      ns.degree.setNum (_degreeCalc ? _degreeCalc->calculate (ObjectHandle) : 0.0);
   }
}


void
dmz::iPhonePluginNodeProperties::_get_link (const Handle LinkHandle, LinkStruct &ls) {

   if (_objectModule) {

      Handle linkAttrObject = _objectModule->lookup_link_attribute_object (LinkHandle);

      if (linkAttrObject) {

         String text;

         _objectModule->lookup_text (linkAttrObject, _nameAttrHandle, text);
//         ls.name = text.get_buffer ();

         _objectModule->lookup_scalar (
            linkAttrObject,
            _eliminationCostAttrHandle,
            ls.eliminationCost);

         _objectModule->lookup_scalar (
            linkAttrObject,
            _consequenceAttrHandle,
            ls.consequence);

         Mask state;
         _objectModule->lookup_state (linkAttrObject, _flowAttrHandle, state);

         if (state.contains (_flowForwardMask) && state.contains (_flowReverseMask)) {

            ls.flow = "both";
         }
         else if (state.contains (_flowForwardMask)) {

            ls.flow = "forward";
         }
         else if (state.contains (_flowReverseMask)) {

            ls.flow = "reverse";
         }
         else {

            ls.flow = "both";
         }
      }
   }
}


void
dmz::iPhonePluginNodeProperties::update_node (
      const Handle ObjectHandle,
      const NodeStruct &Ns) {

   if (_objectModule) {

      _objectModule->store_text (ObjectHandle, _nameAttrHandle, Ns.name);

//      _objectModule->store_text (
//         ObjectHandle,
//         _descriptionAttrHandle,
//         Ns.description);

      _objectModule->store_scalar (
         ObjectHandle,
         _eliminationCostAttrHandle,
         Ns.eliminationCost);

      _objectModule->store_scalar (ObjectHandle, _consequenceAttrHandle, Ns.consequence);
   }
}


void
dmz::iPhonePluginNodeProperties::_update_link (
      const Handle LinkHandle,
      const LinkStruct &Ls) {

   if (_objectModule) {

      Handle linkAttrObject = _objectModule->lookup_link_attribute_object (LinkHandle);

      if (linkAttrObject) {

         _objectModule->store_text (
            linkAttrObject,
            _nameAttrHandle,
            Ls.name);

         _objectModule->store_scalar (
            linkAttrObject,
            _eliminationCostAttrHandle,
            Ls.eliminationCost);

         _objectModule->store_scalar (
            linkAttrObject,
            _consequenceAttrHandle,
            Ls.consequence);

         Mask state;
         _objectModule->lookup_state (linkAttrObject, _flowAttrHandle, state);

         /*
         if (Ls.flow.contains ("forward", Qt::CaseInsensitive)) {

            state |= _flowForwardMask;
            state &= ~_flowReverseMask;
         }
         else if (Ls.flow.contains ("reverse", Qt::CaseInsensitive)) {

            state &= ~_flowForwardMask;
            state |= _flowReverseMask;
         }
         else if (Ls.flow.contains ("both", Qt::CaseInsensitive)) {

            state |= _flowForwardMask | _flowReverseMask;
         }
         else {

            state |= _flowForwardMask | _flowReverseMask;
         }
         */
         _objectModule->store_state (linkAttrObject, _flowAttrHandle, state);
      }
   }
}


void
dmz::iPhonePluginNodeProperties::_init (Config &local) {

   RuntimeContext *context (get_plugin_runtime_context ());

   _objectModuleName = config_to_string ("module.object.name", local);

   _objectAttrHandle =
      config_to_named_handle ("attribute.object.name", local, "object", context);

   _createdAttrHandle =
      config_to_named_handle ("attribute.created.name", local, "created", context);

   _nameAttrHandle =
      config_to_named_handle ("attribute.node.name", local, "NA_Node_Name", context);

   _descriptionAttrHandle =
      config_to_named_handle (
         "attribute.node.description",
         local,
         "NA_Node_Description",
         context);

   _eliminationCostAttrHandle =
      config_to_named_handle (
         "attribute.node.eliminationCost",
         local,
         "NA_Node_Elimination_Cost",
         context);

   _consequenceAttrHandle =
      config_to_named_handle (
         "attribute.node.consequence",
         local,
         "NA_Node_Consequence",
         context);

   _flowAttrHandle =
      config_to_named_handle (
         "attribute.node.flow",
         local,
         "NA_Link_Flow",
         context);

   _editObjectMessage =
      config_create_message_type (
         "message.edit",
         local,
         "EditObjectAttributesMessage",
         context);

   subscribe_to_message (_editObjectMessage);

   _defs.lookup_state (
      config_to_string ("mask.flowForward", local, "NA_Flow_Forward"),
      _flowForwardMask);

   _defs.lookup_state (
      config_to_string ("mask.flowReverse", local, "NA_Flow_Reverse"),
      _flowReverseMask);

   _degreeCalc = config_to_object_attribute_calculator ("degrees", local, context, &_log);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmziPhonePluginNodeProperties (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::iPhonePluginNodeProperties (Info, local);
}

};
