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

   class MBRAPluginArchiveSupport :
         public Plugin,
         public ObjectObserverUtil,
         public ArchiveObserverUtil {

      public:
         MBRAPluginArchiveSupport (const PluginInfo &Info, Config &local);
         ~MBRAPluginArchiveSupport ();

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

         virtual void update_object_scalar (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Float64 Value,
            const Float64 *PreviousValue);

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
         HandleContainer _ecObjects;

         Float64 _offsetX;
         Float64 _offsetY;

         Message _toggleMapMessage;

         Handle _defaultAttrHandle;
         Handle _threatAttrHandle;
         Handle _vulAttrHandle; // Vulnerability
         Handle _ecAttrHandle; // Elimination Cost
         Handle _pcAttrHandle; // Prevention Cost
         Handle _toggleHandle;
         Handle _toggleTargetHandle;
         Boolean _storeObjects;
         Int32 _version;

      private:
         MBRAPluginArchiveSupport ();
         MBRAPluginArchiveSupport (const MBRAPluginArchiveSupport &);
         MBRAPluginArchiveSupport &operator= (const MBRAPluginArchiveSupport &);

   };
};

#endif // DMZ_MBRA_PLUGIN_ARCHIVE_SUPORT_DOT_H
