#include <dmzQtConfigRead.h>
#include "dmzMBRAPluginPreferencesGeneral.h"
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginPreferencesGeneral::MBRAPluginPreferencesGeneral (
      const PluginInfo &Info,
      Config &local) :
      QFrame (0),
      QtWidget (Info),
      Plugin (Info),
      _log (Info) {

   _ui.setupUi (this);
         
   _init (local);
}


dmz::MBRAPluginPreferencesGeneral::~MBRAPluginPreferencesGeneral () {

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
dmz::MBRAPluginPreferencesGeneral::_init (Config &local) {

   setObjectName (get_plugin_name ().get_buffer ());

   qframe_config_read ("frame", local, this);
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
