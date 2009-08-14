#ifndef DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H
#define DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <QtGui/QWidget>
#include "ui_dmzMBRAFTCalculateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class MBRAPluginFTCalculate :
      public QWidget,
      public Plugin,
      public ObjectObserverUtil,
      public QtWidget {

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

         virtual void update_object_flag (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Boolean Value,
            const Boolean *PreviousValue);

         virtual void update_object_scalar (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Float64 Value,
            const Float64 *PreviousValue);

         virtual void update_object_text (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const String &Value,
            const String *PreviousValue);

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();

      protected slots:
         void _slot_calculate (bool On);
         void _slot_update_budget (int budget);
         void on_createRootButton_released ();
         void on_rootBox_currentIndexChanged (int index);

      protected:
         struct EcStruct {
            Boolean hide;
            Float64 value;

            EcStruct () : hide (False), value (0.0) {;}
         };

         void _update_budget ();
         void _init (Config &local);

         Log _log;
         Undo _undo;
         Boolean _inCreate;
         Handle _defaultAttrHandle;
         Handle _channel;
         Handle _budgetAttrHandle;
         Handle _ecHandle;
         Handle _riskSumHandle;
         Handle _riskSumReducedHandle;
         Handle _vulnerabilitySumHandle;
         Handle _vulnerabilitySumReducedHandle;
         Handle _nameAttrHandle;
         Handle _hideAttrHandle;
         Handle _activeAttrHandle;
         Message _calculateOnMessage;
         Message _calculateOffMessage;
         Message _componentEditMessage;
         Handle _root;
         Handle _target;
         Handle _editTarget;
         Handle _objectDataHandle;
         Handle _createdDataHandle;
         ObjectType _rootType;
         Ui::calculateForm _ui;
         HashTableHandleTemplate<EcStruct> _ecTable;

      private:
         MBRAPluginFTCalculate ();
         MBRAPluginFTCalculate (const MBRAPluginFTCalculate &);
         MBRAPluginFTCalculate &operator= (const MBRAPluginFTCalculate &);
   };
};

#endif // DMZ_MBRA_PLUGIN_FT_CALCULATE_DOT_H
