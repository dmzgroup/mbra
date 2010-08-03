#include "dmzMBRAPluginNASimulate.h"
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginNASimulate::MBRAPluginNASimulate (const PluginInfo &Info, Config &local) :
      QFrame (0),
      Plugin (Info),
      MessageObserver (Info),
      QtWidget (Info),
      _log (Info),
      _convertBool (Info),
      _convertString (Info),
      _convertFloat (Info) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _init (local);
}


dmz::MBRAPluginNASimulate::~MBRAPluginNASimulate () {

}


// Plugin Interface
void
dmz::MBRAPluginNASimulate::update_plugin_state (
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
dmz::MBRAPluginNASimulate::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}

// QtWidget Interface
QWidget *
dmz::MBRAPluginNASimulate::get_qt_widget () { return this; }


// Message Observer Interface
void
dmz::MBRAPluginNASimulate::receive_message (
   const Message &Type,
   const Handle MessageSendHandle,
   const Handle TargetObserverHandle,
   const Data *InData,
   Data *outData) {

   Float64 val = _convertFloat.to_float64 (InData, 0);
   _ui.iterationTotal->display(val);
}

void
dmz::MBRAPluginNASimulate::_slot_delay (int delay) {

   Data data = _convertFloat.to_data (delay);

   _updateDelayMessage.send (&data);
}

void
dmz::MBRAPluginNASimulate::_slot_calculate (bool on) {

   Data data = _convertBool.to_data (on ? True : False);
   _simulateMessage.send (&data);
}

void
dmz::MBRAPluginNASimulate::_slot_direction (int index) {

   String val;

   if (index == 0) { val = "Bidirectional"; }
   if (index == 1) { val = "Upstream"; }
   if (index == 2) { val = "Downstream"; }
   Data data = _convertString.to_data (val);
   _simulateDirectionMessage.send (&data);

}

void
dmz::MBRAPluginNASimulate::_slot_links_allow_change (int state) {

   Data data;
   if (state == Qt::Unchecked) {
      data = _convertBool.to_data (False);
   }
   else if (state == Qt::Checked) {
      data = _convertBool.to_data (True);
   }

   _simulateAllowLinksMessage.send (&data);

}

void
dmz::MBRAPluginNASimulate::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _simulateMessage = config_create_message (
      "simulator-message.name",
      local,
      "NASimulateMessage",
      get_plugin_runtime_context (),
      &_log);

   _simulateDirectionMessage = config_create_message (
         "simulate-direction-message.name",
         local,
         "NASimulateDirectionMessage",
         get_plugin_runtime_context (),
         &_log);

   _updateDelayMessage = config_create_message (
         "simulate-delay-message.name",
         local,
         "NASimulateDelayMessage",
         get_plugin_runtime_context (),
         &_log);

   _updateIterationsMessage = config_create_message (
         "simulate-itercount-message.name",
         local,
         "NASimulateIterCountMessage",
         get_plugin_runtime_context (),
         &_log);

   _simulateAllowLinksMessage = config_create_message (
         "simulate-allow-links-message.name",
         local,
         "NASimulateLinksMessage",
         get_plugin_runtime_context (),
         &_log);

   subscribe_to_message (_updateIterationsMessage);

   connect (
         _ui.updateDelayBox, SIGNAL (valueChanged (int)),
         this, SLOT (_slot_delay (int)));

   qwidget_config_read ("widget", local, this);

   qtoolbutton_config_read ("caclulateButton", local, _ui.simulateButton);

   _ui.simulateButton->setStatusTip (_ui.infoLabel->text ());

   QAction *action (_ui.simulateButton->defaultAction ());

   if (action) {

      connect (
         action, SIGNAL (toggled (bool)),
         this, SLOT (_slot_calculate (bool)));

   }

   QStringList failDirections;
   failDirections << "Bidirectional" << "Upstream" << "Downstream";

   _ui.failDirectionBox->insertItems (0, failDirections);

   connect (
         _ui.failDirectionBox, SIGNAL (currentIndexChanged (int)),
         this, SLOT (_slot_direction (int)));

   connect (
         _ui.allowLinkFail, SIGNAL (stateChanged (int)),
         this, SLOT (_slot_links_allow_change (int)));

}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginNASimulate (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginNASimulate (Info, local);
}

};
