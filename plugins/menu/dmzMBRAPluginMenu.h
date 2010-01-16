#ifndef DMZ_MBRA_PLUING_MENU_DOT_H
#define DMZ_MBRA_PLUING_MENU_DOT_H

#include <dmzApplicationState.h>
#include <dmzInputObserverUtil.h>
#include <dmzRuntimeDataConverterTypesBase.h>
#include <dmzRuntimeExit.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzSystemFile.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <QtCore/QObject>
#include <QtGui/QAction>


namespace dmz {

   class ArchiveModule;
   class QtModuleMap;
   class QtModuleCanvas;
   class QtModuleMainWindow;
   
   
   class MBRAPluginMenu :
         public QObject,
         public Plugin,
         public MessageObserver,
         public UndoObserver,
         public InputObserverUtil,
         public ExitObserver {

      Q_OBJECT
   
      public:
         MBRAPluginMenu (const PluginInfo &Info, Config &local, Config &global);
         ~MBRAPluginMenu ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Message Observer Interface
         virtual void receive_message (
            const Message &Type,
            const UInt32 MessageSendHandle,
            const Handle TargetObserverHandle,
            const Data *InData,
            Data *outData);
            
         // Undo Observer Interface
         virtual void update_recording_state (
            const UndoRecordingStateEnum RecordingState,
            const UndoRecordingTypeEnum RecordingType,
            const UndoTypeEnum UndoType);

         virtual void update_current_undo_names (
            const String *NextUndoName,
            const String *NextRedoName);

         // Input Observer Interface
         virtual void update_channel_state (const Handle Channel, const Boolean State);

         virtual void receive_axis_event (
            const Handle Channel,
            const InputEventAxis &Value) {;}

         virtual void receive_button_event (
            const Handle Channel,
            const InputEventButton &Value) {;}

         virtual void receive_switch_event (
            const Handle Channel,
            const InputEventSwitch &Value) {;}

         virtual void receive_key_event (
            const Handle Channel,
            const InputEventKey &Value) {;}

         virtual void receive_mouse_event (
            const Handle Channel,
            const InputEventMouse &Value) {;}

         virtual void receive_data_event (
            const Handle Channel,
            const Handle Source,
            const Data &Value) {;}

         // Exit Observer Interface
         virtual void exit_requested (
            const ExitStatusEnum Status,
            const String &ExitReason);
            
      protected slots:
         void on_openAction_triggered ();
         void on_openRecentAction_triggered (QAction *);
         void on_saveAction_triggered ();
         void on_saveAsAction_triggered ();
         void on_screenGrabAction_triggered ();
         void on_printAction_triggered ();
         void on_undoAction_triggered ();
         void on_redoAction_triggered ();
         void on_clearAction_triggered ();
         void on_toggleLabelsAction_triggered ();
         void on_onlineHelpAction_triggered ();

      protected:
         struct MenuStruct;
         void _update_recent_actions ();
         QPixmap _screen_grab ();
         QPixmap _na_screen_grab ();
         QPixmap _ft_screen_grab ();
         Boolean _ok_to_load (const QString &FileName);
         void _load_file (const QString &FileName);
         void _save_file (const QString &FileName);
         void _set_current_file (const QString &FileName);
         QString _get_last_path ();
         void _init_action_list (Config &actionList, MenuStruct &ms);
         void _init_menu_list (Config &menuList);
         void _init (Config &local, Config &global);

         Log _log;
         QString _startFile;
         Boolean _appStateDirty;
         ApplicationState _appState;
         ArchiveModule *_archiveModule;
         String _archiveModuleName;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         QtModuleCanvas *_ftCanvasModule;
         String _ftCanvasModuleName;
         QtModuleCanvas *_naCanvasModule;
         String _naCanvasModuleName;
         QtModuleMap *_naMapModule;
         String _naMapModuleName;
         Handle _archive;
         Undo _undo;
         DataConverterBoolean _boolConvert;
         QStringList _fileCache;
         Message _cleanUpObjMsg;
         Message _openFileMsg;
         Message _toggleLabelsMsg;
         Message _fileMsg;
         Handle _toggleLabelsTarget;
         Handle _toggleLabelsAttr;
         Handle _fileHandle;
         String _suffix;
         String _defaultExportName;
         String _onlineHelpUrl;
         QAction *_undoAction;
         QAction *_redoAction;
         QMenu *_recentFilesMenu;
         QActionGroup *_recentFilesActionGroup;
         QString _exportName;
         Handle _naChannel;
         Handle _ftChannel;
         Int32 _naActive;
         Int32 _ftActive;
         Int32 _maxRecentFiles;
         Boolean _uncompressFile;
         
         struct MenuStruct {

            const String Name;
            QList<QAction *> actionList;
            
            MenuStruct (const String &TheName) : Name (TheName) {;}
         };
         
         HashTableStringTemplate<MenuStruct> _menuTable;
         
      private:
         MBRAPluginMenu ();
         MBRAPluginMenu (const MBRAPluginMenu &);
         MBRAPluginMenu &operator= (const MBRAPluginMenu &);
   };
};

#endif // DMZ_MBRA_PLUING_MENU_DOT_H
