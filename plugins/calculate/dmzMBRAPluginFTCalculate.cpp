#include "dmzMBRAPluginFTCalculate.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginFTCalculate::MBRAPluginFTCalculate (
      const PluginInfo &Info,
      Config &local) :
      QWidget (0),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _channel (0),
      _budgetAttrHandle (0),
      _ecHandle (0),
      _riskSumHandle (0),
      _riskSumReducedHandle (0),
      _vulnerabilitySumHandle (0),
      _vulnerabilitySumReducedHandle (0),
      _calculateOnMessage (),
      _calculateOffMessage (),
      _target (0),
      _title (tr ("Calculations")),
      _dock (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _ui.riskLabel->setText (QString::number (0.0, 'f', 2));
   _ui.riskReducedLabel->setText (QString::number (0.0, 'f', 2));
   _ui.vulnerabilityLabel->setText (QString::number (0.0, 'f', 2) + QString ("%"));
   _ui.vulnerabilityReducedLabel->setText (QString::number (0.0, 'f', 2) + QString ("%"));

   _init (local);
}


dmz::MBRAPluginFTCalculate::~MBRAPluginFTCalculate () {

   _ecTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginFTCalculate::discover_plugin (
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
               Qt::BottomDockWidgetArea,
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
dmz::MBRAPluginFTCalculate::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   Float64 *value (_ecTable.remove (ObjectHandle));

   if (value) {

      delete value; value = 0;
      _update_budget ();
   }
}


void
dmz::MBRAPluginFTCalculate::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   if (AttributeHandle == _ecHandle) {

      Float64 *ptr (_ecTable.lookup (ObjectHandle));

      if (!ptr) {

         ptr = new Float64 (0.0);

         if (ptr && !_ecTable.store (ObjectHandle, ptr)) {

            delete ptr; ptr = 0;
         }
      }

      if (ptr) {

         *ptr = Value;
         _update_budget ();
      }
   }
   else if (AttributeHandle == _riskSumHandle) {

      _ui.riskLabel->setText (QString::number (Value, 'f', 2));
   }
   else if (AttributeHandle == _riskSumReducedHandle) {

      _ui.riskReducedLabel->setText (QString::number (Value, 'f', 2));
   }
   else if (AttributeHandle == _vulnerabilitySumHandle) {

      _ui.vulnerabilityLabel->setText (
         QString::number (Value * 100.0, 'f', 2) + QString ("%"));
   }
   else if (AttributeHandle == _vulnerabilitySumReducedHandle) {

      _ui.vulnerabilityReducedLabel->setText (
         QString::number (Value * 100.0, 'f', 2) + QString ("%"));
   }
}


void
dmz::MBRAPluginFTCalculate::_slot_calculate (bool on) {

   if (on) {

      Data data;
      data.store_float64 (_budgetAttrHandle, 0, _ui.budgetSpinBox->value ());
      _calculateOnMessage.send (_target, &data, 0);
   }
   else {

      Data data;
      _calculateOffMessage.send (_target, &data, 0);
   }
}


void
dmz::MBRAPluginFTCalculate::_slot_update_budget (int budget) {

   if (_ui.calculateButton->isChecked ()) {

      Data data;
      data.store_float64 (_budgetAttrHandle, 0, Float64 (budget));
      _calculateOnMessage.send (_target, &data, 0);
   }
}


void
dmz::MBRAPluginFTCalculate::_update_budget () {

   HashTableHandleIterator it;

   Float64 total (0.0);

   Float64 *value (_ecTable.get_first (it));

   while (value) {

      total += *value;

      value = _ecTable.get_next (it);
   }

   // _log.error << "Total: " << total << endl;

   _ui.budgetSlider->setMaximum (int (total));
   _ui.budgetSpinBox->setMaximum (int (total));
   _ui.maxBudgetLabel->setNum (int (total));
}


void
dmz::MBRAPluginFTCalculate::_init (Config &local) {

   Definitions defs (get_plugin_runtime_context ());

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _channel = config_to_named_handle (
      "channel.name",
      local,
      "FaultTreeChannel",
      get_plugin_runtime_context ());

   _budgetAttrHandle = config_to_named_handle (
      "attribute.budget.name",
      local,
      "FTBudget",
      get_plugin_runtime_context ());

   _calculateOnMessage = config_create_message_type (
      "message.on",
      local,
      "FTStartWorkMessage",
      get_plugin_runtime_context (),
      &_log);

   _calculateOffMessage = config_create_message_type (
      "message.off",
      local,
      "FTStopWorkMessage",
      get_plugin_runtime_context (),
      &_log);

   _target = config_to_named_handle (
      "message.target",
      local,
      "dmzMBRAPluginFaultTree",
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

   connect (
      _ui.budgetSpinBox, SIGNAL (valueChanged (int)),
      this, SLOT (_slot_update_budget (int)));

   activate_default_object_attribute (ObjectDestroyMask);

   _ecHandle = activate_object_attribute (
      config_to_string ("elimination_cost.name", local, "FT_Threat_Elimination_Cost"),
      ObjectScalarMask);

   _riskSumHandle = activate_object_attribute (
      config_to_string ("risk_sum.name", local, "FT_Risk_Sum_Value"),
      ObjectScalarMask);

   _riskSumReducedHandle = activate_object_attribute (
      config_to_string ("risk_sum_reduced.name", local, "FT_Risk_Sum_Reduced_Value"),
      ObjectScalarMask);

   _vulnerabilitySumHandle = activate_object_attribute (
      config_to_string ("vulnerability_sum.name", local, "FT_Vulnerability_Sum_Value"),
      ObjectScalarMask);

   _vulnerabilitySumReducedHandle = activate_object_attribute (
      config_to_string (
         "vulnerability_sum_reduced.name",
         local,
         "FT_Vulnerability_Sum_Reduced_Value"),
      ObjectScalarMask);

}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFTCalculate (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFTCalculate (Info, local);
}

};
