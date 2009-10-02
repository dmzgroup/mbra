#include <dmzQtConfigRead.h>
#include "dmzMBRAPluginPreferencesGeneral.h"
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginPreferencesGeneral::MBRAPluginPreferencesGeneral (
      const PluginInfo &Info,
      Config &local) :
      QFrame (0),
      QtWidget (Info),
      Plugin (Info),
      _log (Info),
      _defs (Info),
      _layout (0),
      _valueAttrHandle (0),
      _messageTable () {

   _init (local);
}


dmz::MBRAPluginPreferencesGeneral::~MBRAPluginPreferencesGeneral () {

   _messageTable.empty ();
}


// QtWidget Interface
QWidget *
dmz::MBRAPluginPreferencesGeneral::get_qt_widget () {

   return this;
}


// Plugin Interface
void
dmz::MBRAPluginPreferencesGeneral::update_plugin_state (
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
dmz::MBRAPluginPreferencesGeneral::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


void
dmz::MBRAPluginPreferencesGeneral::_slot_scalar_value_changed (double value) {
   
   QDoubleSpinBox *spinBox = qobject_cast<QDoubleSpinBox *> (sender ());
   
   if (spinBox) {
      
      QString name (spinBox->objectName ());
      MessageStruct *ms = _messageTable.lookup (qPrintable (name));
      
      if (ms && ms->message) {
         
         Data data;

         data.store_float64 (_valueAttrHandle, 0, spinBox->value ());

         ms->message.send (ms->targets, &data, 0);
      }
   }
}


void
dmz::MBRAPluginPreferencesGeneral::_get_targets (
      const String &Name,
      Config &config,
      HandleContainer &targets) {

   Config targetList;

   if (config.lookup_all_config (Name, targetList)) {

      ConfigIterator it;
      Config targetConfig;

      while (targetList.get_next_config (it, targetConfig)) {

         const String TargetName (config_to_string ("name", targetConfig));

         if (TargetName) {

            const Handle Target (_defs.create_named_handle (TargetName));
            targets.add_handle (Target);
         }
         else { _log.error << "Unable to add unnamed target" << endl; }
      }
   }
//   else if (_defaultTarget) { targets.add_handle (_defaultTarget); }
}


void
dmz::MBRAPluginPreferencesGeneral::_create_properties (Config &list) {
   
   ConfigIterator it;
   Config property;
   
   RuntimeContext *context (get_plugin_runtime_context ());
   
   while (list.get_prev_config (it, property)) {
      
      const String Type (config_to_string ("type", property));
      const String Name (config_to_string ("name", property) + ":");
      Message message (config_create_message ("message", property, "", context));
      
      if (Type && Name && message) {

         MessageStruct *ms (new MessageStruct);
         
         if (ms) {
            
            if (Type == "scalar") {

               QDoubleSpinBox *scalar (new QDoubleSpinBox);
               qdoublespinbox_config_read ("", property, scalar);
               
               connect (
                  scalar, SIGNAL (valueChanged (double)),
                  this, SLOT (_slot_scalar_value_changed (double)));
               
               ms->widget = scalar;
            }
            else if (Type == "line") {
               
            }
            else if (Type == "text") {
               
            }
            
            if (ms->widget) {
               
               ms->widget->setObjectName (message.get_name ().get_buffer ());
            }
            
            ms->message = message;
            
            _get_targets ("target", property, ms->targets);

            if (_messageTable.store (ms->message.get_name (), ms)) {
            
               QLabel *label (new QLabel (Name.get_buffer ()));
            
               _layout->addRow (label, ms->widget);
            }
            else { delete ms; ms = 0; }
         }
      }
   }
}


void
dmz::MBRAPluginPreferencesGeneral::_init (Config &local) {

   setObjectName (get_plugin_name ().get_buffer ());
   
  RuntimeContext *context (get_plugin_runtime_context ());

   qframe_config_read ("frame", local, this);
   
//   _defaultTarget = config_to_named_handle ("default-target.name", local, context);
   
   _layout = new QFormLayout;
   
   setLayout (_layout);
   
   _valueAttrHandle = config_to_named_handle (
      "attribute.value.name",
      local,
      "value",
      context);
      
   Config list;
   
   if (local.lookup_all_config ("property", list)) {
      
      _create_properties (list);
   }
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginPreferencesGeneral (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginPreferencesGeneral (Info, local);
}

};
