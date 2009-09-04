#include "dmzMBRAPluginLinkEditor.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>

namespace {

static const int HandleRole (Qt::UserRole + 1);
static const dmz::String LinkStr (" --> ");

};


dmz::MBRAPluginLinkEditor::MBRAPluginLinkEditor (const PluginInfo &Info, Config &local) :
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      QtWidget (Info),
      _log (Info),
      _undo (Info),
      _linkAttrHandle (0),
      _naNameAttrHandle (0),
      _ftNameAttrHandle (0) {

   _ui.setupUi (this);

   _init (local);
   
   adjustSize ();
}


dmz::MBRAPluginLinkEditor::~MBRAPluginLinkEditor () {

}


// Plugin Interface
void
dmz::MBRAPluginLinkEditor::update_plugin_state (
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
dmz::MBRAPluginLinkEditor::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


// Object Observer Interface
void
dmz::MBRAPluginLinkEditor::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_type (_naNodeType) || Type.is_of_type (_ftRootType)) {

      QListWidgetItem *item = new QListWidgetItem;

      item->setData (HandleRole, (quint64)ObjectHandle);

      if (_objTable.store (ObjectHandle, item)) {

         if (Type.is_of_type (_naNodeType)) {

            _ui.NAList->addItem (item);
         }
         else if (Type.is_of_type (_ftRootType)) {

            _ui.FTList->addItem (item);
         }
      }
      else { delete item; item = 0; }
   }
}


void
dmz::MBRAPluginLinkEditor::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   QListWidgetItem *item = _objTable.remove (ObjectHandle);

   if (item) {

      ObjectType type;
      ObjectModule *objMod = get_object_module ();

      if (objMod) { type = objMod->lookup_object_type (ObjectHandle); }

      if (type.is_of_type (_naNodeType)) {

         int row = _ui.NAList->row (item);
         _ui.NAList->takeItem (row);
      }
      else if (type.is_of_type (_ftRootType)) {

         int row = _ui.FTList->row (item);
         _ui.FTList->takeItem (row);
      }

      delete item; item = 0;
   }
}


void
dmz::MBRAPluginLinkEditor::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   QListWidgetItem *super = _objTable.lookup (SuperHandle);
   QListWidgetItem *sub = _objTable.lookup (SubHandle);

   if (super && sub) {

      int row = _ui.NAList->row (super);
      _ui.NAList->takeItem (row);
      row = _ui.FTList->row (sub);
      _ui.FTList->takeItem (row);

      LinkStruct *ls = new LinkStruct;

      ls->item = new QListWidgetItem;
      ls->item->setData (HandleRole, (quint64)LinkHandle);

      if (_linkTable.store (LinkHandle, ls)) {

         _superTable.store (SuperHandle, ls);
         _subTable.store (SubHandle, ls);

         ObjectModule *objMod = get_object_module ();

         if (objMod) {

            objMod->lookup_text (SuperHandle, _naNameAttrHandle, ls->naName);
            objMod->lookup_text (SubHandle, _ftNameAttrHandle, ls->ftName);

            ls->item->setText ((ls->naName + LinkStr + ls->ftName).get_buffer ());
         }

         _ui.LinkedList->addItem (ls->item);
      }
      else { delete ls; ls = 0; }
   }
}


void
dmz::MBRAPluginLinkEditor::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   LinkStruct *ls = _linkTable.remove (LinkHandle);
   _superTable.remove (SuperHandle);
   _subTable.remove (SubHandle);

   if (ls) {

      int row = _ui.LinkedList->row (ls->item);
      _ui.LinkedList->takeItem (row);
      delete ls; ls = 0;

      QListWidgetItem *item = _objTable.lookup (SuperHandle);

      if (item) { _ui.NAList->addItem (item); }

      item = _objTable.lookup (SubHandle);

      if (item) { _ui.FTList->addItem (item); }
   }
}


void
dmz::MBRAPluginLinkEditor::update_link_attribute_object (
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
dmz::MBRAPluginLinkEditor::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

}


void
dmz::MBRAPluginLinkEditor::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   QListWidgetItem *item = _objTable.lookup (ObjectHandle);

   if (item) { item->setText (Value.get_buffer ()); }

   LinkStruct *ls = _superTable.lookup (ObjectHandle);
   if (ls) { ls->naName = Value; }
   else {

      ls = _subTable.lookup (ObjectHandle);
      if (ls) { ls->ftName = Value; }
   }

   if (ls) {

      ls->item->setText ((ls->naName + LinkStr + ls->ftName).get_buffer ());
   }
}


// QtWidget Interface
QWidget *
dmz::MBRAPluginLinkEditor::get_qt_widget () { return this; }


void
dmz::MBRAPluginLinkEditor::on_linkButton_clicked () {

   QListWidgetItem *super = _ui.NAList->currentItem ();
   QListWidgetItem *sub = _ui.FTList->currentItem ();

   if (super && sub) {

      const Handle SuperObject (super->data (HandleRole).toULongLong ());
      const Handle SubObject (sub->data (HandleRole).toULongLong ());

      ObjectModule *objMod = get_object_module ();

      if (SuperObject && SubObject && objMod) {

         const Handle UndoHandle = _undo.start_record ("Node and Fault Tree");
         objMod->link_objects (_linkAttrHandle, SuperObject, SubObject);
         _undo.stop_record (UndoHandle);
      }
   }
}


void
dmz::MBRAPluginLinkEditor::on_unlinkButton_clicked () {

   QListWidgetItem *item = _ui.LinkedList->currentItem ();

   if (item) {

      const Handle LinkHandle (item->data (HandleRole).toULongLong ());

      ObjectModule *objMod = get_object_module ();

      if (LinkHandle && objMod) {

         const Handle UndoHandle = _undo.start_record ("Unlink Node and Fault Tree");
         objMod->unlink_objects (LinkHandle);
         _undo.stop_record (UndoHandle);
      }
   }
}


void
dmz::MBRAPluginLinkEditor::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);

   _linkAttrHandle = activate_object_attribute (
      config_to_string ("link.name", local, "NA_Fault_Tree_Link"),
      ObjectLinkMask | ObjectUnlinkMask);

   _naNameAttrHandle = activate_object_attribute (
      config_to_string ("na-text.name", local, "NA_Node_Name"),
      ObjectTextMask);

   _ftNameAttrHandle = activate_object_attribute (
      config_to_string ("ft-text.name", local, "FT_Name"),
      ObjectTextMask);

   _naNodeType = config_to_object_type ("na-node-type.name", local, "na_node", context);

   _ftRootType = config_to_object_type (
      "ft-root-type.name",
      local,
      "ft_component_root",
      context);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginLinkEditor (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginLinkEditor (Info, local);
}

};
