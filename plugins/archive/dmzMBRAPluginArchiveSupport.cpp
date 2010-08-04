#include "dmzMBRAPluginArchiveSupport.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleMap.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeData.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>

dmz::MBRAPluginArchiveSupport::MBRAPluginArchiveSupport (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      ArchiveObserverUtil (Info, local),
      _log (Info),
      _undo (Info),
      _map (0),
      _offsetX (0.5),
      _offsetY (0.5),
      _defaultAttrHandle (0),
      _threatAttrHandle (0),
      _vulAttrHandle (0), // Vulnerability
      _ecAttrHandle (0), // Elimination Cost
      _pcAttrHandle (0), // Prevention Cost
      _toggleHandle (0),
      _toggleTargetHandle (0),
      _storeObjects (False),
      _version (-1) {

   _init (local);
}


dmz::MBRAPluginArchiveSupport::~MBRAPluginArchiveSupport () {

}


// Plugin Interface
void
dmz::MBRAPluginArchiveSupport::update_plugin_state (
      const PluginStateEnum State,
      const UInt32 Level) {

   if (State == PluginStateInit) {

   }
   else if (State == PluginStateStart) {

   }
   else if (State == PluginStateStop) {

   }
   else if (State == PluginStateShutdown) {

   }
}


void
dmz::MBRAPluginArchiveSupport::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_map) { _map = QtModuleMap::cast (PluginPtr); }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_map && (_map == QtModuleMap::cast (PluginPtr))) { _map = 0; }
   }
}


// Object Observer Interface
void
dmz::MBRAPluginArchiveSupport::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (_storeObjects && _typeSet.contains_type (Type)) {

      _objects.add (ObjectHandle);
   }
}


void
dmz::MBRAPluginArchiveSupport::update_object_scalar (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Float64 Value,
      const Float64 *PreviousValue) {

   if (AttributeHandle == _ecAttrHandle) {

      ObjectModule *objMod = get_object_module ();

      if (objMod && _pcAttrHandle) {

         objMod->store_scalar (ObjectHandle, _pcAttrHandle, Value);
         objMod->remove_attribute (ObjectHandle, _ecAttrHandle, ObjectScalarMask);

         _ecObjects.add (ObjectHandle);
      }
   }
}


// Archive Observer Interface
void
dmz::MBRAPluginArchiveSupport::pre_process_archive (
      const Handle ArchiveHandle,
      const Int32 Version) {

   if (Version <= _version) {

      _objects.clear ();
      _storeObjects = True;
   }

   _ecObjects.clear ();
}


void
dmz::MBRAPluginArchiveSupport::post_process_archive (
      const Handle ArchiveHandle,
      const Int32 Version) {

   ObjectModule *objMod = get_object_module ();

   if (objMod && _storeObjects) {

      if (_objects.get_count () > 0) {

         Float64 minx = 1.0e32, miny = 1.0e32, maxx = -1.0e32, maxy = -1.0e32;
         HandleContainerIterator it;
         Handle object (0);

         while (_objects.get_next (it, object)) {

            Vector pos;

            if (objMod->lookup_position (object, _defaultAttrHandle, pos)) {

               const Float64 XX = pos.get_x ();
               const Float64 YY = pos.get_z ();

               if (XX < minx) { minx = XX; }
               if (XX > maxx) { maxx = XX; }

               if (YY < miny) { miny = YY; }
               if (YY > maxy) { maxy = YY; }
            }
         }

         const Float64 OffsetX = minx + ((maxx - minx) * 0.5);
         const Float64 OffsetY = miny + ((maxy - miny) * 0.5);
         const Float64 Scale = is_zero64 (maxx - minx) ? 1.0 : 0.6 / (maxx - minx);

         it.reset ();

         while (_objects.get_next (it, object)) {

            Vector pos;

            if (objMod->lookup_position (object, _defaultAttrHandle, pos)) {

               pos.set_x ((pos.get_x () - OffsetX) * Scale);
               pos.set_z ((pos.get_z () - OffsetY) * Scale);
               objMod->store_position (object, _defaultAttrHandle, pos);
            }
         }

         if (_map) {

            _map->center_on (0.0, 0.0);
            _map->set_zoom (10);

            Data out;
            out.store_boolean (_toggleHandle, 0, False);
            _toggleMapMessage.send (_toggleTargetHandle, &out);
         }

         _undo.reset ();
      }
   }

   if (objMod) {

      HandleContainerIterator it;
      Handle object (0);

      while (_ecObjects.get_next (it, object)) {

         Float64 value (0.0);

         if (!objMod->lookup_scalar (object, _threatAttrHandle, value)) {

            objMod->store_scalar (object, _threatAttrHandle, 1.0);
         }

         if (!objMod->lookup_scalar (object, _vulAttrHandle, value)) {

            objMod->store_scalar (object, _vulAttrHandle, 1.0);
         }
      }
   }

   _storeObjects = False;
}


void
dmz::MBRAPluginArchiveSupport::_init (Config &local) {

   RuntimeContext *context = get_plugin_runtime_context ();

   init_archive (local);

   _defaultAttrHandle = activate_default_object_attribute (ObjectCreateMask);

   _ecAttrHandle = activate_object_attribute (
      config_to_string ("elimination-cost.name", local, "NA_Node_Elimination_Cost"),
      ObjectScalarMask);

   _typeSet = config_to_object_type_set ("object-type-list", local, context);

   if (_typeSet.get_count () == 0) {

      _typeSet.add_object_type ("na_node", context);
   }

   _toggleMapMessage = config_create_message (
      "message.toggle-map.name",
      local,
      "ToggleMapMessage",
      context,
      &_log);

   _threatAttrHandle = config_to_named_handle (
      "threat.name",
      local,
      "NA_Node_Threat",
      context);

   _vulAttrHandle = config_to_named_handle (
      "vulnerability.name",
      local,
      "NA_Node_Vulnerability",
      context);

   _pcAttrHandle = config_to_named_handle (
      "prevention-cost.name",
      local,
      "NA_Node_Prevention_Cost",
      context);


   _toggleHandle = config_to_named_handle ("toggle.name", local, "toggle", context);

   _toggleTargetHandle = config_to_named_handle (
      "toggle-target.name",
      local,
      "dmzQtPluginMapProperties",
      context);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginArchiveSupport (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginArchiveSupport (Info, local);
}

};
