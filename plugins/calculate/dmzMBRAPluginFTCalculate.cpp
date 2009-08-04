#include "dmzMBRAPluginFTCalculate.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
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
      QtWidget (Info),
      _log (Info),
      _undo (Info),
      _defaultAttrHandle (0),
      _channel (0),
      _budgetAttrHandle (0),
      _ecHandle (0),
      _riskSumHandle (0),
      _riskSumReducedHandle (0),
      _vulnerabilitySumHandle (0),
      _vulnerabilitySumReducedHandle (0),
      _nameAttrHandle (0),
      _hideAttrHandle (0),
      _activeAttrHandle (0),
      _root (0),
      _target (0),
      _editTarget (0),
      _objectDataHandle (0),
      _createdDataHandle (0) {

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

   }
   else if (Mode == PluginDiscoverRemove) {

   }
}

// ObjectObserverUtil Interface
void
dmz::MBRAPluginFTCalculate::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   int index = _ui.rootBox->findData (QVariant (qlonglong (ObjectHandle)));

   if ((index < 0) && Type.is_of_exact_type (_rootType)) {

      _ui.rootBox->addItem ("", QVariant (qlonglong (ObjectHandle)));
   }
}


void
dmz::MBRAPluginFTCalculate::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   EcStruct *ecs (_ecTable.remove (ObjectHandle));

   if (ecs) {

      delete ecs; ecs = 0;
      _update_budget ();
   }

   int index = _ui.rootBox->findData (QVariant (qlonglong (ObjectHandle)));

   if (index >= 0) { _ui.rootBox->removeItem (index); }
}


void
dmz::MBRAPluginFTCalculate::remove_object_attribute (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &AttrMask) {

   if (AttributeHandle == _hideAttrHandle) {

      EcStruct *ptr = _ecTable.lookup (ObjectHandle);

      if (ptr && AttrMask.contains (ObjectFlagMask)) {

         ptr->hide = False;
         _update_budget ();
      }
   }
}


void
dmz::MBRAPluginFTCalculate::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

   if (AttributeHandle == _hideAttrHandle) {

      EcStruct *ptr = _ecTable.lookup (ObjectHandle);

      if (ptr) {

         ptr->hide = Value;
         _update_budget ();
      }
   }
   else if (AttributeHandle == _activeAttrHandle) {

      if (Value) {

         _root = ObjectHandle;

         int index = _ui.rootBox->findData (QVariant (qlonglong (ObjectHandle)));
         if (index >= 0) { _ui.rootBox->setCurrentIndex (index); }

         ObjectModule *objMod (get_object_module ());

         if (objMod) {

            Float64 scalar (0.0);
      
            if (objMod->lookup_scalar (_root, _riskSumHandle, scalar)) {

               _ui.riskLabel->setText (QString::number (scalar, 'f', 2));
            }

            if (objMod->lookup_scalar (_root, _riskSumReducedHandle, scalar)) {

               _ui.riskReducedLabel->setText (QString::number (scalar, 'f', 2));
            }

            if (objMod->lookup_scalar (_root, _vulnerabilitySumHandle, scalar)) {

               _ui.vulnerabilityLabel->setText (
                  QString::number (scalar * 100.0, 'f', 2) + QString ("%"));
            }

            if (objMod->lookup_scalar (_root, _vulnerabilitySumReducedHandle, scalar)) {

               _ui.vulnerabilityReducedLabel->setText (
                  QString::number (scalar * 100.0, 'f', 2) + QString ("%"));
            }
         }
      }
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

      EcStruct *ptr (_ecTable.lookup (ObjectHandle));

      if (!ptr) {

         ptr = new EcStruct;

         if (ptr && !_ecTable.store (ObjectHandle, ptr)) {

            delete ptr; ptr = 0;
         }
         else if (ptr) {

            ObjectModule *objMod (get_object_module ());

            if (objMod) {

               ptr->hide = objMod->lookup_flag (ObjectHandle, _hideAttrHandle);
            }
         }
      }

      if (ptr) {

         ptr->value = Value;
         _update_budget ();
      }
   }
   else if (ObjectHandle == _root) {

      if (AttributeHandle == _riskSumHandle) {

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
}


void
dmz::MBRAPluginFTCalculate::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   int index = _ui.rootBox->findData (QVariant (qlonglong (ObjectHandle)));

   if (index >= 0) {

      _ui.rootBox->setItemText (index, Value.get_buffer ());
   }
}


// QtWidget Interface
QWidget *
dmz::MBRAPluginFTCalculate::get_qt_widget () { return this; }


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
dmz::MBRAPluginFTCalculate::on_createRootButton_released () {

   ObjectModule *objMod = get_object_module ();

   if (objMod) {

      const Handle UndoHandle = _undo.start_record ("Create New Fault Tree");

      Handle root = objMod->create_object (_rootType, ObjectLocal);

      if (root) {

         objMod->store_position (root, _defaultAttrHandle, Vector (0.0, 0.0, 0.0));

         _ui.rootBox->addItem ("", QVariant (qulonglong (root)));
         int index = _ui.rootBox->findData (QVariant (qulonglong (root)));

         if (index >= 0) { _ui.rootBox->setCurrentIndex (index); }

         objMod->activate_object (root);

         Data out;
         out.store_handle (_objectDataHandle, 0, root);
         out.store_handle (_createdDataHandle, 0, root);
         _componentEditMessage.send (_editTarget, &out, 0);

         _log.debug << "Created Fault Tree Root: " << root << endl;

         if (objMod->is_object (root)) { _undo.stop_record (UndoHandle); }
         else { _undo.abort_record (UndoHandle); }
      }
      else { _undo.abort_record (UndoHandle); }
   }
}


void
dmz::MBRAPluginFTCalculate::on_rootBox_currentIndexChanged (int index) {

   if (index >= 0) {

      QVariant data = _ui.rootBox->itemData (index);

      ObjectModule *objMod = get_object_module ();
      Handle obj = data.toULongLong ();

      if (obj && objMod) {

         const Handle UndoHandle = _undo.start_record ("Change Active Fault Tree");
         objMod->store_flag (obj, _activeAttrHandle, True);
         _undo.stop_record (UndoHandle);
      }

     // Should not be needed. The script resets it self when the root changes.
#if 0
      if (_ui.calculateButton->isChecked ()) {

         // If calculation is on, toggle it off and on to reset it
         _slot_calculate (false);
         _slot_calculate (true);
      }
#endif // 0
   }
}


void
dmz::MBRAPluginFTCalculate::_update_budget () {

   Float64 total (0.0);

   HashTableHandleIterator it;
   EcStruct *ecs (0);

   while (_ecTable.get_next (it, ecs)) {

      if (!ecs->hide) { total += ecs->value; }
   }

   // _log.error << "Total: " << total << endl;
   _ui.budgetSlider->setMaximum (int (total));
   _ui.budgetSpinBox->setMaximum (int (total));
   _ui.maxBudgetLabel->setNum (int (total));
}


void
dmz::MBRAPluginFTCalculate::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();
   Definitions defs (context);

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

   _calculateOnMessage = config_create_message (
      "message.on",
      local,
      "FTStartWorkMessage",
      get_plugin_runtime_context (),
      &_log);

   _calculateOffMessage = config_create_message (
      "message.off",
      local,
      "FTStopWorkMessage",
      get_plugin_runtime_context (),
      &_log);

   _componentEditMessage = config_create_message (
      "message.component.edit",
      local,
      "FTComponentEditMessage",
      context);

   _target = config_to_named_handle (
      "message.target",
      local,
      "dmzMBRAPluginFaultTree",
      get_plugin_runtime_context ());

   _editTarget = config_to_named_handle (
      "thread-edit-target.name",
      local,
      "FaultTreeComponentProperties",
      get_plugin_runtime_context ());

   _objectDataHandle = config_to_named_handle (
      "attribute.object.name", local, "object", context);

   _createdDataHandle = config_to_named_handle (
      "attribute.created.name", local, "created", context);

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

   _defaultAttrHandle =
      activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);

   _nameAttrHandle = activate_object_attribute (
      config_to_string ("attribute.name", local, "FT_Name"),
      ObjectTextMask);

   _hideAttrHandle = activate_object_attribute (
      ObjectAttributeHideName,
      ObjectRemoveAttributeMask | ObjectFlagMask);

   _activeAttrHandle = activate_object_attribute (
      config_to_string ("attribute.flag", local, "FT_Active_Fault_Tree"),
      ObjectFlagMask);

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

   _rootType = config_to_object_type ("type.root", local, "ft_component_root", context);
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
