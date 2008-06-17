#ifndef DMZ_MBRA_PLUGIN_NODE_TABLE_DOT_H
#define DMZ_MBRA_PLUGIN_NODE_TABLE_DOT_H

#include <dmzObjectCalc.h>
#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <QtGui/QWidget>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include "ui_dmzMBRANodeTable.h"

class QDockWidget;
class QStandarItem;

namespace dmz {

   class QtModuleMainWindow;

   class MBRAPluginNodeTable :
         public QWidget,
         public Plugin,
         public ObjectObserverUtil {

      Q_OBJECT

      public:
         MBRAPluginNodeTable (const PluginInfo &Info, Config &local);
         ~MBRAPluginNodeTable ();

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

         virtual void link_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void unlink_objects (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle);

         virtual void update_link_attribute_object (
            const Handle LinkHandle,
            const Handle AttributeHandle,
            const UUID &SuperIdentity,
            const Handle SuperHandle,
            const UUID &SubIdentity,
            const Handle SubHandle,
            const UUID &AttributeIdentity,
            const Handle AttributeObjectHandle,
            const UUID &PrevAttributeIdentity,
            const Handle PrevAttributeObjectHandle);

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

      protected slots:
         void _node_item_changed (QStandardItem *item);

      protected:
         typedef QList<QStandardItem *> QStandardItemList;

         virtual void _store_object_module (ObjectModule &objMod);
         virtual void _remove_object_module (ObjectModule &objMod);

         void _update_degrees (const Handle ObjectHandle);
         void _init (Config &local);

         Log _log;
         Undo _undo;
         Ui::tableForm _ui;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         QString _title;
         Handle _channel;
         QDockWidget *_dock;
         QStandardItemModel _nodeModel;
         QSortFilterProxyModel _nodeProxyModel;
         Handle _defaultAttrHandle;
         Handle _objectAttrHandle;
         Handle _nameAttrHandle;
         Handle _eliminationCostAttrHandle;
         Handle _consequenceAttrHandle;
         Handle _flowAttrHandle;
         Handle _linkAttrHandle;
         Handle _rankAttrHandle;
         ObjectType _nodeType;
         ObjectType _linkType;
         HashTableHandleTemplate<QStandardItemList> _nodeRowTable;
         HashTableHandleTemplate<QStandardItemList> _linkRowTable;
         Boolean _ignoreChange;
         ObjectAttributeCalculator *_degreeCalc;

      private:
         MBRAPluginNodeTable ();
         MBRAPluginNodeTable (const MBRAPluginNodeTable &);
         MBRAPluginNodeTable &operator= (const MBRAPluginNodeTable &);
   };
};

#endif // DMZ_MBRA_PLUGIN_NODE_TABLE_DOT_H
