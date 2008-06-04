#ifndef DMZ_MBRA_PLUGIN_MODE_TOOL_BAR_DOT_H
#define DMZ_MBRA_PLUGIN_MODE_TOOL_BAR_DOT_H

#include <dmzArchiveObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QToolBar>
#include "ui_dmzMBRAModeForm.h"

class QMainWindow;
class QToolBar;


namespace dmz {

   class InputModule;
   class QtModuleMainWindow;
   
   
   class MBRAPluginModeToolBar :
         public QWidget,
         public Plugin,
         public ArchiveObserverUtil {

      Q_OBJECT
      
      public:
         MBRAPluginModeToolBar (const PluginInfo &Info, Config &local);
         ~MBRAPluginModeToolBar ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // ArchiveObserver Interface.
         virtual void create_archive (
            const Handle ArchiveHandle,
            Config &local,
            Config &global);

         virtual void process_archive (
            const Handle ArchiveHandle,
            Config &local,
            Config &global);
         
      protected slots:
         void _slot_network_analysis ();
         void _slot_fault_tree ();
         
      protected:
         void _init (Config &local);

         Log _log;
         InputModule *_inputModule;
         String _inputModuleName;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         QMainWindow *_mainWindow;
         QToolBar *_toolBar;
         Ui::modeForm _ui;
         Handle _archiveHandle;
         Handle _networkAnalysisChannel;
         Handle _faultTreeChannel;
         Handle _currentChannel;
         
      private:
         MBRAPluginModeToolBar ();
         MBRAPluginModeToolBar (const MBRAPluginModeToolBar &);
         MBRAPluginModeToolBar &operator= (const MBRAPluginModeToolBar &);
   };
};

#endif // DMZ_MBRA_PLUGIN_MODE_TOOL_BAR_DOT_H
