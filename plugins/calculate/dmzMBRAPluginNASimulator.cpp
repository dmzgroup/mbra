#include "dmzMBRAPluginNASimulator.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtConfigRead.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginNASimulator::MBRAPluginNASimulator (
      const PluginInfo &Info,
      Config &local) :
      QFrame (0),
      QtWidget (Info),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _simulatorHandle (0),
      _simulatorType (),
      _weightByHandles (),
      _weightByGroup (),
      _objectiveFunctionHandles (),
      _ignoreUpdates (False) {

   _ui.setupUi (this);
   
   _init (local);
}


dmz::MBRAPluginNASimulator::~MBRAPluginNASimulator () {

}


// QtWidget Interface
QWidget *
dmz::MBRAPluginNASimulator::get_qt_widget () {

   return this;
}


// Plugin Interface
void
dmz::MBRAPluginNASimulator::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

      ObjectModule *objMod = get_object_module ();

      if (objMod && !_simulatorHandle) {
         
         _simulatorHandle = objMod->create_object (_simulatorType, ObjectLocal);
         objMod->activate_object (_simulatorHandle);
      }
   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginNASimulator::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


// Object Observer Interface
void
dmz::MBRAPluginNASimulator::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

   if (!_ignoreUpdates) {
      
      if (_weightByHandles.contains (AttributeHandle)) {

         QAbstractButton *button =
            _weightByGroup.button ((qlonglong)AttributeHandle);

         if (button) { button->setChecked (Value); }
      }
      else if (_objectiveFunctionHandles.contains (AttributeHandle) && Value) {

         Int32 index = _objectiveFunctionHandles.indexOf (AttributeHandle);

         if (index != -1) { _ui.objectiveComboBox->setCurrentIndex (index); }
      }
   }
}


void
dmz::MBRAPluginNASimulator::_slot_weight_by_clicked (int id) {

   ObjectModule *objMod = get_object_module ();

   if (objMod && _simulatorHandle) {

      const Handle AttrHandle (id);
      
      QAbstractButton *button = _weightByGroup.button (id);
      
      if (button) {
         
         Boolean value = button->isChecked ();
         _ignoreUpdates = True;
         objMod->store_flag (_simulatorHandle, AttrHandle, value);
         _ignoreUpdates = False;
      }
   }
}


void
dmz::MBRAPluginNASimulator::on_objectiveComboBox_currentIndexChanged (int id) {

   ObjectModule *objMod = get_object_module ();

   if (objMod && _simulatorHandle) {
      
      const Handle ActiveHandle (_ui.objectiveComboBox->itemData (id).toLongLong ());

      foreach (Handle attrHandle, _objectiveFunctionHandles) {
         
         Boolean value (False);
         if (ActiveHandle == attrHandle) { value = True; }
         
         _ignoreUpdates = True;
         objMod->store_flag (_simulatorHandle, attrHandle, value);
         _ignoreUpdates = False;
      }
   }
}


void
dmz::MBRAPluginNASimulator::_init (Config &local) {

   setObjectName (get_plugin_name ().get_buffer ());

   qframe_config_read ("frame", local, this);

   QVBoxLayout *layout = new QVBoxLayout;
   
   _weightByGroup.setExclusive (False);
   
   connect (
      &_weightByGroup, SIGNAL (buttonClicked (int)),
      this, SLOT (_slot_weight_by_clicked (int)));
      
   QVBoxLayout *vbox = new QVBoxLayout;
   
   Config itemList;
   
   if (local.lookup_all_config ("weight-by", itemList)) {
      
      ConfigIterator it;
      Config cd;
      
      while (itemList.get_next_config (it, cd)) {
         
         const String Text (config_to_string ("text", cd));
         
         const String AttrName (config_to_string ("attribute", cd));
         
         const Boolean Checked (config_to_boolean ("checked", cd));
         
         if (Text && AttrName) {
            
            const Handle AttrHandle (activate_object_attribute (AttrName, ObjectFlagMask));
            
            QCheckBox *checkBox = new QCheckBox (Text.get_buffer ());
            checkBox->setObjectName (Text.get_buffer ());
            checkBox->setChecked (Checked);
            
            vbox->addWidget (checkBox);
            _weightByGroup.addButton (checkBox, (qlonglong)AttrHandle);
            
            _weightByHandles.add_handle (AttrHandle);
         }
      }
   }
   
   _ui.weightGroupBox->setLayout (vbox);
      

   if (local.lookup_all_config ("objective-function", itemList)) {
      
      ConfigIterator it;
      Config cd;
      
      while (itemList.get_next_config (it, cd)) {
         
         const String Text (config_to_string ("text", cd));
         
         const String AttrName (config_to_string ("attribute", cd));
         
         if (Text && AttrName) {
            
            const Handle AttrHandle (activate_object_attribute (AttrName, ObjectFlagMask));

            _ui.objectiveComboBox->addItem (Text.get_buffer (), (qlonglong)AttrHandle);
            _objectiveFunctionHandles.append (AttrHandle);
         }
      }
   }
   
   _simulatorType = config_to_object_type (
      "type.simulator", local, "na_simulator", get_plugin_runtime_context ());
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginNASimulator (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginNASimulator (Info, local);
}

};
