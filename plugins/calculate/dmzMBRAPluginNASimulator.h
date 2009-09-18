#ifndef DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H
#define DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHandleContainer.h>
#include <QtGui/QFrame>
#include <QtGui/QButtonGroup>
#include "ui_NASimulatorForm.h"


namespace dmz {

   class MBRAPluginNASimulator :
         public QFrame,
         public QtWidget,
         public Plugin,
         public ObjectObserverUtil {

      Q_OBJECT
      
      public:
         MBRAPluginNASimulator (const PluginInfo &Info, Config &local);
         ~MBRAPluginNASimulator ();

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();
         
         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Object Observer Interface
         virtual void update_object_flag (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Boolean Value,
            const Boolean *PreviousValue);

      protected slots:
         void _slot_weight_by_clicked (int id);
         void on_objectiveComboBox_currentIndexChanged (int id);
         
      protected:
         void _init (Config &local);

         Log _log;
         Ui::NASimulatorForm _ui;
         Handle _simulatorHandle;
         ObjectType _simulatorType;
         HandleContainer _weightByHandles;
         QButtonGroup _weightByGroup;
         QList<Handle> _objectiveFunctionHandles;
         Boolean _ignoreUpdates;

      private:
         MBRAPluginNASimulator ();
         MBRAPluginNASimulator (const MBRAPluginNASimulator &);
         MBRAPluginNASimulator &operator= (const MBRAPluginNASimulator &);
   };
};

#endif // DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H
