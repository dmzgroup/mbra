#include <dmzArchiveModule.h>
#include <dmzInputConsts.h>
#include <dmzInputModule.h>
#include "dmzMBRAPluginModeToolBar.h"
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginModeToolBar::MBRAPluginModeToolBar (
      const PluginInfo &Info,
      Config &local) :
      QWidget (0),
      Plugin (Info),
      ArchiveObserverUtil (Info, local),
      _log (Info),
      _inputModule (0),
      _inputModuleName (),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _toolBar (0),
      _networkAnalysisChannel (0),
      _faultTreeChannel (0),
      _currentChannel (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

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

      if (!_currentChannel) {

         _ui.networkButton->click ();
      }
   }
}


void
dmz::MBRAPluginModeToolBar::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_inputModule) {

         _inputModule = InputModule::cast (PluginPtr, _inputModuleName);

         if (_inputModule) {

            _inputModule->create_channel (_networkAnalysisChannel);
            _inputModule->create_channel (_faultTreeChannel);

            _inputModule->set_channel_state (_faultTreeChannel, False);
            _inputModule->set_channel_state (_networkAnalysisChannel, False);
         }
      }

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule && _toolBar) {

            _mainWindowModule->add_tool_bar (_toolBar);
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_inputModule && (_inputModule == InputModule::cast (PluginPtr))) {

         _inputModule = 0;
      }

      if (_mainWindowModule &&
            (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {

         _mainWindowModule->remove_tool_bar (_toolBar);
         _toolBar->setParent (0);
         _mainWindowModule = 0;
      }
   }
}


// ArchiveObserver Interface.
void
dmz::MBRAPluginModeToolBar::create_archive (
      const Handle ArchiveHandle,
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
      Config &local,
      Config &global) {

   if (is_active_archive_handle (ArchiveHandle)) {

      Definitions defs (get_plugin_runtime_context (), &_log);

      Handle channel (defs.lookup_named_handle (config_to_string ("channel", local)));

      if (channel) {

         if (channel == _faultTreeChannel) { _ui.faultTreeButton->click (); }
         else { _ui.networkButton->click (); }
      }
   }
}


void
dmz::MBRAPluginModeToolBar::_slot_network_analysis () {

   if (_inputModule) {

      _inputModule->set_channel_state (_currentChannel, False);
      _inputModule->set_channel_state (_networkAnalysisChannel, True);
      _currentChannel = _networkAnalysisChannel;
   }
}


void
dmz::MBRAPluginModeToolBar::_slot_fault_tree () {

   if (_inputModule) {

      _inputModule->set_channel_state (_currentChannel, False);
      _inputModule->set_channel_state (_faultTreeChannel, True);
      _currentChannel = _faultTreeChannel;
   }
}


void
dmz::MBRAPluginModeToolBar::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   init_archive (local);

   _networkAnalysisChannel = defs.create_named_handle (
      config_to_string ("networkAnalysis.channel", local, "NetworkAnalysisChannel"));

   _faultTreeChannel = defs.create_named_handle (
      config_to_string ("faultTree.channel", local, "FaultTreeChannel"));

   _toolBar = new QToolBar ("Mode", 0);
   _toolBar->setObjectName (get_plugin_name ().get_buffer ());

   qtoolbar_config_read ("toolBar", local, _toolBar);

   _toolBar->addWidget (this);

   qpushbutton_config_read ("networkAnalysis", local, _ui.networkButton);

   connect (
      _ui.networkButton, SIGNAL (clicked ()),
      this, SLOT (_slot_network_analysis ()));

   qpushbutton_config_read ("faultTree", local, _ui.faultTreeButton);

   connect (
      _ui.faultTreeButton, SIGNAL (clicked ()),
      this, SLOT (_slot_fault_tree ()));
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
