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
         public MessageObserver,
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

         // Message Observer Interface
         virtual void receive_message (
            const Message &Type,
            const Handle MessageSendHandle,
            const Handle TargetObserverHandle,
            const Data *InData,
            Data *outData);

      protected slots:
         void _slot_delay (int delay);
         void _slot_calculate (bool On);
         void _slot_direction (int index);
         void _slot_links_allow_change (int state);
         void _simulation_type_change (bool state);


      protected:
         void _init (Config &local);

         Log _log;
         DataConverterBoolean _convertBool;
         DataConverterString _convertString;
         DataConverterFloat64 _convertFloat;
         Message _simulateMessage;
         Message _simulateDirectionMessage;
         Message _updateIterationsMessage;
         Message _updateDelayMessage;
         Message _simulateAllowLinksMessage;
         Message _simulationTypeMessage;
         Ui::simulateForm _ui;

      private:
         MBRAPluginNASimulate ();
         MBRAPluginNASimulate (const MBRAPluginNASimulate &);
         MBRAPluginNASimulate &operator= (const MBRAPluginNASimulate &);

   };
};

#endif // DMZ_MBRA_PLUGIN_NA_SIMULATE_DOT_H
