#include <dmzArchiveModule.h>
#include <dmzFoundationConfigFileIO.h>
#include <dmzFoundationConsts.h>
#include <dmzInputEventMasks.h>
#include <dmzInputModule.h>
#include <dmzInputConsts.h>
#include "dmzMBRAPluginMenu.h"
#include <dmzQtModuleCanvas.h>
#include <dmzQtModuleMap.h>
#include <dmzQtModuleMainWindow.h>
#include <dmzQtUtil.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzRuntimeSession.h>
#include <dmzSystemFile.h>
#include <QtGui/QtGui>
#include <qmapcontrol.h>


namespace {

static const dmz::String MBRAFileList ("mbra-file-list");

};

dmz::MBRAPluginMenu::MBRAPluginMenu (
      const PluginInfo &Info,
      Config &local,
      Config &global) :
      QObject (0),
      Plugin (Info),
      MessageObserver (Info),
      UndoObserver (Info),
      InputObserverUtil (Info, local),
      ExitObserver (Info),
      _log (Info),
      _appStateDirty (False),
      _appState (Info),
      _archiveModule (0),
      _archiveModuleName (),
      _mainWindowModule (0),
      _mainWindowModuleName (),
      _ftCanvasModule (0),
      _ftCanvasModuleName (),
      _naCanvasModule (0),
      _naCanvasModuleName (),
      _naMapModule (0),
      _naMapModuleName (),
      _archive (0),
      _undo (Info),
      _fileHandle (0),
      _suffix ("mbra"),
      _defaultExportName ("NetworkAnalysisExport"),
      _cleanUpObjMsg (0),
      _openFileMsg (0),
      _toggleLabelsMsg (0),
      _toggleLabelsTarget (0),
      _toggleLabelsAttr (0),
      _onlineHelpUrl ("http://dmzdev.org/wiki/mbra"),
      _undoAction (0),
      _redoAction (0),
      _recentFilesMenu (0),
      _recentFilesActionGroup (0),
      _exportName (QString::null),
      _ftChannel (0),
      _naChannel (0),
      _naActive (0),
      _ftActive (0),
      _maxRecentFiles (10) {

   setObjectName (get_plugin_name ().get_buffer ());

   _init (local, global);

   update_current_undo_names (0, 0);
}


dmz::MBRAPluginMenu::~MBRAPluginMenu () {

}


// Plugin Interface
void
dmz::MBRAPluginMenu::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

      Config session = get_session_config (MBRAFileList, get_plugin_runtime_context ());

      if (session) {

         ConfigIterator it;
         Config file;

         while (session.get_next_config (it, file)) {

            const String FileName = config_to_string ("name", file);

            if (is_valid_path (FileName) &&
                  (_fileCache.indexOf (FileName.get_buffer ()) < 0)) {

               _fileCache << FileName.get_buffer ();
            }
         }

         if (!_startFile.isEmpty ()) { _set_current_file (_startFile); }
         
         _update_recent_actions ();
      }
   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

      if (_fileCache.count () > 0) {

         Config fileList (MBRAFileList);
         
         foreach (QString file, _fileCache) {
         
            Config data ("file");
            data.store_attribute ("name", qPrintable (file));
            fileList.add_config (data);
         }

         set_session_config (get_plugin_runtime_context (), fileList);
      }
   }
}


void
dmz::MBRAPluginMenu::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_archiveModule) {

         _archiveModule = ArchiveModule::cast (PluginPtr, _archiveModuleName);
      }

      if (!_ftCanvasModule) {

         _ftCanvasModule = QtModuleCanvas::cast (PluginPtr, _ftCanvasModuleName);
      }

      if (!_naCanvasModule) {

         _naCanvasModule = QtModuleCanvas::cast (PluginPtr, _naCanvasModuleName);
      }

      if (!_naMapModule) {

         _naMapModule = QtModuleMap::cast (PluginPtr, _naMapModuleName);
      }

      if (!_mainWindowModule) {

         _mainWindowModule = QtModuleMainWindow::cast (PluginPtr, _mainWindowModuleName);

         if (_mainWindowModule) {
            
            HashTableStringIterator it;
            MenuStruct *ms (0);

            while (_menuTable.get_next (it, ms)) {

               foreach (QAction *action, ms->actionList) {
               
                  _mainWindowModule->add_menu_action (ms->Name, action);
                  
                  if (action->objectName () == "openAction") {

                     QMenu *menu (_mainWindowModule->lookup_menu ("&File"));
                     
                     if (menu && !_recentFilesMenu) {
                        
                        _recentFilesMenu = new QMenu ("Open Recent", menu);
                        menu->addMenu (_recentFilesMenu);
                     }
                  }
               }
            }
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_archiveModule && (_archiveModule == ArchiveModule::cast (PluginPtr))) {

         _archiveModule = 0;
      }

      if (_ftCanvasModule && (_ftCanvasModule == QtModuleCanvas::cast (PluginPtr))) {

         _ftCanvasModule = 0;
      }

      if (_naCanvasModule && (_naCanvasModule == QtModuleCanvas::cast (PluginPtr))) {

         _naCanvasModule = 0;
      }

      if (_naMapModule && (_naMapModule == QtModuleMap::cast (PluginPtr))) {

         _naMapModule = 0;
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

         if (_ok_to_load (fileName.get_buffer ())) {
         
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

      Boolean on (False);
      QString tip ("Undo");

      if (NextUndoName) {

         tip = tip + ": " + NextUndoName->get_buffer ();
         on = True;
      }

      _undoAction->setEnabled (on);
      _undoAction->setText (tip);
      _undoAction->setStatusTip (tip);
   }

   if (_redoAction) {

      Boolean on (False);
      QString tip ("Redo");

      if (NextRedoName) {

         tip = tip + ": " + NextRedoName->get_buffer ();
         on = True;
      }

      _redoAction->setEnabled (on);
      _redoAction->setText (tip);
      _redoAction->setStatusTip (tip);
   }
}


// Input Observer Interface
void
dmz::MBRAPluginMenu::update_channel_state (const Handle Channel, const Boolean State) {

   if (Channel == _naChannel) {

      _naActive += (State ? 1 : -1);
   }
   else if (Channel == _ftChannel) {
      
      _ftActive += (State ? 1 : -1);
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

      if (Value & QMessageBox::Save) { on_saveAction_triggered (); }
   }

   _appStateDirty = False;
}


void
dmz::MBRAPluginMenu::on_openAction_triggered () {
   
   if (_archiveModule) {

      QString fileName =
         QFileDialog::getOpenFileName (
            _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
            tr ("Load File"),
            _get_last_path (),
            QString ("*.") + _suffix.get_buffer ());

      if (_ok_to_load (fileName)) {
         
         _load_file (fileName);
      }
   }
}


void
dmz::MBRAPluginMenu::on_openRecentAction_triggered (QAction *action) {
   
   if (action) {
      
      const QString FileName (action->data ().toString ());
      if (_ok_to_load (FileName)) { _load_file (FileName); }
   }
}


void
dmz::MBRAPluginMenu::on_saveAction_triggered () {

   if (_exportName.isEmpty ()) { on_saveAsAction_triggered (); }
   else { _save_file (_exportName); }
}


void
dmz::MBRAPluginMenu::on_saveAsAction_triggered () {

   if (_mainWindowModule) {
      
      const QString Extension = QString::fromAscii(_suffix.get_buffer ());
      const QString Filter (tr ("MBRA files (*.%1)").arg (Extension));
   
      const QString FileName (get_save_file_name_with_extension (
         _mainWindowModule->get_qt_main_window (),
         tr ("Save File"),
         _get_last_path (),
         Filter,
         Extension));

      if (!FileName.isEmpty ()) { _save_file (FileName); }
   }
}


void
dmz::MBRAPluginMenu::on_screenGrabAction_triggered () {

   if (_mainWindowModule) {
      
      QMainWindow *mainWindow (_mainWindowModule->get_qt_main_window ());
      
      const char *format = "png";
      const QString Extension = QString::fromAscii (format);
      const QString Filter = tr ("Image files (*.%1)").arg (Extension);

      QImage image;
   
      do {
   
         const QString FileName (get_save_file_name_with_extension (
            mainWindow, tr ("Save Image"), QString::null, Filter, Extension));

         if (FileName.isEmpty ()) { break; }

         if (image.isNull ()) {

            QPixmap pixmap = _screen_grab ();

            if (!pixmap.isNull ()) { image = pixmap.toImage (); }
         }

         if (image.save (FileName, format)) {

            if (mainWindow) {
               
               QString msg = tr ("Saved image %1.").arg (QFileInfo (FileName).fileName ()); 
               mainWindow->statusBar ()->showMessage (msg, 2000);
            }

            break;
          }

          QMessageBox box (
             QMessageBox::Warning,
             tr("Save Image"),
             tr("The file %1 could not be written.").arg (FileName),
             QMessageBox::Retry | QMessageBox::Cancel,
             mainWindow);
             
           if (box.exec () == QMessageBox::Cancel) { break; }
      } while (True);
   }
}


void
dmz::MBRAPluginMenu::on_printAction_triggered () {

   if (_mainWindowModule) {
      
      QMainWindow *mainWindow (_mainWindowModule->get_qt_main_window ());
      
      QPrinter printer;
      printer.setFullPage (False);

      // Grab the image to be able to suggest suitable orientation
      const QPixmap pixmap (_screen_grab ());

      if (!pixmap.isNull ()) {

         const QSizeF pixmapSize (pixmap.size ());
         printer.setOrientation (
            pixmapSize.width () > pixmapSize.height () ? QPrinter::Landscape : QPrinter::Portrait);

         QPrintDialog dialog (&printer, mainWindow);

         if (dialog.exec () == QDialog::Accepted) {

            qApp->setOverrideCursor (QCursor (Qt::WaitCursor));

            QPainter painter (&printer);
            painter.setRenderHint (QPainter::SmoothPixmapTransform);

            const QRectF page =  painter.viewport ();
         
            const double scaling =
               qMin( page.size ().width () / pixmapSize.width (),
                     page.size ().height () / pixmapSize.height ());
                  
            const double xOffset =
               page.left () + qMax (0.0, (page.size ().width () - scaling * pixmapSize.width ())  / 2.0);
               
            const double yOffset =
               page.top ()  + qMax (0.0, (page.size ().height () - scaling * pixmapSize.height ()) / 2.0);

            // Draw
            painter.translate (xOffset, yOffset);
            painter.scale (scaling, scaling);

            painter.drawPixmap (0, 0, pixmap);

            qApp->restoreOverrideCursor ();

            if (mainWindow) {
       
                mainWindow->statusBar ()->showMessage ("Printing finished.", 2000);
            }
         }
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

      if (Value & QMessageBox::Save) { on_saveAction_triggered (); }
   }

   _cleanUpObjMsg.send ();
   _undo.reset ();
   
   _set_current_file (QString::null);
}


void
dmz::MBRAPluginMenu::on_toggleLabelsAction_triggered () {
   
   QAction *action = qobject_cast<QAction *>(sender ());
   
   if (action) {
      
      Data data;
      data.store_float64 (_toggleLabelsAttr, 0, action->isChecked ());

      _toggleLabelsMsg.send (_toggleLabelsTarget, &data, 0);
   }
}


void
dmz::MBRAPluginMenu::on_onlineHelpAction_triggered () {

   if (_onlineHelpUrl) {

      QUrl Url (_onlineHelpUrl.get_buffer ());

      QDesktopServices::openUrl (Url);
   }
}


void
dmz::MBRAPluginMenu::_update_recent_actions () {

   if (_recentFilesActionGroup && _recentFilesMenu) {
      
      QList<QAction *>actionList = _recentFilesActionGroup->actions ();
      
      foreach (QAction *action, actionList) {
         
         _recentFilesMenu->removeAction (action);
         _recentFilesActionGroup->removeAction (action);
         
         delete action;
      }
      
      if (_fileCache.count () > 0) {

         String file;
         Int32 count (0);
         QList<QString>::iterator it = _fileCache.end ();

         while ((it != _fileCache.begin ()) && (count++ < _maxRecentFiles)) {
         
            it--;
            
            QFileInfo fi (*it);
            
            QAction *action = new QAction (this);
            action->setText (fi.fileName ());
            action->setData (fi.filePath ());
            action->setStatusTip (fi.filePath ());
            
            _recentFilesMenu->addAction (action);
            _recentFilesActionGroup->addAction (action);
         }
      }
   }
}


QPixmap
dmz::MBRAPluginMenu::_screen_grab () {
   
   QPixmap screenGrab;
   
   qApp->setOverrideCursor (QCursor (Qt::WaitCursor));
   if (_naActive) { screenGrab = _na_screen_grab (); }
   else if (_ftActive) { screenGrab = _ft_screen_grab (); }
   qApp->restoreOverrideCursor ();
   
   return screenGrab;
}


QPixmap
dmz::MBRAPluginMenu::_na_screen_grab () {
   
   QPixmap screenGrab;
   
   if (_naCanvasModule && _naMapModule) {

      qmapcontrol::MapControl *map (_naMapModule->get_map_control ());
      QGraphicsView *canvas (_naCanvasModule->get_view ());
      
      if (map && canvas) {
         
         screenGrab = QPixmap (canvas->viewport ()->rect ().size ());
         
         QPainter painter (&screenGrab);
         painter.setRenderHint( QPainter::Antialiasing);
         
         map->render (&painter);
         canvas->render (&painter);
         
         painter.end ();
      }
   }
   
   return screenGrab;
}


QPixmap
dmz::MBRAPluginMenu::_ft_screen_grab () {

   QPixmap screenGrab;

   if (_ftCanvasModule) {

      QGraphicsView *view (_ftCanvasModule->get_view ());
      
      if (view) {

         screenGrab = QPixmap (view->viewport ()->rect ().size ());

         QPainter painter (&screenGrab);
         painter.setRenderHint( QPainter::Antialiasing);

         view->render (&painter);

         painter.end ();
      }
   }
   
   return screenGrab;
}


dmz::Boolean
dmz::MBRAPluginMenu::_ok_to_load (const QString &FileName) {

   Boolean retVal (False);
   
   if (!FileName.isEmpty ()) {
      
      retVal = True;
      
      if (_appStateDirty) {

         QString msg = 
            tr ("Would like to save changes before opening file: %1?").arg (FileName);

         const QMessageBox::StandardButton Button (QMessageBox::warning (
            _mainWindowModule ? _mainWindowModule->get_qt_main_window () : 0,
            "Save changes before opening file.",
            msg,
            QMessageBox::Discard | QMessageBox::Cancel | QMessageBox::Save));

         if (Button & QMessageBox::Save) { on_saveAction_triggered (); }
         else if (Button & QMessageBox::Cancel) { retVal = False; }
      }
   }
   
   return retVal;
}


void
dmz::MBRAPluginMenu::_load_file (const QString &FileName) {

   if (!FileName.isEmpty () && _mainWindowModule) {

      qApp->setOverrideCursor (QCursor (Qt::BusyCursor));

      _exportName = QString::null;

      QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();

      Config global ("global");

      if (read_config_file (
            qPrintable (FileName),
            global,
            FileTypeXML,
            &_log)) {

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

         _set_current_file (FileName);
         
          if (mainWindow) { mainWindow->statusBar ()->showMessage (msg, 5000); }
      }

      qApp->restoreOverrideCursor ();
   }
}


void
dmz::MBRAPluginMenu::_save_file (const QString &FileName) {

   if (_archiveModule && !FileName.isEmpty ()) {

      qApp->setOverrideCursor (QCursor (Qt::BusyCursor));

      Config config = _archiveModule->create_archive (_archive);

      write_config_file (
         qPrintable (FileName),
         "mbra.xml",
         config,
         0, // ConfigPrettyPrint,
         FileTypeXML,
         &_log);

      QString msg (QString ("File saved as: ") + FileName);
      _log.info << qPrintable (msg) << endl;

      _set_current_file (FileName);
         
      if (_mainWindowModule) {

         QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();
         if (mainWindow) {

            mainWindow->statusBar ()->showMessage (msg, 5000);
         }
      }

      _appState.set_default_directory (qPrintable (_exportName));

      qApp->restoreOverrideCursor ();
   }
}


void
dmz::MBRAPluginMenu::_set_current_file (const QString &FileName) {

   if (_mainWindowModule && !FileName.isEmpty ()) {

      QMainWindow *mainWindow = _mainWindowModule->get_qt_main_window ();
      if (mainWindow) {

         QString title (_mainWindowModule->get_window_name ());
         
         if (!FileName.isEmpty ()) {

            title = title + ": " + FileName;

            _appState.set_default_directory (qPrintable (FileName));
         }

         mainWindow->setWindowTitle (title);

         _exportName = FileName;
         
         Int32 index = _fileCache.indexOf  (FileName);
         if (index >= 0) { _fileCache.removeAt (index); }
         
         _fileCache << FileName;
         
         _update_recent_actions ();
      }
   }
   
   _appStateDirty = False;
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
      
#ifdef Q_WS_MAC
      action->setIconVisibleInMenu (False);
#endif
      
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

   _ftCanvasModuleName = config_to_string ("module.ft-canvas.name", local);
   _naCanvasModuleName = config_to_string ("module.na-canvas.name", local);
   _naMapModuleName = config_to_string ("module.na-map.name", local);

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
      
   _toggleLabelsMsg = config_create_message (
      "message.toggle-labels.name",
      local,
      "ToggleNodeLabelMessage",
      get_plugin_runtime_context (),
      &_log);

   _toggleLabelsTarget = config_to_named_handle (
      "message.toggle-labels.target",
      local,
      "dmzMBRAPluginNALabelFormatter",
      get_plugin_runtime_context ());

   _toggleLabelsAttr = config_to_named_handle (
      "message.toggle-labels.attribute",
      local,
      "toggle",
      get_plugin_runtime_context ());

   _suffix = config_to_string (
      "suffix.value",
      local,
      _suffix);
      
   _onlineHelpUrl = config_to_string ("help.url", local, _onlineHelpUrl);
   
   _ftChannel =
      activate_input_channel (
         config_to_string ("ft.channel", local, "FaultTreeChannel"),
         InputEventChannelStateMask);

   _naChannel =
      activate_input_channel (
         config_to_string ("na.channel", local, "NetworkAnalysisChannel"),
         InputEventChannelStateMask);

   _maxRecentFiles = config_to_int32 ("max-recent-files.value", local, _maxRecentFiles);
   
   _recentFilesActionGroup = new QActionGroup (this);
   _recentFilesActionGroup->setObjectName ("openRecentAction");
   
   Config menuList;
   if (local.lookup_all_config ("menu", menuList)) { _init_menu_list (menuList); }
   
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
