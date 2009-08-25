#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include "dmzMBRAPluginMenuEdit.h"
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginMenuEdit::MBRAPluginMenuEdit (const PluginInfo &Info, Config &local) :
      QObject (0),
      Plugin (Info),
      MessageObserver (Info),
      UndoObserver (Info),
      _log (Info),
      _undo (Info),
      _appState (Info),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _undoAction (0),
      _redoAction (0),
      _clearAction (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _init (local);
   
   update_current_undo_names (0, 0);
}


dmz::MBRAPluginMenuEdit::~MBRAPluginMenuEdit () {

}


// Plugin Interface
void
dmz::MBRAPluginMenuEdit::update_plugin_state (
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
dmz::MBRAPluginMenuEdit::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {
         
   const String EditMenu ("&Edit");

   if (Mode == PluginDiscoverAdd) {

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule) {

            _mainWindowModule->add_menu_action (EditMenu, _undoAction);
            _mainWindowModule->add_menu_action (EditMenu, _redoAction);
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_mainWindowModule &&
            (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {

         _mainWindowModule->remove_menu_action (EditMenu, _undoAction);
         _mainWindowModule->remove_menu_action (EditMenu, _redoAction);
         _mainWindowModule = 0;
      }
   }
}


// Message Observer Interface
void
dmz::MBRAPluginMenuEdit::receive_message (
      const Message &Type,
      const UInt32 MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

}


// Undo Observer Interface
void
dmz::MBRAPluginMenuEdit::update_current_undo_names (
      const String *NextUndoName,
      const String *NextRedoName) {

   if (_undoAction) {

      QString name ("Undo");

      if (NextUndoName) {

         name = name + ": " + NextUndoName->get_buffer ();
         _undoAction->setEnabled (True);
      }
      else {

         _undoAction->setEnabled (False);
      }

      _undoAction->setText (name);
      _undoAction->setStatusTip (name);
   }

   if (_redoAction) {

      QString name ("Redo");

      if (NextRedoName) {

         name = name + ": " + NextRedoName->get_buffer ();
         _redoAction->setEnabled (True);
      }
      else {

         _redoAction->setEnabled (False);
      }

      _redoAction->setText (name);
      _redoAction->setStatusTip (name);
   }
}


void
dmz::MBRAPluginMenuEdit::_slot_undo () {

   _appState.push_mode (ApplicationModeUndoing);

   _undo.do_next (UndoTypeUndo);

   // when pressing on the tool button associated with this action the status bar
   // text is not getting updated properly. This code gets a pointer to the widget
   // currently under the mouse pointer. If it happens to be the widget associated
   // with the action we manually tell the main window widget to update its status
   // bar message. But when this slot is called from the keyboard we don't want this
   // same behavior. That is why we check if the widget is the one under the mouse.
   // This same check is done in _slot_redo

   if (_undoAction && _mainWindowModule) {

      QWidget *widgetUnderMouse (QApplication::widgetAt (QCursor::pos ()));

      if (widgetUnderMouse) {

         QList<QWidget *> widgetList (_undoAction->associatedWidgets ());

         foreach (QWidget *widget, widgetList) {

            if (widget == widgetUnderMouse) {

               _undoAction->showStatusText (_mainWindowModule->get_qt_main_window ());
            }
         }
      }
   }

   _appState.pop_mode ();
}


void
dmz::MBRAPluginMenuEdit::_slot_redo () {

   _appState.push_mode (ApplicationModeUndoing);

   _undo.do_next (UndoTypeRedo);

   if (_redoAction && _mainWindowModule) {

      QWidget *widgetUnderMouse (QApplication::widgetAt (QCursor::pos ()));

      if (widgetUnderMouse) {

         QList<QWidget *> widgetList (_redoAction->associatedWidgets ());

         foreach (QWidget *widget, widgetList) {

            if (widget == widgetUnderMouse) {

               _redoAction->showStatusText (_mainWindowModule->get_qt_main_window ());
            }
         }
      }
   }

   _appState.pop_mode ();
}

void
dmz::MBRAPluginMenuEdit::_slot_clear () {

//   _cleanUpObjMsg.send ();
   _undo.reset ();
}


void
dmz::MBRAPluginMenuEdit::_init (Config &local) {

   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _undoAction = new QAction (this);
   qaction_config_read ("undo", local, _undoAction);

   if (_undoAction) {

      _undoAction->setShortcut (QKeySequence::Undo);

      connect (
         _undoAction, SIGNAL (triggered ()),
         this, SLOT (_slot_undo ()));
   }

   _redoAction = new QAction (this);
   qaction_config_read ("redo", local, _redoAction);

   if (_redoAction) {

      _redoAction->setShortcut (QKeySequence::Redo);

      connect (
         _redoAction, SIGNAL (triggered ()),
         this, SLOT (_slot_redo ()));
   }

   _clearAction = new QAction (this);
   qaction_config_read ("clear", local, _clearAction);

   if (_clearAction) {

      connect (
         _clearAction, SIGNAL (triggered ()),
         this, SLOT (_slot_clear ()));
   }
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginMenuEdit (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginMenuEdit (Info, local);
}

};
