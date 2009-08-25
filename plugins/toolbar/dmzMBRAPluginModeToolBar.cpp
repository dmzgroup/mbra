#include <dmzArchiveModule.h>
#include <dmzInputConsts.h>
#include <dmzInputModule.h>
#include <dmzInputEventMasks.h>
#include "dmzMBRAPluginModeToolBar.h"
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>

dmz::MBRAPluginModeToolBar::MBRAPluginModeToolBar (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      ArchiveObserverUtil (Info, local),
      InputObserverUtil (Info, local),
      _log (Info),
      _inputModule (0),
      _networkAnalysisChannel (0),
      _faultTreeChannel (0),
      _currentChannel (0) {

   _init (local);
}


dmz::MBRAPluginModeToolBar::~MBRAPluginModeToolBar () {

}


// Plugin Interface
void
dmz::MBRAPluginModeToolBar::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateStart) {

   }
}


void
dmz::MBRAPluginModeToolBar::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_inputModule) {

         _inputModule = InputModule::cast (PluginPtr, _inputModuleName);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_inputModule && (_inputModule == InputModule::cast (PluginPtr))) {

         _inputModule = 0;
      }
   }
}


// ArchiveObserver Interface.
void
dmz::MBRAPluginModeToolBar::create_archive (
      const Handle ArchiveHandle,
      const Int32 Version,
      Config &local,
      Config &global) {

   if (is_active_archive_handle (ArchiveHandle)) {

      if (_currentChannel) {

         Definitions defs (get_plugin_runtime_context (), &_log);

         local.store_attribute (
            "channel",
            defs.lookup_named_handle_name (_currentChannel));
      }
   }
}


void
dmz::MBRAPluginModeToolBar::process_archive (
      const Handle ArchiveHandle,
      const Int32 Version,
      Config &local,
      Config &global) {

   if (is_active_archive_handle (ArchiveHandle)) {

      Definitions defs (get_plugin_runtime_context (), &_log);

      Handle channel (defs.lookup_named_handle (config_to_string ("channel", local)));

      if (channel && _inputModule) {

         _inputModule->set_channel_state (
            channel == _networkAnalysisChannel ?
               _faultTreeChannel : _networkAnalysisChannel, False);

         _inputModule->set_channel_state (channel, True);
      }
   }
}


// InputObserverUtil Interface
void
dmz::MBRAPluginModeToolBar::update_channel_state (
      const Handle Channel,
      const Boolean State) {

   if (State) { _currentChannel = Channel; }
}


void
dmz::MBRAPluginModeToolBar::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   init_archive (local);

   _networkAnalysisChannel = defs.create_named_handle (
      config_to_string ("networkAnalysis.channel", local, "NetworkAnalysisChannel"));

   _faultTreeChannel = defs.create_named_handle (
      config_to_string ("faultTree.channel", local, "FaultTreeChannel"));

   activate_input_channel (_networkAnalysisChannel, InputEventChannelStateMask);
   activate_input_channel (_faultTreeChannel, InputEventChannelStateMask);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginModeToolBar (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginModeToolBar (Info, local);
}

};
