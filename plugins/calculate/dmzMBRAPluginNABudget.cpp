#include "dmzMBRAPluginNABudget.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>

dmz::MBRAPluginNABudget::MBRAPluginNABudget (const PluginInfo &Info, Config &local) :
      QtWidget (Info),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _budgetHandle (0),
      _pcAttrHandle (0),
      _rcAttrHandle (0),
      _maxPreventionBudget (0.0),
      _maxResponseBudget (0.0),
      _lastResponseBudget (0.0),
      _lastPreventionBudget (0.0) {

   _ui.setupUi (this);

   _init (local);
}


dmz::MBRAPluginNABudget::~MBRAPluginNABudget () {

}


// QtWidget Interface
QWidget *   
dmz::MBRAPluginNABudget::get_qt_widget () { return this; }


// Plugin Interface
void
dmz::MBRAPluginNABudget::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginNABudget::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}


// Object Observer Interface
void
dmz::MBRAPluginNABudget::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_type (_nodeType) || Type.is_of_type (_linkType)) {

      ObjectStruct *os = new ObjectStruct;

      if (!_objectTable.store (ObjectHandle, os)) { delete os; os = 0; }
   }
}


void
dmz::MBRAPluginNABudget::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   ObjectStruct *os = _objectTable.remove (ObjectHandle);

   if (os) {

      delete os; os = 0;

      _update_max_prevention_budget ();
      _update_max_response_budget ();
   }
}


void
dmz::MBRAPluginNABudget::remove_object_attribute (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &AttrMask) {

   ObjectStruct *os = _objectTable.lookup (ObjectHandle);

   if (os) {

      if (AttributeHandle == _pcAttrHandle) {

         os->pc = 0.0;
         _update_max_prevention_budget ();
      }
      else if (AttributeHandle == _rcAttrHandle) {

         os->rc = 0.0;
         _update_max_response_budget ();
      }
   }
}


void
dmz::MBRAPluginNABudget::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   ObjectStruct *os = _objectTable.lookup (ObjectHandle);

   if (os) {

      if (AttributeHandle == _pcAttrHandle) {

         os->pc = Value;
         _update_max_prevention_budget ();
      }
      else if (AttributeHandle == _rcAttrHandle) {

         os->rc = Value;
         _update_max_response_budget ();
      }
   }
}


void
dmz::MBRAPluginNABudget::on_preventionBudgetBox_valueChanged (int value) {

   _lastPreventionBudget = (Float64)value;

   if (!_ui.preventionBudgetSlider->isSliderDown ()) {

      Data data;
      data.store_float64 (_budgetHandle, 0, _lastPreventionBudget);
      data.store_float64 (_budgetHandle, 1, _maxPreventionBudget);

      _preventionBudgetMessage.send (&data);
   }
}


void
dmz::MBRAPluginNABudget::on_responseBudgetBox_valueChanged (int value) {

   _lastResponseBudget = (Float64)value;
   if (!_ui.responseBudgetSlider->isSliderDown ()) {

      Data data;
      data.store_float64 (_budgetHandle, 0, _lastResponseBudget);
      data.store_float64 (_budgetHandle, 1, _maxResponseBudget);

      _responseBudgetMessage.send (&data);
   }
}


void
dmz::MBRAPluginNABudget::_update_max_prevention_budget () {

   _maxPreventionBudget = 0.0;

   HashTableHandleIterator it;
   ObjectStruct *os (0);

   while (_objectTable.get_next (it, os)) { _maxPreventionBudget += os->pc; }

   _ui.preventionBudgetSlider->setMaximum (int (_maxPreventionBudget));
   _ui.preventionBudgetBox->setMaximum (int (_maxPreventionBudget));
   _ui.maxPreventionBudgetLabel->setText (
      QString ("$") + QString::number (int (_maxPreventionBudget)));

   on_preventionBudgetBox_valueChanged (_ui.preventionBudgetBox->value ());
}


void
dmz::MBRAPluginNABudget::_responseSliderReleased () {

   Data data;
   data.store_float64 (_budgetHandle, 0, _lastResponseBudget);
   data.store_float64 (_budgetHandle, 1, _maxResponseBudget);

   _responseBudgetMessage.send (&data);
}


void
dmz::MBRAPluginNABudget::_preventionSliderReleased () {

   Data data;
   data.store_float64 (_budgetHandle, 0, _lastPreventionBudget);
   data.store_float64 (_budgetHandle, 1, _maxPreventionBudget);

   _preventionBudgetMessage.send (&data);
}


void
dmz::MBRAPluginNABudget::_update_max_response_budget () {

   _maxResponseBudget = 0.0;

   HashTableHandleIterator it;
   ObjectStruct *os (0);

   while (_objectTable.get_next (it, os)) { _maxResponseBudget += os->rc; }

   _ui.responseBudgetSlider->setMaximum (int (_maxResponseBudget));
   _ui.responseBudgetBox->setMaximum (int (_maxResponseBudget));
   _ui.maxResponseBudgetLabel->setText (
      QString ("$") + QString::number (int (_maxResponseBudget)));

   on_responseBudgetBox_valueChanged (_ui.responseBudgetBox->value ());
}


void
dmz::MBRAPluginNABudget::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   _nodeType = config_to_object_type ("node-type.name", local, "na_node", context);

   _linkType = config_to_object_type (
      "link-attribute-type.name",
       local,
       "na_link_attribute",
       context);

   _preventionBudgetMessage = config_create_message (
      "prevention-budget-message.name",
      local,
      "PreventionBudgetMessage",
      context,
      &_log);

   _responseBudgetMessage = config_create_message (
      "response-budget-message.name",
      local,
      "ResponseBudgetMessage",
      context,
      &_log);

   _budgetHandle = config_to_named_handle (
      "budget.name",
      local,
      "Budget",
      context);

   activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);

   _pcAttrHandle = activate_object_attribute (
      config_to_string ("prevention-cost.name", local, "NA_Node_Prevention_Cost"),
      ObjectRemoveAttributeMask | ObjectScalarMask);

   _rcAttrHandle = activate_object_attribute (
      config_to_string ("response-cost.name", local, "NA_Node_Response_Cost"),
      ObjectRemoveAttributeMask | ObjectScalarMask);

   connect (
      _ui.responseBudgetSlider,
      SIGNAL (sliderReleased ()),
      this,
      SLOT (_responseSliderReleased ()));

   connect (
      _ui.preventionBudgetSlider,
      SIGNAL (sliderReleased ()),
      this,
      SLOT (_preventionSliderReleased ()));
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginNABudget (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginNABudget (Info, local);
}

};
