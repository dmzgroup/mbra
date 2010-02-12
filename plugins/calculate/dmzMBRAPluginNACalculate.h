#ifndef DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H
#define DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeDataConverterTypesBase.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHandleContainer.h>
#include <QtGui/QFrame>
#include <QtGui/QButtonGroup>
#include "ui_NACalculateForm.h"


namespace dmz {

   class MBRAPluginNACalculate :
         public QFrame,
         public QtWidget,
         public Plugin,
         public MessageObserver,
         public ObjectObserverUtil {

      Q_OBJECT
      
      public:
         MBRAPluginNACalculate (const PluginInfo &Info, Config &local);
         ~MBRAPluginNACalculate ();

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();
         
         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Message Observer Interface
         virtual void receive_message (
            const Message &Type,
            const Handle MessageSendHandle,
            const Handle TargetObserverHandle,
            const Data *InData,
            Data *outData);

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
         DataConverterFloat64 _convert;
         Message _updateObjectiveMsg;
         Ui::NACalculateForm _ui;
         Handle _simulatorHandle;
         ObjectType _simulatorType;
         HandleContainer _weightByHandles;
         QButtonGroup _weightByGroup;
         QList<Handle> _objectiveFunctionHandles;
         Boolean _ignoreUpdates;

      private:
         MBRAPluginNACalculate ();
         MBRAPluginNACalculate (const MBRAPluginNACalculate &);
         MBRAPluginNACalculate &operator= (const MBRAPluginNACalculate &);
   };
};

#endif // DMZ_MBRA_PLUGIN_NA_SIMULATOR_DOT_H
