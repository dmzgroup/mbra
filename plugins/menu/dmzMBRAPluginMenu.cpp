#include <dmzArchiveModule.h>
#include "dmzMBRAPluginMenu.h"
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzSystemFile.h>
#include <dmzSystemStreamFile.h>
#include <dmzXMLUtil.h>
#include <QtGui/QtGui>


dmz::MBRAPluginMenu::MBRAPluginMenu (
      const PluginInfo &Info,
      Config &local,
      Config &global) :
      QObject (0),
      Plugin (Info),
      MessageObserver (Info),
      UndoObserver (Info),
      ExitObserver (Info),
      _log (Info),
      _appStateDirty (False),
      _appState (Info),
      _archiveModule (0),
      _archiveModuleName (),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _archive (0),
      _undo (Info),
      _fileHandle (0),
      _mapPropertiesTarget (0),
      _suffix ("mbra"),
      _defaultExportName ("NetworkAnalysisExport"),
      _cleanUpObjMsg (0),
      _openFileMsg (0),
      _mapPropertiesMsg (0),
      _undoAction (0),
      _redoAction (0) {

   setObjectName (get_plugin_name ().get_buffer ());

   _init (local, global);

   update_current_undo_names (0, 0);
}


dmz::MBRAPluginMenu::~MBRAPluginMenu () {

}


// Plugin Interface
void
dmz::MBRAPluginMenu::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_archiveModule) {

         _archiveModule = ArchiveModule::cast (PluginPtr, _archiveModuleName);
      }

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule) {
            
            HashTableStringIterator it;
            MenuStruct *ms (0);

            while (_menuTable.get_next (it, ms)) {

               foreach (QAction *action, ms->actionList) {
               
                  _mainWindowModule->add_menu_action (ms->Name, action);
               }
            }
            
            QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();
            if (mainWindow) {
               
               if (!_startFile.isEmpty ()) {

                  QString name (_mainWindowModule->get_window_name () + ": " + _startFile);
                  mainWindow->setWindowTitle (name);
               }
            }
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_archiveModule && (_archiveModule == ArchiveModule::cast (PluginPtr))) {

         _archiveModule = 0;
      }

      if (_mainWindowModule &&
            (_mainWindowModule == QtModuleMainWindow::cast (PluginPtr))) {

         HashTableStringIterator it;
         MenuStruct *ms (0);
         
         while (_menuTable.get_next (it, ms)) {

            foreach (QAction *action, ms->actionList) {

               _mainWindowModule->remove_menu_action (ms->Name, action);
            }
         }

         _mainWindowModule = 0;
      }
   }
}


// Message Observer Interface
void
dmz::MBRAPluginMenu::receive_message (
      const Message &Type,
      const Handle MessageSendHandle,
      const Handle TargetObserverHandle,
      const Data *InData,
      Data *outData) {

   if (Type == _openFileMsg) {

      if (InData) {

         String fileName;
         InData->lookup_string (_fileHandle, 0, fileName);

         if (_appStateDirty) {

            QString msg ("Would like to save changes before opening file: ");
            msg += fileName.get_buffer ();
            msg += "?";

            const QMessageBox::StandardButton Button (QMessageBox::warning (
               _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
               "Save changes before opening file.",
               msg,
               QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save));

            if (Button & QMessageBox::Save) { on_exportAction_triggered (); }
            else if (Button & QMessageBox::Cancel) { fileName.flush (); }
         }

         if (fileName) {

            _load_file (fileName.get_buffer ());
         }
      }
   }
}


// Undo Observer Interface
void
dmz::MBRAPluginMenu::update_recording_state (
      const UndoRecordingStateEnum RecordingState,
      const UndoRecordingTypeEnum RecordingType,
      const UndoTypeEnum UndoType) {

   if (_appState.is_mode_normal () && (RecordingState == UndoRecordingStateStart)) {

      _appStateDirty = True;
   }
}


void
dmz::MBRAPluginMenu::update_current_undo_names (
      const String *NextUndoName,
      const String *NextRedoName) {

   if (_undoAction) {

      QString tip ("Undo");

      if (NextUndoName) {

         tip = tip + ": " + NextUndoName->get_buffer ();
         _undoAction->setEnabled (True);
      }
      else {

         _undoAction->setEnabled (False);
      }

      _undoAction->setStatusTip (tip);
   }

   if (_redoAction) {

      QString tip ("Redo");

      if (NextRedoName) {

         tip = tip + ": " + NextRedoName->get_buffer ();
         _redoAction->setEnabled (True);
      }
      else {

         _redoAction->setEnabled (False);
      }

      _redoAction->setStatusTip (tip);
   }
}


void
dmz::MBRAPluginMenu::exit_requested (
      const ExitStatusEnum Status,
      const String &ExitReason) {

   if (_appStateDirty) {

      const QMessageBox::StandardButton Value (QMessageBox::question (
         _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
         "Save before exit.",
         "Do you want to save your changes?",
         QMessageBox::Save | QMessageBox::Discard,
         QMessageBox::Save));

      if (Value & QMessageBox::Save) { on_exportAction_triggered (); }
   }

   _appStateDirty = False;
}


void
dmz::MBRAPluginMenu::on_openAction_triggered () {
   
   if (_archiveModule) {

      QString fileName =
         QFileDialog::getOpenFileName (
            _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
            tr ("Import File"),
            _get_last_path (),
            QString ("*.") + _suffix.get_buffer ());

      _load_file (fileName);
   }
}


void
dmz::MBRAPluginMenu::on_exportAction_triggered () {

   if (_archiveModule) {

      QString fileName =
         QFileDialog::getSaveFileName (
            _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
            tr ("Export File"),
            _get_last_path (),
            QString ("*.") + _suffix.get_buffer ());

      // This check is for when the file is missing the extension so we have to 
      // manually check if the file already exists.
      if (!fileName.isEmpty () && QFileInfo (fileName).suffix ().isEmpty ()) {

         fileName += ".";
         fileName += _suffix.get_buffer ();

         if (QFileInfo (fileName).isFile ()) {

            const QMessageBox::StandardButton Button (QMessageBox::warning (
               _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
               "File already exists",
               fileName + "already exists. Do you want to replace it?",
               QMessageBox::Cancel | QMessageBox::Save));

            if (Button & QMessageBox::Cancel) { fileName.clear (); }
         }
      }

      if (!fileName.isEmpty ()) {
         qApp->setOverrideCursor (QCursor (Qt::BusyCursor));

         FILE *file = open_file (qPrintable (fileName), "wb");

         if (file) {

            _appStateDirty = False;

            StreamFile out (file);

            Config config = _archiveModule->create_archive (_archive);

            write_xml_header (out);
            format_config_to_xml (config, out);

            QString msg (QString ("File exported as: ") + fileName);

            _log.info << qPrintable (msg) << endl;

            if (_mainWindowModule) {

               QString name (_mainWindowModule->get_window_name () + ": " + fileName);
               
               QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();
               if (mainWindow) {

                  mainWindow->setWindowTitle (name);
                  mainWindow->statusBar ()->showMessage (msg, 5000);
               }
            }

            close_file (file);

            _appState.set_default_directory (qPrintable (fileName));
         }

         qApp->restoreOverrideCursor ();
      }
   }
}


void
dmz::MBRAPluginMenu::on_undoAction_triggered () {

   _appState.push_mode (ApplicationModeUndoing);

   _undo.do_next (UndoTypeUndo);

   // when pressing on the tool button associated with this action the status bar
   // text is not getting updated properly. This code gets a pointer to the widget
   // currently under the mouse pointer. If it happens to be the widget associated
   // with the action we manually tell the main window widget to update its status
   // bar message. But when this slot is called from the keyboard we don't want this
   // same behavior. That is why we check if the widget is the one under the mouse.
   // This same check is done in on_redoAction_triggered

   if (_undoAction && _mainWindowModule) {

      QWidget *mouseWidget (QApplication::widgetAt (QCursor::pos ()));

      if (mouseWidget) {

         QList<QWidget *> widgetList (_undoAction->associatedWidgets ());

         foreach (QWidget *widget, widgetList) {

            if (widget == mouseWidget) {

               _undoAction->showStatusText (_mainWindowModule->get_qt_main_window ());
            }
         }
      }
   }

   _appState.pop_mode ();
}


void
dmz::MBRAPluginMenu::on_redoAction_triggered () {

   _appState.push_mode (ApplicationModeUndoing);

   _undo.do_next (UndoTypeRedo);

   if (_redoAction && _mainWindowModule) {

      QWidget *mouseWidget (QApplication::widgetAt (QCursor::pos ()));

      if (mouseWidget) {

         QList<QWidget *> widgetList (_redoAction->associatedWidgets ());

         foreach (QWidget *widget, widgetList) {

            if (widget == mouseWidget) {

               _redoAction->showStatusText (_mainWindowModule->get_qt_main_window ());
            }
         }
      }
   }

   _appState.pop_mode ();
}

void
dmz::MBRAPluginMenu::on_clearAction_triggered () {

    if (_appStateDirty) {

      const QMessageBox::StandardButton Value (QMessageBox::question (
         _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
         "Save before clearing.",
         "Do you want to save your changes before clearing?",
         QMessageBox::Save | QMessageBox::Discard,
         QMessageBox::Save));

      if (Value & QMessageBox::Save) { on_exportAction_triggered (); }
   }

   _cleanUpObjMsg.send ();
   _undo.reset ();
}


void
dmz::MBRAPluginMenu::on_mapPropertiesAction_triggered () {

   _mapPropertiesMsg.send (_mapPropertiesTarget, 0, 0);
}


void
dmz::MBRAPluginMenu::_load_file (const QString &FileName) {

   if (!FileName.isEmpty () && _mainWindowModule) {

      qApp->setOverrideCursor (QCursor (Qt::BusyCursor));

      QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();

      Config global ("global");

      if (xml_to_config (qPrintable (FileName), global, &_log)) {

         QString msg (QString ("Loading file: ") + FileName);

         if (mainWindow) { mainWindow->statusBar ()->showMessage (msg); }

         Config archiveConfig;
         global.lookup_all_config_merged ("dmz", archiveConfig);

         _cleanUpObjMsg.send ();

         _undo.reset ();

         qApp->processEvents ();
         _archiveModule->process_archive (_archive, archiveConfig);

         msg = QString ("File loaded: ") + FileName;
         _log.info << qPrintable (msg) << endl;

         if (mainWindow) {

            QString name (_mainWindowModule->get_window_name () + ": " + FileName);
            mainWindow->setWindowTitle (name);
            mainWindow->statusBar ()->showMessage (msg, 5000);
         }

         _appState.set_default_directory (qPrintable (FileName));

         _appStateDirty = False;
      }

      qApp->restoreOverrideCursor ();
   }
}

QString
dmz::MBRAPluginMenu::_get_last_path () {

   String lastPath (_appState.get_default_directory ());

   if (is_valid_path (lastPath)) {

      if (is_directory (lastPath)) {

         lastPath << "/" << _defaultExportName << "." << _suffix;
      }
   }
   else {

      lastPath.flush () << get_home_directory () << "/" << _defaultExportName << "."
         << _suffix;
   }

   QFileInfo fi (lastPath.get_buffer ());

   return fi.absoluteFilePath ();
}


void
dmz::MBRAPluginMenu::_init_action_list (Config &actionList, MenuStruct &ms) {
   
   ConfigIterator it;
   Config actionConfig;

   while (actionList.get_next_config (it, actionConfig)) {

      QAction *action (new QAction (this));
      qaction_config_read ("", actionConfig, action);
      ms.actionList.append (action);
      
      if (action->objectName () == "undoAction") { _undoAction = action; }
      else if (action->objectName () == "redoAction") { _redoAction = action; }
   }
}


void
dmz::MBRAPluginMenu::_init_menu_list (Config &menuList) {

   ConfigIterator it;
   Config menu;
   
   while (menuList.get_next_config (it, menu)) {
      
      const String MenuName (config_to_string ("name", menu));

      MenuStruct *ms = _menuTable.lookup (MenuName);
      
      if (!ms) {
         
         ms = new MenuStruct (MenuName);
         if (!_menuTable.store (ms->Name, ms)) { delete ms; ms = 0; }
      }
      
      if (ms) {
         
         Config actionList;
         
         if (menu.lookup_all_config ("action", actionList)) {
            
            _init_action_list (actionList, *ms);
         }
      }
   }
}


void
dmz::MBRAPluginMenu::_init (Config &local, Config &global) {

   _startFile = config_to_string ("launch-file.name", global).get_buffer ();

   Definitions defs (get_plugin_runtime_context ());

   _archiveModuleName = config_to_string ("module.archive.name", local);
   _mainWindowModuleName = config_to_string ("module.mainWindow.name", local);

   _archive = defs.create_named_handle (
      config_to_string ("archive.name", local, ArchiveDefaultName));

   _fileHandle = defs.create_named_handle ("file");
   
   _openFileMsg = config_create_message (
      "message.name",
      local,
      "DMZ_Open_File_Message",
      get_plugin_runtime_context (),
      &_log);

   subscribe_to_message (_openFileMsg);

   _cleanUpObjMsg = config_create_message (
      "message.name",
      local,
      "CleanupObjectsMessage",
      get_plugin_runtime_context (),
      &_log);

   _mapPropertiesMsg = config_create_message (
      "message.mapProperties.name",
      local,
      "MapPropertiesEditMessage",
      get_plugin_runtime_context (),
      &_log);
      
   _mapPropertiesTarget = config_to_named_handle (
      "message.mapProperties.target",
      local,
      "dmzQtPluginMapProperties",
      get_plugin_runtime_context ());

   _suffix = config_to_string (
      "suffix.value",
      local,
      _suffix);
   
   Config menuList;
   if (local.lookup_all_config ("menu", menuList)) {

      _init_menu_list (menuList);
   }

   QMetaObject::connectSlotsByName (this);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginMenu (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginMenu (Info, local, global);
}

};
