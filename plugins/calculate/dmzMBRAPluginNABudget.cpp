#include "dmzMBRAPluginNABudget.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>

dmz::MBRAPluginNABudget::MBRAPluginNABudget (const PluginInfo &Info, Config &local) :
      QtWidget (Info),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _pcAttrHandle (0),
      _rcAttrHandle (0),
      _maxBudget (0.0) {

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

   if (Type.is_of_type (_nodeType)) {

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

      _update_max_budget ();
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
      }
      else if (AttributeHandle == _rcAttrHandle) {

         os->rc = 0.0;
      }

      _update_max_budget ();
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
      }
      else if (AttributeHandle == _rcAttrHandle) {

         os->rc = Value;
      }

      _update_max_budget ();
   }
}


void
dmz::MBRAPluginNABudget::_update_max_budget () {

   _maxBudget = 0.0;

   HashTableHandleIterator it;
   ObjectStruct *os (0);

   while (_objectTable.get_next (it, os)) {

      _maxBudget += os->pc;
      _maxBudget += os->rc;
   }

   _ui.budgetSlider->setMaximum (int (_maxBudget));
   _ui.budgetBox->setMaximum (int (_maxBudget));
   _ui.maxBudgetLabel->setText (QString ("$ ") + QString::number (int (_maxBudget)));
}


void
dmz::MBRAPluginNABudget::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   _nodeType = config_to_object_type ("node-type.name", local, "na_node", context);

   activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);

   _pcAttrHandle = activate_object_attribute (
      config_to_string ("prevention-cost.name", local, "NA_Node_Prevention_Cost"),
      ObjectRemoveAttributeMask | ObjectScalarMask);

   _rcAttrHandle = activate_object_attribute (
      config_to_string ("response-cost.name", local, "NA_Node_Response_Cost"),
      ObjectRemoveAttributeMask | ObjectScalarMask);
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
