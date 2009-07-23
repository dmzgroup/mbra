#include "dmzMBRAPluginFaultTreeAutoLayout.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectConsts.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginFaultTreeAutoLayout::MBRAPluginFaultTreeAutoLayout (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      TimeSlice (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _canvasModule (0),
      _canvasModuleName (),
      _defaultAttrHandle (0),
      _linkAttrHandle (0),
      _logicAttrHandle (0),
      _nameAttrHandle (0),
      _hideAttrHandle (0),
      _activeAttrHandle (0),
      _root (0),
      _subHandle (0),
      _rootType (),
      _rootText ("Fault Tree Root"),
      _hOffset (300.0),
      _vOffset (100.0),
      _doTreeUpdate (True),
      _doZoomUpdate (False),
      _path (),
      _pathItem (0) {

   _init (local);
}


dmz::MBRAPluginFaultTreeAutoLayout::~MBRAPluginFaultTreeAutoLayout () {

   if (_pathItem) { delete _pathItem; _pathItem = 0; }
}


// Plugin Interface
void
dmz::MBRAPluginFaultTreeAutoLayout::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvasModule) {

         _canvasModule = QtModuleCanvas::cast (PluginPtr, _canvasModuleName);

         if (_canvasModule) {

            QGraphicsScene *scene (_canvasModule->get_scene ());

            if (scene) {

               _pathItem = new QGraphicsPathItem ();

               if (_pathItem) {

                  _pathItem->setZValue (-1.0);

                  scene->addItem (_pathItem);
               }
            }
         }
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvasModule && (_canvasModule == QtModuleCanvas::cast (PluginPtr))) {

         if (_pathItem) { delete _pathItem; _pathItem = 0; }

         _canvasModule = 0;
      }
   }
}


// TimeSlice Interface
void
dmz::MBRAPluginFaultTreeAutoLayout::update_time_slice (const Float64 TimeDelta) {

   if (_doTreeUpdate) {

      _update_tree ();

      _doTreeUpdate = False;
      _doZoomUpdate = True;
   }
   else if (_doZoomUpdate && _canvasModule) {

      _canvasModule->center_on (_subHandle);
      _subHandle = 0;
      
//      _canvasModule->zoom_extents ();

      _doZoomUpdate = False;
   }
}


// Object Observer Interface
void
dmz::MBRAPluginFaultTreeAutoLayout::link_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      _doTreeUpdate = True;
      _subHandle = SubHandle;
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::unlink_objects (
      const Handle LinkHandle,
      const Handle AttributeHandle,
      const UUID &SuperIdentity,
      const Handle SuperHandle,
      const UUID &SubIdentity,
      const Handle SubHandle) {

   if (AttributeHandle == _linkAttrHandle) {

      _doTreeUpdate = True;
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

   if (Value) { _root = ObjectHandle; }

   ObjectModule *objMod = get_object_module ();

   if (objMod) {

      _set_component_hide_state (ObjectHandle, Value ? False : True, *objMod);
   }

   _update_tree ();
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_set_component_hide_state (
      const Handle Obj,
      const Boolean Value,
      ObjectModule &objMod) {

_log.error << (Value ? "Hiding: " : "Showing: ") << Obj << endl;
   objMod.store_flag (Obj, _hideAttrHandle, Value);

   HandleContainer list;

   if (objMod.lookup_sub_links (Obj, _linkAttrHandle, list)) {

      Handle child = list.get_first ();

      while (child) {

         _set_component_hide_state (child, Value, objMod);
         child = list.get_next ();
      }
   }

   if (objMod.lookup_sub_links (Obj, _logicAttrHandle, list)) {

      Handle child = list.get_first ();

      while (child) {

_log.error << (Value ? "Hiding: " : "Showing: ") << child << endl;
         objMod.store_flag (child, _hideAttrHandle, Value);
         child = list.get_next ();
      }
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_update_tree () {

   ObjectModule *objMod (get_object_module ());

   if (objMod && _root) {

      HandleContainer children;

      objMod->lookup_sub_links (_root, _linkAttrHandle, children);

      _path = QPainterPath ();

      Int32 count (0);

      Handle current (children.get_first ());

      while (current) {

         _update_tree (_root, current, 1, count);

         current = children.get_next ();
      }

      Vector offset (0.0, 0.0, 0.0);

      if (count) {

         offset.set_z ((count - 1) * 0.5f * _vOffset);
      }

      objMod->store_position (_root, _defaultAttrHandle, offset);

      _update_logic (_root);

      if (children.get_count ()) {

         _update_path (_root);
      }

      if (_pathItem) { _pathItem->setPath (_path); }
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_update_tree (
      const Handle SuperHandle,
      const Handle SubHandle,
      const Int32 Column,
      Int32 &count) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && SuperHandle && SubHandle) {

      Vector rootPos (0.0, 0.0, 0.0);

      Vector superPos;
      objMod->lookup_position (SuperHandle, _defaultAttrHandle, superPos);

      Vector offset ((Column * _hOffset), 0.0, (count * _vOffset));
      Vector topPos (rootPos + offset);

      HandleContainer children;
      objMod->lookup_sub_links (SubHandle, _linkAttrHandle, children);

      if (children.get_count ()) {

         Int32 startCount (count);

         Handle current (children.get_first ());

         while (current) {

            _update_tree (SubHandle, current, Column + 1, count);

            current = children.get_next ();
         }

         Int32 endCount (count);

         offset.set_xyz (0.0, 0.0, ((endCount - startCount - 1) * 0.5f * _vOffset));
         objMod->store_position (SubHandle, _defaultAttrHandle, topPos + offset);

         _update_logic (SubHandle);

         _update_path (SubHandle);
      }
      else {

         offset.set_xyz ((Column * _hOffset), 0.0, (count * _vOffset));
         objMod->store_position (SubHandle, _defaultAttrHandle, rootPos + offset);

         count++;
      }
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_update_logic (const Handle Parent) {

   ObjectModule *objMod (get_object_module ());

   if (objMod) {

      HandleContainer logic;
      objMod->lookup_sub_links (Parent, _logicAttrHandle, logic);

      if (logic.get_count ()) {

         Vector pos;
         objMod->lookup_position (Parent, _defaultAttrHandle, pos);

         pos.set_x (pos.get_x () + (_hOffset * 0.5f));
         objMod->store_position (logic.get_first (), _defaultAttrHandle, pos);
      }
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_update_path (const Handle Object) {

   ObjectModule *objMod (get_object_module ());

   if (objMod && Object) {

      Vector rootPos;
      objMod->lookup_position (Object, _defaultAttrHandle, rootPos);
      const QPointF RootPoint (rootPos.get_x (), rootPos.get_z ());

      Vector logicPos (
         rootPos.get_x () + (0.5f * _hOffset),
         rootPos.get_y (),
         rootPos.get_z ());

      const QPointF LogicPoint (logicPos.get_x (), logicPos.get_z ());

      _path.moveTo (RootPoint);
      _path.lineTo (LogicPoint);

      HandleContainer children;
      objMod->lookup_sub_links (Object, _linkAttrHandle, children);

      if (children.get_count ()) {

         Vector pos;
         Handle current (children.get_first ());

         while (current) {

            Vector pos;
            objMod->lookup_position (current, _defaultAttrHandle, pos);
            QPointF end (pos.get_x (), pos.get_z ());

            _path.moveTo (LogicPoint);
            _path.lineTo (LogicPoint.x (), end.y ());
            _path.lineTo (end);

            current = children.get_next ();
         }
      }
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_init (Config &local) {

   const Mask EmptyMask;
   RuntimeContext *context = get_plugin_runtime_context ();

   _canvasModuleName = config_to_string ("module.canvas.name", local);

   _defaultAttrHandle = activate_default_object_attribute (EmptyMask);

   _activeAttrHandle = activate_object_attribute (
      config_to_string ("attribute.activate.name", local, "FT_Active_Fault_Tree"),
      ObjectFlagMask);

   _linkAttrHandle = activate_object_attribute (
      config_to_string ("attribute.link.name", local, "FT_Link"),
      ObjectLinkMask | ObjectUnlinkMask);

   _logicAttrHandle = config_to_named_handle (
      "attribute.logic.name", local, "FT_Logic_Link", context);

   _nameAttrHandle = config_to_named_handle (
      "attribute.threat.name", local, "FT_Name", context);

   _hideAttrHandle = config_to_named_handle (
      "attribute.hide.name", local, ObjectAttributeHideName, context);

   Definitions defs (get_plugin_runtime_context (), &_log);

   defs.lookup_object_type (
      config_to_string ("root.type", local, "ft_component_root"), _rootType);

   _rootText = config_to_string ("root.text", local, _rootText);

   _hOffset = config_to_float64 ("offset.horizontal", local, _hOffset);
   _vOffset = config_to_float64 ("offset.vertical", local, _vOffset);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFaultTreeAutoLayout (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFaultTreeAutoLayout (Info, local);
}

};
