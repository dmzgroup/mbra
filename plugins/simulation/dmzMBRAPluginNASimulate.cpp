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
      QtWidget (Info),
      _log (Info),
      _convert (Info) {

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


void
dmz::MBRAPluginNASimulate::_slot_calculate (bool on) {

   Data data = _convert.to_data (on ? True : False);
   _simulatorMessage.send (&data);
}


void
dmz::MBRAPluginNASimulate::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _simulatorMessage = config_create_message (
      "simulator-message.name",
      local,
      "NASimulateMessage",
      get_plugin_runtime_context (),
      &_log);

   qwidget_config_read ("widget", local, this);

   qtoolbutton_config_read ("caclulateButton", local, _ui.simulateButton);

   _ui.simulateButton->setStatusTip (_ui.infoLabel->text ());

   QAction *action (_ui.simulateButton->defaultAction ());

   if (action) {

      connect (
         action, SIGNAL (toggled (bool)),
         this, SLOT (_slot_calculate (bool)));
   }

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
