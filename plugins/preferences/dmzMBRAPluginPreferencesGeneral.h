#ifndef DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H
#define DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H

#include <dmzQtWidget.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzTypesHandleContainer.h>
#include <dmzTypesHashTableStringTemplate.h>
#include <QtGui/QFrame>

class QFormLayout;


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

      protected Q_SLOTS:
         void _slot_scalar_value_changed (double);
      
      protected:
         struct MessageStruct {

            QWidget *widget;
            Message message;
            HandleContainer targets;

            MessageStruct () : widget (0), message (), targets () {;}
         };
         
         void _get_targets (
               const String &Name,
               Config &config,
               HandleContainer &targets);
               
         void _create_properties (Config &list);
         void _init (Config &local);

         Log _log;
         Definitions _defs;
         QFormLayout *_layout;
         Handle _valueAttrHandle;
         HashTableStringTemplate<MessageStruct> _messageTable;

      private:
         MBRAPluginPreferencesGeneral ();
         MBRAPluginPreferencesGeneral (const MBRAPluginPreferencesGeneral &);
         MBRAPluginPreferencesGeneral &operator= (const MBRAPluginPreferencesGeneral &);

   };
};

#endif // DMZ_MBRA_PLUGIN_PREFERENCES_GENERAL_DOT_H
