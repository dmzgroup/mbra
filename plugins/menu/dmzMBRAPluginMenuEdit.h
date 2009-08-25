#ifndef DMZ_MBRA_PLUING_MENU_EDIT_DOT_H
#define DMZ_MBRA_PLUING_MENU_EDIT_DOT_H

#include <dmzApplicationState.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>
#include <QtCore/QObject>

class QAction;


namespace dmz {

   class QtModuleMainWindow;
   
   
   class MBRAPluginMenuEdit :
         public QObject,
         public Plugin,
         public MessageObserver,
         public UndoObserver {

      Q_OBJECT
   
      public:
         MBRAPluginMenuEdit (const PluginInfo &Info, Config &local);
         ~MBRAPluginMenuEdit ();

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
            const UndoTypeEnum UndoType) {;}

         virtual void update_current_undo_names (
            const String *NextUndoName,
            const String *NextRedoName);

      protected slots:
         void _slot_undo ();
         void _slot_redo ();
         void _slot_clear ();

      protected:
         void _init (Config &local);

         Log _log;
         Undo _undo;
         ApplicationStateWrapper _appState;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Message _cleanUpObjMsg;
         Message _mapPropertiesMsg;
         QAction *_undoAction;
         QAction *_redoAction;
         QAction *_clearAction;

      private:
         MBRAPluginMenuEdit ();
         MBRAPluginMenuEdit (const MBRAPluginMenuEdit &);
         MBRAPluginMenuEdit &operator= (const MBRAPluginMenuEdit &);
   };
};

#endif // DMZ_MBRA_PLUING_MENU_EDIT_DOT_H
