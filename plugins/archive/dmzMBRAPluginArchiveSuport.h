#ifndef DMZ_MBRA_PLUGIN_ARCHIVE_SUPORT_DOT_H
#define DMZ_MBRA_PLUGIN_ARCHIVE_SUPORT_DOT_H

#include <dmzArchiveObserverUtil.h>
#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimeMessaging.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimeObjectType.h>
#include <dmzRuntimeUndo.h>
#include <dmzTypesHandleContainer.h>

namespace dmz {

   class QtModuleMap;

   class MBRAPluginArchiveSuport :
         public Plugin,
         public ObjectObserverUtil,
         public ArchiveObserverUtil {

      public:
         MBRAPluginArchiveSuport (const PluginInfo &Info, Config &local);
         ~MBRAPluginArchiveSuport ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Object Observer Interface
         virtual void create_object (
            const UUID &Identity,
            const Handle ObjectHandle,
            const ObjectType &Type,
            const ObjectLocalityEnum Locality);

         // Archive Observer Interface
         virtual void pre_process_archive (
            const Handle ArchiveHandle,
            const Int32 Version);

         virtual void post_process_archive (
            const Handle ArchiveHandle,
            const Int32 Version);

      protected:
         void _init (Config &local);

         Log _log;
         Undo _undo;

         QtModuleMap *_map;

         ObjectTypeSet _typeSet;
         HandleContainer _objects;

         Float64 _offsetX;
         Float64 _offsetY;

         Message _toggleMapMessage;

         Handle _defaultAttrHandle;
         Handle _toggleHandle;
         Handle _toggleTargetHandle;
         Boolean _storeObjects;
         Int32 _version;

      private:
         MBRAPluginArchiveSuport ();
         MBRAPluginArchiveSuport (const MBRAPluginArchiveSuport &);
         MBRAPluginArchiveSuport &operator= (const MBRAPluginArchiveSuport &);

   };
};

#endif // DMZ_MBRA_PLUGIN_ARCHIVE_SUPORT_DOT_H
