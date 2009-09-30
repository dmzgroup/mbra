#ifndef DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H
#define DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H

#include <dmzQtWidget.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimePlugin.h>
#include <QtGui/QFrame>
#include "ui_dmzMBRAPluginPreferencesGeneral.h"


namespace dmz {

   class MBRAPluginPreferencesGeneral :
         public QFrame,
         public QtWidget,
         public Plugin {
            
   Q_OBJECT

      public:
         MBRAPluginPreferencesGeneral (const PluginInfo &Info, Config &local);
         ~MBRAPluginPreferencesGeneral ();

         // QtWidget Interface
         virtual QWidget *get_qt_widget ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

      protected:
         void _init (Config &local);

         Log _log;
         Ui::MBRAPreferencesGeneralForm _ui;

      private:
         MBRAPluginPreferencesGeneral ();
         MBRAPluginPreferencesGeneral (const MBRAPluginPreferencesGeneral &);
         MBRAPluginPreferencesGeneral &operator= (const MBRAPluginPreferencesGeneral &);

   };
};

#endif // DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H
