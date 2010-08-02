#ifndef DMZ_MBRA_PLUGIN_NA_SIMULATE_DOT_H
#define DMZ_MBRA_PLUGIN_NA_SIMULATE_DOT_H

#include <dmzQtWidget.h>
#include <dmzRuntimeDataConverterTypesBase.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QComboBox>
#include <QtGui/QWidget>
#include "ui_dmzMBRAPluginNASimulateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class MBRAPluginNASimulate :
         public QFrame,
         public Plugin,
         public QtWidget {

      Q_OBJECT

      public:
         MBRAPluginNASimulate (const PluginInfo &Info, Config &local);
         ~MBRAPluginNASimulate ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();

      protected slots:
         void _slot_calculate (bool On);
         void _slot_direction (int index);

      protected:
         void _init (Config &local);

         Log _log;
         DataConverterBoolean _convertBool;
         DataConverterString _convertString;
         Message _simulateMessage;
         Message _simulateDirectionMessage;
         Ui::simulateForm _ui;

      private:
         MBRAPluginNASimulate ();
         MBRAPluginNASimulate (const MBRAPluginNASimulate &);
         MBRAPluginNASimulate &operator= (const MBRAPluginNASimulate &);

   };
};

#endif // DMZ_MBRA_PLUGIN_NA_SIMULATE_DOT_H
