#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleMainWindow.h>
#include "dmzMBRAPluginFTTable.h"
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


namespace dmz {

   const UInt32 FTNameColumn (0);
   const UInt32 FTThreatColumn (1);
   const UInt32 FTVulnerabilityColumn (2);
   const UInt32 FTEliminationCostColumn (3);
   const UInt32 FTConsequenceColumn (4);
   const UInt32 FTVulnerabilityReducedColumn (5);
   const UInt32 FTAllocationColumn (6);
   const UInt32 FTColumnCount (7);

   const UInt32 FTHandleRole (Qt::UserRole + 1);

   const char FTNameHeader[] = "Name";
   const char FTThreatHeader[] = "Threat (%)";
   const char FTVulnerabilityHeader[] = "Vulnerability (%)";
   const char FTEliminationCostHeader[] = "Elimination Cost ($)";
   const char FTConsequenceHeader[] ="Consequence ($)";
   const char FTVulnerabilityReducedHeader[] = "Reduced Vulnerability (%)";
   const char FTAllocationHeader[] = "Allocation ($)";

class SpinBoxDelegate : public QItemDelegate {

   public:
      SpinBoxDelegate(QObject *parent = 0);

       QWidget *createEditor(
             QWidget *parent,
             const QStyleOptionViewItem &Option,
             const QModelIndex &Index) const;
};


SpinBoxDelegate::SpinBoxDelegate (QObject *parent) : QItemDelegate(parent) {;}


QWidget *
SpinBoxDelegate::createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &Option,
      const QModelIndex &Index) const {

   QWidget *result (QItemDelegate::createEditor (parent, Option, Index));

   Int32 Column (Index.column ());

   if (Column == FTNameColumn) {

      QLineEdit *line = qobject_cast<QLineEdit *> (result);

      if (line) { line->setMaxLength (16); }
   }
   else if ((Column == FTThreatColumn) || (Column == FTVulnerabilityColumn)) {

      QDoubleSpinBox *box = qobject_cast<QDoubleSpinBox *> (result);

      if (box) {

         box->setMinimum(0);
         box->setMaximum(100);
      }
   }
   else if ((Column == FTEliminationCostColumn) || (Column == FTConsequenceColumn)) {

      QDoubleSpinBox *box = qobject_cast<QDoubleSpinBox *> (result);

      if (box) { box->setMinimum(0); }
   }

   return result;
}

};


dmz::MBRAPluginFTTable::MBRAPluginFTTable (const PluginInfo &Info, Config &local) :
      QWidget (0),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _undo (Info),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _channel (0),
      _title (tr ("Fault Tree Data")),
      _dock (0),
      _model (0, FTColumnCount, this),
      _proxyModel (this),
      _defaultAttrHandle (0),
      _nameAttrHandle (0),
      _eliminationCostAttrHandle (0),
      _consequenceAttrHandle (0),
      _allocationAttrHandle (0),
      _threatAttrHandle (0),
      _vulnerabilityAttrHandle (0),
      _vreducedAttrHandle (0),
      _threatType (),
      _rowTable (),
      _ignoreChange (False),
      _vcalc (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _proxyModel.setSourceModel (&_model);
   _proxyModel.setDynamicSortFilter (True);

   _ui.tableView->setModel (&_proxyModel);
   _ui.tableView->setItemDelegate (new SpinBoxDelegate (this));

   QStringList labels;
   labels << FTNameHeader
          << FTThreatHeader
          << FTVulnerabilityHeader
          << FTEliminationCostHeader
          << FTConsequenceHeader
          << FTVulnerabilityReducedHeader
          << FTAllocationHeader;

   _model.setHorizontalHeaderLabels (labels);

   QHeaderView *header (_ui.tableView->horizontalHeader ());

   if (header) {

      header->setResizeMode (FTNameColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTThreatColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTVulnerabilityColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTEliminationCostColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTConsequenceColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTVulnerabilityReducedColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (FTAllocationColumn, QHeaderView::ResizeToContents);
   }

   _init (local);

   connect (
      &_model, SIGNAL (itemChanged (QStandardItem *)),
      this, SLOT (_item_changed (QStandardItem *)));
}


dmz::MBRAPluginFTTable::~MBRAPluginFTTable () {

   if (_vcalc) { delete _vcalc; _vcalc = 0; }

   HashTableHandleIterator it;

   QStandardItemList *itemList (_rowTable.get_first (it));

   while (itemList) {

      qDeleteAll (*itemList);
      itemList = _rowTable.get_next (it);
   }

   _rowTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginFTTable::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule) {

            _dock = new QDockWidget (_title, this);
            _dock->setObjectName (get_plugin_name ().get_buffer ());
            _dock->setAllowedAreas (Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

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


// Object Observer Interface
void
dmz::MBRAPluginFTTable::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_type (_threatType)) {

      QStandardItemList *itemList = new QStandardItemList ();

      for (int ix = 0; ix < FTColumnCount; ix++) {

         QStandardItem *item (new QStandardItem ());

         item->setData ((quint64)ObjectHandle, FTHandleRole);
         item->setSelectable (True);
         item->setEditable (True);
         item->setEnabled (True);

         if ((ix == FTVulnerabilityReducedColumn) || (ix == FTAllocationColumn)) {

            item->setEditable (False);
            item->setEnabled (False);
         }

         itemList->append (item);
      }

      if (_rowTable.store (ObjectHandle, itemList)) {

         _model.appendRow (*itemList);
      }
      else {

         itemList->empty ();
         delete itemList;
         itemList = 0;
      }

//      _ui.tableView->resizeColumnsToContents ();
   }
}


void
dmz::MBRAPluginFTTable::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   QStandardItemList *itemList (_rowTable.remove (ObjectHandle));

   if (itemList) {

      QStandardItem *item (itemList->at (FTNameColumn));

      if (item) {

         _model.takeRow (item->row ());

         qDeleteAll (*itemList);
         delete itemList; itemList = 0;
      }
   }
}


void
dmz::MBRAPluginFTTable::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   QStandardItemList *itemList (_rowTable.lookup (ObjectHandle));

   if (itemList) {

      _ignoreChange = True;

      Boolean updateVulnerabilty (False);

      QStandardItem *item (0);

      if (AttributeHandle == _threatAttrHandle) {

         item = itemList->at (FTThreatColumn);

         if (item) {

            item->setData (Value * 100, Qt::DisplayRole);
         }
      }
      else if (AttributeHandle == _vulnerabilityAttrHandle) {

         item = itemList->at (FTVulnerabilityColumn);

         if (item) {

            item->setData (Value * 100, Qt::DisplayRole);
         }
      }
      else if (AttributeHandle == _vreducedAttrHandle) {

         item = itemList->at (FTVulnerabilityReducedColumn);

         if (item) {

            item->setData (Value * 100, Qt::DisplayRole);
         }
      }
      else if (AttributeHandle == _eliminationCostAttrHandle) {

         item = itemList->at (FTEliminationCostColumn);

         if (item) {

            item->setData (Value, Qt::DisplayRole);

            updateVulnerabilty = True;
         }
      }
      else if (AttributeHandle == _consequenceAttrHandle) {

         item = itemList->at (FTConsequenceColumn);

         if (item) {

            item->setData (Value, Qt::DisplayRole);
         }
      }
      else if (AttributeHandle == _allocationAttrHandle) {

         item = itemList->at (FTAllocationColumn);

         if (item) {

            item->setData (Value, Qt::DisplayRole);

            updateVulnerabilty = True;
         }
      }

#if 0
      if (updateVulnerabilty && _vcalc) {

         item = itemList->at (FTVulnerabilityColumn);

         if (item) {

            bool ok (false);

            Handle ObjHandle (item->data (FTHandleRole).toULongLong (&ok));

            if (ObjHandle) {

               item->setData (_vcalc->calculate (ObjHandle), Qt::DisplayRole);
            }
         }
      }
#endif

      _ignoreChange = False;
   }
}


void
dmz::MBRAPluginFTTable::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   if (AttributeHandle == _nameAttrHandle) {

      QStandardItemList *itemList (_rowTable.lookup (ObjectHandle));

      if (itemList) {

         _ignoreChange = True;

         QStandardItem *item (itemList->at (FTNameColumn));

         if (item) {

            item->setText (Value.get_buffer ());
         }

         _ignoreChange = False;

         QHeaderView *header (_ui.tableView->horizontalHeader ());

         if (header) {

            header->setResizeMode (FTNameColumn, QHeaderView::ResizeToContents);
         }

      }
   }
}


void
dmz::MBRAPluginFTTable::_store_object_module (ObjectModule &objMod) {

   if (_vcalc) { _vcalc->store_object_module (&objMod); }
}


void
dmz::MBRAPluginFTTable::_remove_object_module (ObjectModule &objMod) {

   if (_vcalc) { _vcalc->store_object_module (0); }
}


void
dmz::MBRAPluginFTTable::_item_changed (QStandardItem *item) {

   if (!_ignoreChange && item) {

      bool ok (false);

      Handle ObjHandle (item->data (FTHandleRole).toULongLong (&ok));

      ObjectModule *objMod (get_object_module ());

      if (ObjHandle && ok && objMod) {

         ok = false;
         QVariant data (item->data (Qt::DisplayRole));
         Float64 val (0.0);

         Handle undoHandle (0);

         switch (item->column ()) {

            case FTNameColumn:

               undoHandle = _undo.start_record ("Edit Threat Name");
               objMod->store_text (
                  ObjHandle, _nameAttrHandle, qPrintable (data.toString ()));

               break;

            case FTThreatColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Edit Threat");
                  objMod->store_scalar (ObjHandle, _threatAttrHandle, val / 100.0);
               }

               break;

            case FTVulnerabilityColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Edit Threat Vulnerability");
                  objMod->store_scalar (ObjHandle, _vulnerabilityAttrHandle, val / 100.0);
               }

               break;

            case FTEliminationCostColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Edit Threat Elimination Cost");
                  objMod->store_scalar (ObjHandle, _eliminationCostAttrHandle, val);
               }

               break;

            case FTConsequenceColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Edit Threat Consequence");
                  objMod->store_scalar (ObjHandle, _consequenceAttrHandle, val);
               }

               break;

            default: break;
         }

         if (undoHandle) { _undo.stop_record (undoHandle); }
      }
   }
}


void
dmz::MBRAPluginFTTable::_init (Config &local) {

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _title = config_to_string (
      "dockWidget.title",
      local,
      qPrintable (_title)).get_buffer ();

   _channel = config_to_named_handle (
      "channel.name",
      local,
      "FaultTreeChannel",
      get_plugin_runtime_context ());

   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask | ObjectDestroyMask);

   _nameAttrHandle = activate_object_attribute (
      config_to_string ("attribute.threat.name", local, "FT_Name"),
      ObjectTextMask);

   _eliminationCostAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.threat.eliminationCost",
         local,
         "FT_Threat_Elimination_Cost"),
      ObjectScalarMask);

   _consequenceAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.threat.consequence",
         local,
         "FT_Threat_Consequence"),
      ObjectScalarMask);

   _allocationAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.threat.allocation",
         local,
         "FT_Threat_Allocation"),
      ObjectScalarMask);

   _threatAttrHandle = activate_object_attribute (
      config_to_string ("attribute.threat.value", local, "FT_Threat_Value"),
      ObjectScalarMask);

   _vulnerabilityAttrHandle = activate_object_attribute (
      config_to_string ("attribute.vulnerability.value", local, "FT_Vulnerability_Value"),
      ObjectScalarMask);

   _vreducedAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.vreduced.value",
         local,
         "FT_Vulnerability_Reduced_Value"),
      ObjectScalarMask);

   Definitions defs (get_plugin_runtime_context (), &_log);

   defs.lookup_object_type (
      config_to_string ("type.threat", local, "ft_threat"),
      _threatType);

   _vcalc = config_to_object_attribute_calculator (
      "vulnerability",
      local,
      get_plugin_runtime_context (),
      &_log);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFTTable (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFTTable (Info, local);
}

};
