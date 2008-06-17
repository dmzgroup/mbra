#ifndef DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
#define DMZ_MBRA_PLUGIN_CALCULATE_DOT_H

#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QWidget>
#include "ui_dmzMBRACalculateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class QtModuleMainWindow;


   class MBRAPluginCalculate : public QWidget, public Plugin {

      Q_OBJECT

      public:
         MBRAPluginCalculate (const PluginInfo &Info, Config &local);
         ~MBRAPluginCalculate ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

      protected slots:
         void _slot_calculate (bool On);

      protected:
         void _init (Config &local);

         Log _log;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Handle _channel;
         Message _calculateOnMessage;
         Message _calculateOffMessage;
         Handle _target;
         QString _title;
         QDockWidget *_dock;
         Ui::calculateForm _ui;

      private:
         MBRAPluginCalculate ();
         MBRAPluginCalculate (const MBRAPluginCalculate &);
         MBRAPluginCalculate &operator= (const MBRAPluginCalculate &);

   };
};

#endif // DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
