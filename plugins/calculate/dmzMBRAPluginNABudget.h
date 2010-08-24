#ifndef DMZ_MBRA_PLUGIN_NA_BUDGET_DOT_H
#define DMZ_MBRA_PLUGIN_NA_BUDGET_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHashTableHandleTemplate.h>

#include "ui_BudgetForm.h"

#include <QtGui/QtGui>

namespace dmz {

   class MBRAPluginNABudget :
         public QWidget,
         public QtWidget,
         public Plugin,
         public ObjectObserverUtil {

      Q_OBJECT

      public:
         MBRAPluginNABudget (const PluginInfo &Info, Config &local);
         ~MBRAPluginNABudget ();

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
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         virtual void destroy_object (const UUID &Identity, const Handle ObjectHandle);

         virtual void remove_object_attribute (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Mask &AttrMask);

         virtual void update_object_scalar (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Float64 Value,
            const Float64 *PreviousValue);

      protected slots:
         void on_preventionBudgetBox_valueChanged (int value);
         void on_responseBudgetBox_valueChanged (int value);
         void on_attackBudgetBox_valueChanged (int value);

      protected:
         struct ObjectStruct {

            Float64 pc;
            Float64 rc;
            Float64 ac;

            ObjectStruct () : pc (0.0), rc (0.0), ac (0.0) {;}
         };

         void _update_max_prevention_budget ();
         void _update_max_response_budget ();
         void _update_max_attack_budget ();

         void _init (Config &local);

         Log _log;

         Ui::BudgetForm _ui;

         ObjectType _nodeType;
         ObjectType _linkType;

         Message _preventionBudgetMessage;
         Message _responseBudgetMessage;
         Message _attackBudgetMessage;

         Handle _budgetHandle;

         Handle _pcAttrHandle;
         Handle _rcAttrHandle;
         Handle _acAttrHandle;

         Float64 _maxPreventionBudget;
         Float64 _maxResponseBudget;
         Float64 _maxAttackBudget;

         HashTableHandleTemplate<ObjectStruct> _objectTable;

      private:
         MBRAPluginNABudget ();
         MBRAPluginNABudget (const MBRAPluginNABudget &);
         MBRAPluginNABudget &operator= (const MBRAPluginNABudget &);

   };
};

#endif // DMZ_MBRA_PLUGIN_NA_BUDGET_DOT_H
