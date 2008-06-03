#ifndef DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
#define DMZ_MBRA_PLUGIN_CALCULATE_DOT_H

#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QWidget>
#include "ui_dmzMBRACalculateForm.h"

class QAction;
class QDockWidget;


namespace dmz {

   class QtModuleMainWindow;
   
   
   class MBRAPluginCalculate : public QWidget, public Plugin {

      Q_OBJECT
      
      public:
         MBRAPluginCalculate (const PluginInfo &Info, Config &local);
         ~MBRAPluginCalculate ();

         // Plugin Interface
         virtual void discover_plugin (const Plugin *PluginPtr);
         virtual void start_plugin ();
         virtual void stop_plugin ();
         virtual void shutdown_plugin ();
         virtual void remove_plugin (const Plugin *PluginPtr);

      protected slots:
         void _slot_calculate (bool On);
         
      protected:
         void _init (Config &local);

         Log _log;
         QtModuleMainWindow *_mainWindowModule;
         String _mainWindowModuleName;
         Handle _channel;
         MessageType _calculateOnMessage;
         MessageType _calculateOffMessage;
         Handle _target;
         QString _title;
         QDockWidget *_dock;
         Ui::calculateForm _ui;
         
      private:
         MBRAPluginCalculate ();
         MBRAPluginCalculate (const MBRAPluginCalculate &);
         MBRAPluginCalculate &operator= (const MBRAPluginCalculate &);

   };
};

#endif // DMZ_MBRA_PLUGIN_CALCULATE_DOT_H
