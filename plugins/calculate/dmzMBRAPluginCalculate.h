#ifndef DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
#define DMZ_MBRA_PLUGIN_CALCULATE_DOT_H

#include <dmzQtWidget.h>
#include <dmzRuntimeDataConverterTypesBase.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QWidget>
#include "ui_dmzMBRACalculateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class MBRAPluginCalculate : public QWidget, public Plugin, public QtWidget {

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

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();

      protected slots:
         void _slot_calculate (bool On);
         void _simulation_type_change (bool state);

      protected:
         void _init (Config &local);

         Log _log;
         DataConverterBoolean _convert;
         Message _simulatorMessage;
         Message _simulationTypeMessage;
         Ui::calculateForm _ui;

      private:
         MBRAPluginCalculate ();
         MBRAPluginCalculate (const MBRAPluginCalculate &);
         MBRAPluginCalculate &operator= (const MBRAPluginCalculate &);

   };
};

#endif // DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
