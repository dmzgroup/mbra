#ifndef DMZ_MBRA_PLUGIN_NA_PROPERTIES_DOT_H
#define DMZ_MBRA_PLUGIN_NA_PROPERTIES_DOT_H

#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeUndo.h>

class QWidget;
class QFormLayout;

namespace dmz {

   class ObjectModule;
   class QtModuleMainWindow;

   class MBRAPluginNAProperties :
         public Plugin,
         public MessageObserver {

      public:
         class PropertyUpdater {

            public:
               const Handle AttrHandle;
               PropertyUpdater *next;

               virtual ~PropertyUpdater () {

                  while (next) {

                     PropertyUpdater *temp = next;
                     next = next->next;
                     temp->next = 0;
                     delete temp; temp = 0;
                  }
               }

               virtual void update_object (const Handle Object, ObjectModule &module) = 0;

            protected:
               PropertyUpdater (const Handle TheAttrHandle) :
                     AttrHandle (TheAttrHandle),
                     next (0) {;}

            private:
               PropertyUpdater ();
               PropertyUpdater (const PropertyUpdater &);
               PropertyUpdater &operator= (const PropertyUpdater &);
         };

         class PropertyWidget {

            public:
               const Handle AttrHandle;
               const String Name;

               PropertyWidget *next;

               virtual ~PropertyWidget () {

                  while (next) {

                     PropertyWidget *temp = next;
                     next = next->next;
                     temp->next = 0;
                     delete temp; temp = 0;
                  }
               }

               virtual PropertyUpdater *create_widgets (
                  const Handle Object,
                  ObjectModule &module,
                  QWidget *parent,
                  QFormLayout *layout) = 0;

            protected:
               PropertyWidget (
                     const Handle TheAttrHandle,
                     const String &TheName) :
                     AttrHandle (TheAttrHandle),
                     Name (TheName),
                     next (0) {;}

            private:
               PropertyWidget ();
               PropertyWidget (const PropertyWidget &);
               PropertyWidget &operator= (const PropertyWidget &);
         };

         MBRAPluginNAProperties (const PluginInfo &Info, Config &local);
         ~MBRAPluginNAProperties ();

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

      protected:
         void _edit_node (const Handle Object, const Boolean Created);
         void _edit_link (const Handle Link, const Boolean Created);
         PropertyWidget *_create_widgets (Config &list);

         void _init (Config &local);

         Log _log;
         Undo _undo;
         Definitions _defs;

         ObjectModule *_objMod;
         QtModuleMainWindow *_window;

         Message _editMessage;

         Handle _objectDataHandle;
         Handle _createdDataHandle;

         PropertyWidget *_objectWidgets;
         PropertyWidget *_linkWidgets;

      private:
         MBRAPluginNAProperties ();
         MBRAPluginNAProperties (const MBRAPluginNAProperties &);
         MBRAPluginNAProperties &operator= (const MBRAPluginNAProperties &);

   };
};

#endif // DMZ_MBRA_PLUGIN_NA_PROPERTIES_DOT_H
