#include "dmzMBRAPluginCalculate.h"
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginCalculate::MBRAPluginCalculate (const PluginInfo &Info, Config &local) :
      QWidget (0),
      Plugin (Info),
      _log (Info),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _channel (0),
      _calculateOnMessage (),
      _calculateOffMessage (),
      _target (0),
      _title (tr ("Calculations")),
      _dock (0) {

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

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule) {

            _dock = new QDockWidget (_title, this);
            _dock->setObjectName (get_plugin_name ().get_buffer ());
            _dock->setAllowedAreas (Qt::AllDockWidgetAreas);

            _dock->setFeatures (QDockWidget::NoDockWidgetFeatures);
   //            QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

            _mainWindowModule->add_dock_widget (
               _channel,
               Qt::RightDockWidgetArea,
               _dock);

            _dock->setWidget (this);
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_mainWindowModule &&
            (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {

         _mainWindowModule->remove_dock_widget (_channel, _dock);
         _mainWindowModule = 0;
      }
   }
}


void
dmz::MBRAPluginCalculate::_slot_calculate (bool on) {

   if (on) {

      Data data;
      _calculateOnMessage.send (_target, &data, 0);
   }
   else {

      Data data;
      _calculateOffMessage.send (_target, &data, 0);
   }
}


void
dmz::MBRAPluginCalculate::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _channel = config_to_named_handle (
      "channel.name",
      local,
      "NetworkAnalysisChannel",
      get_plugin_runtime_context ());

   _calculateOnMessage = config_create_message_type (
      "message.on",
      local,
      "NARankObjectsMessage",
      get_plugin_runtime_context (),
      &_log);

   _calculateOffMessage = config_create_message_type (
      "message.off",
      local,
      "NAHideObjectsMessage",
      get_plugin_runtime_context (),
      &_log);

   _target = config_to_named_handle (
      "message.target",
      local,
      "dmzMBRAPluginNARanking",
      get_plugin_runtime_context ());

   _title = config_to_string (
      "title",
      local,
      qPrintable (_title)).get_buffer ();

   qwidget_config_read ("widget", local, this);

   qtoolbutton_config_read ("caclulateButton", local, _ui.calculateButton);

   _ui.calculateButton->setStatusTip (_ui.infoLabel->text ());

   QAction *action (_ui.calculateButton->defaultAction ());

   if (action) {

      connect (
         action, SIGNAL (toggled (bool)),
         this, SLOT (_slot_calculate (bool)));
   }
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
