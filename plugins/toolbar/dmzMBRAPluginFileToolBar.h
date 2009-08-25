#ifndef DMZ_MBRA_PLUGIN_FILE_TOOL_BAR_DOT_H
#define DMZ_MBRA_PLUGIN_FILE_TOOL_BAR_DOT_H

#include <dmzApplicationState.h>
#include <dmzRuntimeExit.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <QtCore/QObject>

class QAction;
class QToolBar;


namespace dmz {

   class ArchiveModule;
   class QtModuleMainWindow;


   class MBRAPluginFileToolBar :
         public QObject,
         public Plugin,
         public MessageObserver,
         public UndoObserver,
         public ExitObserver {

      Q_OBJECT

      public:
         MBRAPluginFileToolBar (const PluginInfo &Info, Config &local, Config &global);
         ~MBRAPluginFileToolBar ();

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
            const Handle MessageSendHandle,
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
         void _slot_file_load ();
         void _slot_file_export ();
         void _slot_undo ();
         void _slot_redo ();
         void _slot_clear ();
         void _slot_map_properties_edit ();

      protected:
         void _load_file (const QString &FileName);
         QString _get_last_path ();
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
         Message _mapPropertiesEditMsg;
         Handle _fileHandle;
         Handle _mapPropertiesEditTarget;
         String _suffix;
         String _defaultExportName;
         QToolBar *_toolBar;
         QAction *_loadAction;
         QAction *_exportAction;
         QAction *_undoAction;
         QAction *_redoAction;
         QAction *_clearAction;
         QAction *_mapPropertiesEditAction;

      private:
         MBRAPluginFileToolBar ();
         MBRAPluginFileToolBar (const MBRAPluginFileToolBar &);
         MBRAPluginFileToolBar &operator= (const MBRAPluginFileToolBar &);
   };
};

#endif // DMZ_MBRA_PLUGIN_FILE_TOOL_BAR_DOT_H
