#include "dmzMBRAPluginCalculate.h"
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginCalculate::MBRAPluginCalculate (const PluginInfo &Info, Config &local) :
      QWidget (0),
      Plugin (Info),
      MessageObserver (Info),
      QtWidget (Info),
      _log (Info),
      _convert (Info),
      _convertString (Info) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _init (local);
}


dmz::MBRAPluginCalculate::~MBRAPluginCalculate () {

}


// Plugin Interface
void
dmz::MBRAPluginCalculate::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


void
dmz::MBRAPluginCalculate::receive_message (
      const Message &Type,
      const Handle MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (Type == _simulationErrorMessage) {

      _ui.errorLabel->setText (
         QString ("<font color='red'>") + to_qstring (_convertString.to_string (InData))
         + QString ("</font>"));
   }
}


// QtWidget Interface
QWidget *
dmz::MBRAPluginCalculate::get_qt_widget () { return this; }


void
dmz::MBRAPluginCalculate::_slot_calculate (bool on) {

   Data data = _convert.to_data (on ? True : False);
   _ui.errorLabel->hide ();
   _simulatorMessage.send (&data);
}


void
dmz::MBRAPluginCalculate::_simulation_type_change (bool state) {

   Data data;
   data = _convert.to_data (state);

   _ui.errorLabel->hide ();
   _simulationTypeMessage.send (&data);
}


void
dmz::MBRAPluginCalculate::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _simulatorMessage = config_create_message (
      "simulator-message.name",
      local,
      "NASimulatorMessage",
      get_plugin_runtime_context (),
      &_log);

   _simulationTypeMessage = config_create_message (
      "simulation-type-message.name",
      local,
      "CalcSimulationType",
      get_plugin_runtime_context (),
      &_log);

   _simulationErrorMessage = config_create_message (
      "simulation-error-message.name",
      local,
      "SimulationErrorMessage",
      get_plugin_runtime_context (),
      &_log);

   subscribe_to_message (_simulationErrorMessage);

   _ui.errorLabel->hide ();

   qwidget_config_read ("widget", local, this);

   qtoolbutton_config_read ("caclulateButton", local, _ui.calculateButton);

   _ui.calculateButton->setStatusTip (_ui.infoLabel->text ());

   QAction *action (_ui.calculateButton->defaultAction ());

   if (action) {

      connect (
         action, SIGNAL (toggled (bool)),
         this, SLOT (_slot_calculate (bool)));
   }

   connect (
      _ui.cascadeButton,
      SIGNAL (toggled (bool)),
      this,
      SLOT (_simulation_type_change (bool)));

}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginCalculate (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginCalculate (Info, local);
}

};
