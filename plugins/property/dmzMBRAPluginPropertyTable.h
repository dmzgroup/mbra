#ifndef DMZ_MBRA_PLUGIN_PROPERTY_TABLE_DOT_H
#define DMZ_MBRA_PLUGIN_PROPERTY_TABLE_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzQtWidget.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableHandleTemplate.h>
#include <dmzTypesHashTableUInt32Template.h>

#include <QtGui/QItemDelegate>
#include <QtGui/QFrame>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QStandardItemModel>
#include <ui_PropertyTable.h>

class QDockWidget;
class QStandarItem;

namespace dmz {

   class QtModuleCanvas;
   class QtModuleMap;

   class MBRAPluginPropertyTable :
         public QFrame,
         public Plugin,
         public ObjectObserverUtil,
         public QtWidget {

      Q_OBJECT

      public:
         class PropertyWidget {

            public:
               const Handle Attribute;
               const String Name;
               const Int32 Column;
               const Boolean Editable;

               virtual ~PropertyWidget () {;}

               virtual QWidget *create_widget (QWidget *parent) = 0;

               virtual void update_property (
                  const Handle Object,
                  const QVariant &Data,
                  ObjectModule &module) = 0;

               virtual QVariant update_variant (const QVariant &data) = 0;

            protected:
               PropertyWidget (
                     const Handle TheAttribute,
                     const String &TheName,
                     const Int32 TheColumn,
                     const Boolean IsEditable) :
                     Attribute (TheAttribute),
                     Name (TheName),
                     Column (TheColumn),
                     Editable (IsEditable) {;}
         };

         MBRAPluginPropertyTable (const PluginInfo &Info, Config &local);
         ~MBRAPluginPropertyTable ();

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

         virtual void update_object_counter (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Int64 Value,
            const Int64 *PreviousValue);

         virtual void update_object_counter_minimum (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Int64 Value,
            const Int64 *PreviousValue);

         virtual void update_object_counter_maximum (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Int64 Value,
            const Int64 *PreviousValue);

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
         void _item_changed (QStandardItem *item);
         void on_exportButton_clicked ();
         void on_centerButton_clicked ();
         void on_filter_textChanged (const QString &Text);

      protected:
         typedef QList<QStandardItem *> QStandardItemList;

         void _create_properties (Config &list);
         void _init (Config &local);

         Log _log;
         Definitions _defs;
         Undo _undo;

         QtModuleCanvas *_canvas;
         String _canvasName;

         QtModuleMap *_map;
         String _mapName;

         String _lastPath;

         Ui::PropertyTable _ui;

         QStandardItemModel _model;
         QSortFilterProxyModel _proxyModel;

         ObjectTypeSet _typeSet;

         Boolean _ignoreChange;

         Handle _defaultAttrHandle;
         Handle _hideAttrHandle;

         HashTableHandleTemplate<QStandardItemList> _rowTable;

         HashTableHandleTemplate<PropertyWidget> _attrTable;
         HashTableUInt32Template<PropertyWidget> _colTable;

      private:
         MBRAPluginPropertyTable ();
         MBRAPluginPropertyTable (const MBRAPluginPropertyTable &);
         MBRAPluginPropertyTable &operator= (const MBRAPluginPropertyTable &);

   };
};

#endif // DMZ_MBRA_PLUGIN_PROPERTY_TABLE_DOT_H
