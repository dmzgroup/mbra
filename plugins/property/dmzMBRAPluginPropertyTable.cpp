#include "dmzMBRAPluginPropertyTable.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzQtModuleMap.h>
#include <dmzRuntimeConfig.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzRuntimeSession.h>
#include <dmzSystemFile.h>
#include <dmzSystemStreamFile.h>
#include <dmzTypesVector.h>

#include <QtGui/QtGui>

using namespace dmz;

namespace {

typedef dmz::MBRAPluginPropertyTable::PropertyWidget pwidget;

const UInt32 HandleRole (Qt::UserRole + 1);

class LineWidget : public pwidget {

   public:
      LineWidget (
         const Handle Attribute,
         const String &Name,
         const Int32 Column,
         const Boolean Editable,
         const int MaxLength);

      virtual ~LineWidget () {;}

      virtual QWidget *create_widget (QWidget *parent);

      virtual void update_property (
         const Handle Object,
         const QVariant &Data,
         ObjectModule &module);

      virtual QVariant update_variant (const QVariant &data) { return data; }

   protected:
      const int _MaxLength;
};

class ScalarWidget : public pwidget {

   public:
      ScalarWidget (
         const Handle Attribute,
         const String &Name,
         const Int32 Column,
         const Boolean Editable,
         const double Scale,
         const int Decimals,
         const double Max,
         const double Min,
         const double Step);

      virtual ~ScalarWidget () {;}

      virtual QWidget *create_widget (QWidget *parent);

      virtual void update_property (
         const Handle Object,
         const QVariant &Data,
         ObjectModule &module);

      virtual QVariant update_variant (const QVariant &data);

   protected:
      const double _Scale;
      const int _Decimals;
      const double _Max;
      const double _Min;
      const double _Step;
};

class LocalDelegate : public QItemDelegate {

   public:
      LocalDelegate (QObject *parent, HashTableUInt32Template<pwidget> &table);

      virtual QWidget *createEditor (
         QWidget *parent,
         const QStyleOptionViewItem &Option,
         const QModelIndex &Index) const;

   protected:
      HashTableUInt32Template<pwidget> &_table;
};


inline static Handle
get_real_object (const Handle Object, ObjectModule &module) {

   return module.is_link (Object) ?
      module.lookup_link_attribute_object (Object) :
      Object;
}

};


LineWidget::LineWidget (
      const Handle Attribute,
      const String &Name,
      const Int32 Column,
      const Boolean Editable,
      const int MaxLength) :
      pwidget (Attribute, Name, Column, Editable),
      _MaxLength (MaxLength) {;}


QWidget *
LineWidget::create_widget (QWidget *parent) {

   QLineEdit *result (new QLineEdit (parent));

   if (_MaxLength > 0) { result->setMaxLength (_MaxLength); }

   return result;
}


void
LineWidget::update_property (
      const Handle Object,
      const QVariant &Data,
      ObjectModule &module) {

   module.store_text (Object, Attribute, qPrintable (Data.toString ()));
}


ScalarWidget::ScalarWidget (
      const Handle Attribute,
      const String &Name,
      const Int32 Column,
      const Boolean Editable,
      const double Scale,
      const int Decimals,
      const double Max,
      const double Min,
      const double Step) :
      PropertyWidget (Attribute, Name, Column, Editable),
      _Scale (Scale > 0.0 ? Scale : 1.0),
      _Decimals (Decimals),
      _Max (Max),
      _Min (Min),
      _Step (Step) {;}


QWidget *
ScalarWidget::create_widget (QWidget *parent) {

   QDoubleSpinBox *result = new QDoubleSpinBox (parent);

   result->setDecimals (_Decimals);
   result->setRange (_Min, _Max),
   result->setSingleStep (_Step);

   return result;
}


void
ScalarWidget::update_property (
      const Handle Object,
      const QVariant &Data,
      ObjectModule &module) {

   module.store_scalar (Object, Attribute, Data.toDouble () / _Scale);
}


QVariant
ScalarWidget::update_variant (const QVariant &Data) {

   return QVariant (QString::number (Data.toDouble () * _Scale, 'f', _Decimals));
}


LocalDelegate::LocalDelegate (QObject *parent, HashTableUInt32Template<pwidget> &table) :
      QItemDelegate (parent),
      _table (table) {;}


QWidget *
LocalDelegate::createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &Option,
      const QModelIndex &Index) const {

   QWidget *result (0);

   UInt32 Column ((UInt32)Index.column ());

   pwidget *pw = _table.lookup (Column);

   if (pw) { result = pw->create_widget (parent); }
   else { result = QItemDelegate::createEditor (parent, Option, Index); }

   return result;
}


dmz::MBRAPluginPropertyTable::MBRAPluginPropertyTable (
      const PluginInfo &Info,
      Config &local) :
      QWidget (0),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      QtWidget (Info),
      _log (Info),
      _defs (Info, &_log),
      _undo (Info),
      _canvas (0),
      _map (0),
      _lastPath (get_home_directory ()),
      _model (this),
      _proxyModel (this),
      _ignoreChange (False),
      _defaultAttrHandle (0),
      _hideAttrHandle (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _init (local);
   
   adjustSize ();
}


dmz::MBRAPluginPropertyTable::~MBRAPluginPropertyTable () {

   HashTableHandleIterator it;
   QStandardItemList *itemList (0);

   while (_rowTable.get_next (it, itemList)) { qDeleteAll (*itemList); }

   _rowTable.empty ();

   _colTable.clear ();
   _attrTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginPropertyTable::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

      if (!_canvas && !_map) {

         if (_canvasName) {

             _log.error << "Canvas Module: " << _canvasName << " not found." << endl;
         }

         if (_mapName) {

             _log.error << "Map Module: " << _mapName << " not found." << endl;
         }

         _ui.centerButton->hide ();
      }

      Config session (get_session_config (
         get_plugin_name (),
         get_plugin_runtime_context ()));

      _lastPath = config_to_string ("last-path.value", session, _lastPath);
   }
   else if (State == PluginStateStop) {

      Config session (get_plugin_name ());
      Config data ("last-path");
      data.store_attribute ("value", _lastPath);
      session.add_config (data);
      set_session_config (get_plugin_runtime_context (), session);
   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginPropertyTable::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvas && _canvasName) {

         _canvas = QtModuleCanvas::cast (PluginPtr, _canvasName);
      }

      if (!_map && _mapName) {

         _map = QtModuleMap::cast (PluginPtr, _mapName);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvas && (_canvas == QtModuleCanvas::cast (PluginPtr))) { _canvas = 0; }

      if (_map && (_map == QtModuleMap::cast (PluginPtr))) { _map = 0; }
   }
}


// Object Observer Interface
void
dmz::MBRAPluginPropertyTable::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (_typeSet.contains_type (Type)) {

      QStandardItemList *itemList = new QStandardItemList ();

      HashTableUInt32Iterator it;
      PropertyWidget *pw (0);

      while (_colTable.get_next (it, pw)) {

         QStandardItem *item (new QStandardItem ());

         item->setData ((quint64)ObjectHandle, HandleRole);
         item->setSelectable (True);
         item->setEditable (pw->Editable);
         item->setEnabled (pw->Editable);

         itemList->append (item);
      }

      if (_rowTable.store (ObjectHandle, itemList)) {

         _model.appendRow (*itemList);
      }
      else {

         qDeleteAll (*itemList);
         delete itemList;
         itemList = 0;
      }
   }
}


void
dmz::MBRAPluginPropertyTable::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   QStandardItemList *itemList (_rowTable.remove (ObjectHandle));

   if (itemList) {

      QStandardItem *item (itemList->at (0));

      if (item) {

         _model.takeRow (item->row ());

         qDeleteAll (*itemList);
         delete itemList; itemList = 0;
      }
   }
}


void
dmz::MBRAPluginPropertyTable::remove_object_attribute (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Mask &AttrMask) {

   if (AttributeHandle == _hideAttrHandle) {

      if (AttrMask.contains (ObjectFlagMask)) {

         update_object_flag (Identity, ObjectHandle, AttributeHandle, False, 0);
      }
   }
}


void
dmz::MBRAPluginPropertyTable::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

}


void
dmz::MBRAPluginPropertyTable::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

}


void
dmz::MBRAPluginPropertyTable::update_link_attribute_object (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle,
      const UUID &AttributeIdentity,
      const Handle AttributeObjectHandle,
      const UUID &PrevAttributeIdentity,
      const Handle PrevAttributeObjectHandle) {


}


void
dmz::MBRAPluginPropertyTable::update_object_counter (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Int64 Value,
      const Int64 *PreviousValue) {

}


void
dmz::MBRAPluginPropertyTable::update_object_counter_minimum (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Int64 Value,
      const Int64 *PreviousValue) {

}


void
dmz::MBRAPluginPropertyTable::update_object_counter_maximum (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Int64 Value,
      const Int64 *PreviousValue) {

}


void
dmz::MBRAPluginPropertyTable::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

   if (AttributeHandle == _hideAttrHandle) {

      QStandardItemList *itemList (_rowTable.lookup (ObjectHandle));

      if (itemList) {

         QStandardItem *item (itemList->at (0));

         if (item) {

            const int Index = item->row ();
            if (Value && (Index >= 0)) { _model.takeRow (Index); }
            else if (Index < 0) { _model.appendRow (*itemList); }
         }
      }
   }
}


void
dmz::MBRAPluginPropertyTable::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   QStandardItemList *itemList (_rowTable.lookup (ObjectHandle));
   PropertyWidget *pw = _attrTable.lookup (AttributeHandle);

   if (itemList && pw) {

      _ignoreChange = True;

      QStandardItem *item = itemList->at (pw->Column);

      if (item) {

         QVariant data (Value);
         data = pw->update_variant (data);
         item->setData (data, Qt::DisplayRole);
      }

      _ignoreChange = False;

      QHeaderView *header (_ui.tableView->horizontalHeader ());
      if (header) { header->setResizeMode (pw->Column, QHeaderView::ResizeToContents); }
   }
}


void
dmz::MBRAPluginPropertyTable::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   QStandardItemList *itemList (_rowTable.lookup (ObjectHandle));
   PropertyWidget *pw = _attrTable.lookup (AttributeHandle);

   if (itemList && pw) {

      _ignoreChange = True;

      QStandardItem *item = itemList->at (pw->Column);

      if (item) {

         QVariant data (Value.get_buffer ());
         data = pw->update_variant (data);
         item->setData (data, Qt::DisplayRole);
      }

      _ignoreChange = False;

      QHeaderView *header (_ui.tableView->horizontalHeader ());
      if (header) { header->setResizeMode (pw->Column, QHeaderView::ResizeToContents); }
   }
}


// QtWidget Interface
QWidget *
dmz::MBRAPluginPropertyTable::get_qt_widget () { return this; }


// MBRAPluginPropertyTable Interface
void
dmz::MBRAPluginPropertyTable::_item_changed (QStandardItem *item) {

   if (!_ignoreChange && item) {

      const Handle Object (item->data (HandleRole).toULongLong ());
      ObjectModule *objMod (get_object_module ());
      PropertyWidget *pw (_colTable.lookup ((UInt32)item->column ()));

      if (Object && objMod && pw) {

         const String Msg ("Edit Property ");
         const Handle UndoHandle = _undo.start_record (Msg + pw->Name);
         pw->update_property (Object, item->data (Qt::DisplayRole), *objMod);
         _undo.stop_record (UndoHandle);
      }
   }
}


void
dmz::MBRAPluginPropertyTable::on_exportButton_clicked () {

   QString fileName =
      QFileDialog::getSaveFileName (
         this,
         tr ("Export Data"),
         _lastPath.get_buffer (),
         QString ("*.csv"));

   // This check is for when the file is missing the extension so we have to 
   // manually check if the file already exists.
   if (!fileName.isEmpty () && QFileInfo (fileName).suffix ().isEmpty ()) {

      fileName += ".csv";

      if (QFileInfo (fileName).isFile ()) {

         const QMessageBox::StandardButton Button (QMessageBox::warning (
            this,
            "File already exists",
            fileName + "already exists. Do you want to replace it?",
            QMessageBox::Cancel | QMessageBox::Save));

         if (Button & QMessageBox::Cancel) { fileName.clear (); }
      }
   }

   if (!fileName.isEmpty ()) {

      _lastPath = qPrintable (fileName);

      qApp->setOverrideCursor (QCursor (Qt::BusyCursor));

      FILE *file = open_file (qPrintable (fileName), "wb");

      if (file) {

         StreamFile out (file);

         HashTableHandleIterator it;
         PropertyWidget *pw (0);

         Boolean first (True);

         while (_attrTable.get_next (it, pw)) {

            if (!first) { out << ","; }
            else { first = False; }

            out << "\"" << pw->Name << "\"";
         }

         out << endl;

         const int Columns = _model.columnCount ();

         it.reset ();
         QStandardItemList *list (0);

         while (_rowTable.get_next (it, list)) {

            first = True;

            for (int ix = 0; ix < Columns; ix++) {

               if (!first) { out << ","; }
               else { first = False; }

               QStandardItem *item = list->at (ix);

               if (item) {

                  out << "\"" << qPrintable (item->text ()) << "\"";
               }
               else { out << "\"\""; }
            }

            out << endl;
         }

         QString msg (QString ("File exported as: ") + fileName);

         _log.info << qPrintable (msg) << endl;

         close_file (file);
      }

      qApp->restoreOverrideCursor ();
   }
}


void
dmz::MBRAPluginPropertyTable::on_centerButton_clicked () {

   if (_canvas || _map) {

      QItemSelectionModel *selectionModel = _ui.tableView->selectionModel ();

      if (selectionModel) {

         QModelIndexList list = _proxyModel.mapSelectionToSource (
            selectionModel->selection ()).indexes ();

         if (!list.empty ()) {

            QModelIndex &index = list.first ();
            // itemFromIndex returns NULL even with valid index.
            //QStandardItem *item = _model.itemFromIndex (index);
            QStandardItem *item = _model.item (index.row (), index.column ());

            if (item) {

               const Handle Object (item->data (HandleRole).toULongLong ());

               if (Object) {

                  if (_map) {

                     ObjectModule *objMod = get_object_module ();

                     Vector pos;

                     if (objMod &&
                           objMod->lookup_position (Object, _defaultAttrHandle, pos)) {

                        _map->center_on (pos.get_x (), pos.get_z ());
                        _map->set_zoom (_map->get_zoom ());
                     }
                  }
                  else if (_canvas) { _canvas->center_on (Object); }
               }
            }
         }
      }
   }
}


void
dmz::MBRAPluginPropertyTable::on_filter_textChanged (const QString &Text) {

   _proxyModel.setFilterFixedString (Text);

   QHeaderView *header (_ui.tableView->horizontalHeader ());
   if (header) { header->setResizeMode (0, QHeaderView::ResizeToContents); }
}


void
dmz::MBRAPluginPropertyTable::_create_properties (Config &list) {

   ConfigIterator it;
   Config property;
   Int32 count = 0;

   QStringList labels;

   while (list.get_next_config (it, property)) {

      PropertyWidget *pe (0);

      const String Type = config_to_string ("type", property);
      String name = config_to_string ("name", property);
      const Handle Attribute = _defs.create_named_handle (
         config_to_string ("attribute", property, ObjectAttributeDefaultName));
      const Int32 Column = count;
      const Boolean Editable = config_to_boolean ("editable", property, True);

      const String Prefix = config_to_string ("prefix", property);
      const String Suffix = config_to_string ("suffix", property);

      if ((Type == "line") || (Type == "line-label")) {

         const int MaxLength = config_to_int32 ("max-length", property);

         pe = new LineWidget (
            Attribute,
            name,
            Column,
            (Type == "line-label" ? False : Editable),
            MaxLength);

         if (Prefix) { name << " (" << Prefix << ")"; }
         else if (Suffix) { name << " (" << Suffix << ")"; }

         activate_object_attribute (Attribute, ObjectTextMask);
      }
      else if ((Type == "scalar") || (Type == "scalar-label")) {

         const double Scale = config_to_float64 ("scale", property, 1.0);
         const int Decimals = (int)config_to_int32 ("decimals", property, 2);
         const double Max = config_to_float64 ("max", property, 1e+10);
         const double Min = config_to_float64 ("min", property, 0);
         const double Step = config_to_float64 ("step", property, 1000);

         pe = new ScalarWidget (
            Attribute,
            name,
            Column,
            (Type == "scalar-label" ? False : Editable),
            Scale,
            Decimals,
            Max,
            Min,
            Step);

         if (Prefix) { name << " (" << Prefix << ")"; }
         else if (Suffix) { name << " (" << Suffix << ")"; }

         activate_object_attribute (Attribute, ObjectScalarMask);
      }
      else if (Type == "calc-label") { }
      else if (Type == "link-label") { } // Ignore
      else if (Type == "text") { } // Ignore
      else if (Type == "state") { } // Ignore

      if (pe) {

         if (_attrTable.store (Attribute, pe)) {

            if (!_colTable.store ((UInt32)Column, pe)) {

            }

            count++;
            labels << name.get_buffer ();
         }
         else { delete pe; pe = 0; }
      }
   }

   _model.setHorizontalHeaderLabels (labels);
}


void
dmz::MBRAPluginPropertyTable::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   _typeSet = config_to_object_type_set ("object-type-list", local, context);

   _defaultAttrHandle =
      activate_default_object_attribute (ObjectCreateMask | ObjectDestroyMask);

   _hideAttrHandle = activate_object_attribute (
      ObjectAttributeHideName,
      ObjectRemoveAttributeMask | ObjectFlagMask);

   _proxyModel.setSourceModel (&_model);
   _proxyModel.setDynamicSortFilter (True);

   _ui.tableView->setModel (&_proxyModel);
   _ui.tableView->setItemDelegate (new LocalDelegate (this, _colTable));

   Config list;

   if (local.lookup_all_config ("property-list.property", list)) {

      _create_properties (list);
   }

   QHeaderView *header (_ui.tableView->horizontalHeader ());

   if (header) {

      HashTableHandleIterator it;
      PropertyWidget *pw (0);

      while (_attrTable.get_next (it, pw)) {

         header->setResizeMode (pw->Column, QHeaderView::ResizeToContents);
      }
   }

   _canvasName = config_to_string ("module.canvas.name", local);

   if (_canvasName) { _log.info << "Canvas Module Name: " << _canvasName << endl; }

   _mapName = config_to_string ("module.map.name", local);

   if (_mapName) { _log.info << "Map Module Name: " << _mapName << endl; }

   connect (
      &_model, SIGNAL (itemChanged (QStandardItem *)),
      this, SLOT (_item_changed (QStandardItem *)));
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginPropertyTable (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginPropertyTable (Info, local);
}

};
