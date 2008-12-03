#ifndef DMZ_MBRA_PLUGIN_FT_TABLE_DOT_H
#define DMZ_MBRA_PLUGIN_FT_TABLE_DOT_H

#include <dmzObjectCalc.h>
#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <QtGui/QWidget>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include "ui_dmzMBRAFTTable.h"

class QDockWidget;
class QStandarItem;


namespace dmz {

   class MBRAPluginFTTable :
         public QWidget,
         public Plugin,
         public ObjectObserverUtil,
         public QtWidget {

      Q_OBJECT

      public:
         MBRAPluginFTTable (const PluginInfo &Info, Config &local);
         ~MBRAPluginFTTable ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

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
         void _item_changed (QStandardItem *item);

      protected:
         typedef QList<QStandardItem *> QStandardItemList;

         virtual void _store_object_module (ObjectModule &objMod);
         virtual void _remove_object_module (ObjectModule &objMod);

         void _init (Config &local);

         Log _log;
         Undo _undo;
         Ui::mainForm _ui;
         QString _title;
         QStandardItemModel _model;
         QSortFilterProxyModel _proxyModel;
         Handle _defaultAttrHandle;
         Handle _nameAttrHandle;
         Handle _eliminationCostAttrHandle;
         Handle _consequenceAttrHandle;
         Handle _allocationAttrHandle;
         Handle _threatAttrHandle;
         Handle _vulnerabilityAttrHandle;
         Handle _vreducedAttrHandle;
         ObjectType _threatType;
         HashTableHandleTemplate<QStandardItemList> _rowTable;
         Boolean _ignoreChange;
         ObjectAttributeCalculator *_vcalc; // Vulnerability Calculator

      private:
         MBRAPluginFTTable ();
         MBRAPluginFTTable (const MBRAPluginFTTable &);
         MBRAPluginFTTable &operator= (const MBRAPluginFTTable &);

   };
};

#endif // DMZ_MBRA_PLUGIN_FT_TABLE_DOT_H
