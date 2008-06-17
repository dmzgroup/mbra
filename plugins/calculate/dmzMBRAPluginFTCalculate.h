#ifndef DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H
#define DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <QtGui/QWidget>
#include "ui_dmzMBRAFTCalculateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class QtModuleMainWindow;


   class MBRAPluginFTCalculate :
      public QWidget,
      public Plugin,
      public ObjectObserverUtil {

      Q_OBJECT

      public:
         MBRAPluginFTCalculate (const PluginInfo &Info, Config &local);
         ~MBRAPluginFTCalculate ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // ObjectObserverUtil Interface
         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void update_object_scalar (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Float64 Value,
            const Float64 *PreviousValue);

      protected slots:
         void _slot_calculate (bool On);
         void _slot_update_budget (int budget);

      protected:
         void _update_budget ();
         void _init (Config &local);

         Log _log;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Handle _channel;
         Handle _budgetAttrHandle;
         Handle _ecHandle;
         Handle _riskSumHandle;
         Handle _riskSumReducedHandle;
         Handle _vulnerabilitySumHandle;
         Handle _vulnerabilitySumReducedHandle;
         Message _calculateOnMessage;
         Message _calculateOffMessage;
         Handle _target;
         QString _title;
         QDockWidget *_dock;
         Ui::calculateForm _ui;
         HashTableHandleTemplate<Float64> _ecTable;

      private:
         MBRAPluginFTCalculate ();
         MBRAPluginFTCalculate (const MBRAPluginFTCalculate &);
         MBRAPluginFTCalculate &operator= (const MBRAPluginFTCalculate &);
   };
};

#endif // DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H
