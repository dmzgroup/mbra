#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtCanvasConsts.h>
#include <dmzQtModuleMainWindow.h>
#include "dmzMBRAPluginNodeTable.h"
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>

namespace dmz {

   const UInt32 NAHandleRole (Qt::UserRole + 1);

   const UInt32 NodeNameColumn (0);
   const UInt32 NodeDegreeColumn (1);
   const UInt32 NodeEliminationCostColumn (2);
   const UInt32 NodeConsequenceColumn (3);
   const UInt32 NodeRankColumn (4);
   const UInt32 NodeColumnCount (5);

   const char NodeNameHeader[] = "Name";
   const char NodeDegreeHeader[] = "Degree";
   const char NodeEliminationCostHeader[] = "Elimination Cost ($)";
   const char NodeConsequenceHeader[] ="Consequence ($)";
   const char NodeRankHeader[] = "Rank";

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

   if ((Column == NodeEliminationCostColumn) || (Column == NodeConsequenceColumn)) {

      QDoubleSpinBox *box = qobject_cast<QDoubleSpinBox *> (result);

      if (box) { box->setMinimum(0); }
   }

   return result;
}

};


dmz::MBRAPluginNodeTable::MBRAPluginNodeTable (const PluginInfo &Info, Config &local) :
      QWidget (0),
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _undo (Info),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _channel (0),
      _title (tr ("Node Data")),
      _dock (0),
      _nodeModel (0, NodeColumnCount, this),
      _nodeProxyModel (this),
      _defaultAttrHandle (0),
      _objectAttrHandle (0),
      _nameAttrHandle (0),
      _eliminationCostAttrHandle (0),
      _consequenceAttrHandle (0),
      _flowAttrHandle (0),
      _linkAttrHandle (0),
      _rankAttrHandle (0),
      _nodeType (),
      _linkType (),
      _nodeRowTable (),
      _linkRowTable (),
      _ignoreChange (False),
      _degreeCalc (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _ui.setupUi (this);

   _nodeProxyModel.setSourceModel (&_nodeModel);
   _nodeProxyModel.setDynamicSortFilter (True);

   _ui.nodeTableView->setModel (&_nodeProxyModel);
   _ui.nodeTableView->setItemDelegate (new SpinBoxDelegate (this));

   QStringList labels;
   labels << NodeNameHeader
          << NodeDegreeHeader
          << NodeEliminationCostHeader
          << NodeConsequenceHeader
          << NodeRankHeader;

   _nodeModel.setHorizontalHeaderLabels (labels);

   QHeaderView *header (_ui.nodeTableView->horizontalHeader ());

   if (header) {

      header->setResizeMode (NodeNameColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (NodeDegreeColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (NodeEliminationCostColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (NodeConsequenceColumn, QHeaderView::ResizeToContents);
      header->setResizeMode (NodeRankColumn, QHeaderView::ResizeToContents);
   }

   _init (local);

   connect (
      &_nodeModel, SIGNAL (itemChanged (QStandardItem *)),
      this, SLOT (_node_item_changed (QStandardItem *)));
}


dmz::MBRAPluginNodeTable::~MBRAPluginNodeTable () {

   if (_degreeCalc) { delete _degreeCalc; _degreeCalc = 0; }

   HashTableHandleIterator it;

   QStandardItemList *itemList (_nodeRowTable.get_first (it));

   while (itemList) {

      qDeleteAll (*itemList);
      itemList = _nodeRowTable.get_next (it);
   }

   _nodeRowTable.empty ();
}


// Plugin Interface
void
dmz::MBRAPluginNodeTable::discover_plugin (
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
dmz::MBRAPluginNodeTable::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_type (_nodeType) || Type.is_of_type (_linkType)) {

      QStandardItemList *itemList = new QStandardItemList ();

      for (int ix = 0; ix < NodeColumnCount; ix++) {

         QStandardItem *item (new QStandardItem ());

         item->setData ((quint64)ObjectHandle, NAHandleRole);
         item->setSelectable (True);
         item->setEditable (True);
         item->setEnabled (True);

         if (ix == NodeDegreeColumn ||
             ix == NodeRankColumn) {

            item->setEditable (False);
            item->setEnabled (False);
         }

         if ((ix == NodeDegreeColumn) && Type.is_of_type (_linkType)) {

            item->setData (1, Qt::DisplayRole);
         }

         itemList->append (item);
      }

      if (_nodeRowTable.store (ObjectHandle, itemList)) {

         _nodeModel.appendRow (*itemList);
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
dmz::MBRAPluginNodeTable::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   QStandardItemList *itemList (_nodeRowTable.remove (ObjectHandle));

   if (itemList) {

      QStandardItem *item (itemList->at (NodeNameColumn));

      if (item) {

         _nodeModel.takeRow (item->row ());

         qDeleteAll (*itemList);
         delete itemList; itemList = 0;
      }
   }
}


void
dmz::MBRAPluginNodeTable::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      _update_degrees (SuperHandle);
      _update_degrees (SubHandle);
   }
}


void
dmz::MBRAPluginNodeTable::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      _update_degrees (SuperHandle);
      _update_degrees (SubHandle);
   }
}


void
dmz::MBRAPluginNodeTable::update_link_attribute_object (
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

   if (AttributeHandle == _linkAttrHandle) {

   }
}


void
dmz::MBRAPluginNodeTable::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   QStandardItemList *itemList (_nodeRowTable.lookup (ObjectHandle));

   if (itemList) {

      _ignoreChange = True;

      Boolean updateVulnerabilty (False);

      QStandardItem *item (0);

      if (AttributeHandle == _eliminationCostAttrHandle) {

         item = itemList->at (NodeEliminationCostColumn);

         if (item) {

            item->setData (Value, Qt::DisplayRole);

            updateVulnerabilty = True;
         }
      }
      else if (AttributeHandle == _consequenceAttrHandle) {

         item = itemList->at (NodeConsequenceColumn);

         if (item) {

            item->setData (Value, Qt::DisplayRole);
         }
      }

      _ignoreChange = False;
   }
}


void
dmz::MBRAPluginNodeTable::update_object_text (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const String &Value,
      const String *PreviousValue) {

   QStandardItemList *itemList (_nodeRowTable.lookup (ObjectHandle));

   if (itemList) {

      _ignoreChange = True;

      QStandardItem *item (0);

      if (AttributeHandle == _nameAttrHandle) {

         item = itemList->at (NodeNameColumn);

         if (item) {

            item->setText (Value.get_buffer ());

            QHeaderView *header (_ui.nodeTableView->horizontalHeader ());

            if (header) {

               header->setResizeMode (NodeNameColumn, QHeaderView::ResizeToContents);
            }
         }
      }
      else if (AttributeHandle == _rankAttrHandle) {

         item = itemList->at (NodeRankColumn);

         if (item) {

            bool ok (false);
            QString tmp (Value.get_buffer ());
            Int32 rank (tmp.toInt (&ok));
            if (ok) { item->setData ((int)rank, Qt::DisplayRole); }
         }
      }

      _ignoreChange = False;
   }
}


void
dmz::MBRAPluginNodeTable::_store_object_module (ObjectModule &objMod) {

   if (_degreeCalc) { _degreeCalc->store_object_module (&objMod); }
}


void
dmz::MBRAPluginNodeTable::_remove_object_module (ObjectModule &objMod) {

   if (_degreeCalc) { _degreeCalc->store_object_module (0); }
}


void
dmz::MBRAPluginNodeTable::_node_item_changed (QStandardItem *item) {

   if (!_ignoreChange && item) {

      bool ok (false);

      Handle ObjHandle (item->data (NAHandleRole).toULongLong (&ok));

      ObjectModule *objMod (get_object_module ());

      if (ObjHandle && ok && objMod) {

         ok = false;
         QVariant data (item->data (Qt::DisplayRole));
         Float64 val (0.0);

         Handle undoHandle (0);

         switch (item->column ()) {

            case NodeNameColumn:

               undoHandle = _undo.start_record ("Set Node Name");
               objMod->store_text (
                  ObjHandle, _nameAttrHandle, qPrintable (data.toString ()));

               break;

            case NodeEliminationCostColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Set Node Elimination Cost");
                  objMod->store_scalar (ObjHandle, _eliminationCostAttrHandle, val);
               }

               break;

            case NodeConsequenceColumn:

               val = data.toDouble (&ok);

               if (ok) {

                  undoHandle = _undo.start_record ("Set Node Consequence");
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
dmz::MBRAPluginNodeTable::_update_degrees (const Handle ObjectHandle) {

   QStandardItemList *itemList (_nodeRowTable.lookup (ObjectHandle));

   if (itemList && _degreeCalc) {

      _ignoreChange = True;

      QStandardItem *item (itemList->at (NodeDegreeColumn));

      if (item) {

         item->setData ((int)_degreeCalc->calculate (ObjectHandle), Qt::DisplayRole);
      }

      _ignoreChange = False;
   }
}


void
dmz::MBRAPluginNodeTable::_init (Config &local) {

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _title = config_to_string (
      "dockWidget.title",
      local,
      qPrintable (_title)).get_buffer ();

   _channel = config_to_named_handle (
      "channel.name",
      local,
      "NetworkAnalysisChannel",
      get_plugin_runtime_context ());

   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask | ObjectDestroyMask); //  | ObjectStateMask);

   _objectAttrHandle = config_to_named_handle (
      "attribute.object.name", local, "object", get_plugin_runtime_context ());

   _nameAttrHandle = activate_object_attribute (
      config_to_string ("attribute.node.name", local, "NA_Node_Name"),
      ObjectTextMask);

   _eliminationCostAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.node.eliminationCost",
         local,
         "NA_Node_Elimination_Cost"),
      ObjectScalarMask);

   _consequenceAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.node.consequence",
         local,
         "NA_Node_Consequence"),
      ObjectScalarMask);

   _linkAttrHandle = activate_object_attribute (
      config_to_string (
         "attribute.node.link",
         local,
         ObjectAttributeNodeLinkName),
      ObjectLinkMask |
      ObjectUnlinkMask |
      ObjectLinkAttributeMask);

   _rankAttrHandle = activate_object_attribute (
      config_to_string ("attribute.node.rank", local, "NA_Node_Rank"),
      ObjectTextMask);

   Definitions defs (get_plugin_runtime_context (), &_log);

   defs.lookup_object_type (
      config_to_string ("type.node", local, "na_node"),
      _nodeType);

   defs.lookup_object_type (
      config_to_string ("type.link", local, "na_link_attribute"),
      _linkType);

   _degreeCalc = config_to_object_attribute_calculator (
      "degrees",
      local,
      get_plugin_runtime_context (),
      &_log);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginNodeTable (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginNodeTable (Info, local);
}

};
