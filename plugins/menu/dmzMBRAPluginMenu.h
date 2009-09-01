#ifndef DMZ_MBRA_PLUING_MENU_DOT_H
#define DMZ_MBRA_PLUING_MENU_DOT_H

#include <dmzApplicationState.h>
#include <dmzRuntimeExit.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <QtCore/QObject>
#include <QtGui/QAction>


namespace dmz {

   class ArchiveModule;
   class QtModuleMainWindow;
   
   
   class MBRAPluginMenu :
         public QObject,
         public Plugin,
         public MessageObserver,
         public UndoObserver,
         public ExitObserver {

      Q_OBJECT
   
      public:
         MBRAPluginMenu (const PluginInfo &Info, Config &local, Config &global);
         ~MBRAPluginMenu ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

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

         // Exit Observer Interface
         virtual void exit_requested (
            const ExitStatusEnum Status,
            const String &ExitReason);
            
      protected slots:
         void on_openAction_triggered ();
         void on_saveAction_triggered ();
         void on_saveAsAction_triggered ();
         void on_undoAction_triggered ();
         void on_redoAction_triggered ();
         void on_clearAction_triggered ();
         void on_mapPropertiesAction_triggered ();

      protected:
         struct MenuStruct;
         
         void _load_file (const QString &FileName);
         void _save_file (const QString &FileName);
         QString _get_last_path ();
         void _init_action_list (Config &actionList, MenuStruct &ms);
         void _init_menu_list (Config &menuList);
         void _init (Config &local, Config &global);

         Log _log;
         QString _startFile;
         Boolean _appStateDirty;
         ApplicationStateWrapper _appState;
         ArchiveModule *_archiveModule;
         String _archiveModuleName;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Handle _archive;
         Undo _undo;
         Message _cleanUpObjMsg;
         Message _openFileMsg;
         Message _mapPropertiesMsg;
         Handle _fileHandle;
         Handle _mapPropertiesTarget;
         String _suffix;
         String _defaultExportName;
         QAction *_undoAction;
         QAction *_redoAction;
         QString _exportName;
         
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
