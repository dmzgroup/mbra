#include "dmzMBRAPluginFaultTreeAutoLayout.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimeDefinitions.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <QtGui/QtGui>


dmz::MBRAPluginFaultTreeAutoLayout::MBRAPluginFaultTreeAutoLayout (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      Sync (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _canvasModule (0),
      _canvasModuleName (),
      _defaultAttrHandle (0),
      _linkAttrHandle (0),
      _logicAttrHandle (0),
      _nameAttrHandle (0),
      _root (0),
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
dmz::MBRAPluginFaultTreeAutoLayout::discover_plugin (const Plugin *PluginPtr) {

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


void
dmz::MBRAPluginFaultTreeAutoLayout::start_plugin () {

//   if (!_root) { _root = _create_root (); }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::remove_plugin (const Plugin *PluginPtr) {

   if (_canvasModule && (_canvasModule == QtModuleCanvas::cast (PluginPtr))) {

      if (_pathItem) { delete _pathItem; _pathItem = 0; }
      
      _canvasModule = 0;
   }
}


// Sync Interface
void
dmz::MBRAPluginFaultTreeAutoLayout::update_sync (const Float64 TimeDelta) {

   if (_doTreeUpdate) {

      _update_tree ();
      
      _doTreeUpdate = False;
      _doZoomUpdate = True;
   }
   else if (_doZoomUpdate && _canvasModule) {

      _canvasModule->zoom_extents ();
      
      _doZoomUpdate = False;
   }
}


// Object Observer Interface
void
dmz::MBRAPluginFaultTreeAutoLayout::create_object (
      const UUID &Identity,
      const Handle ObjectHandle,
      const ObjectType &Type,
      const ObjectLocalityEnum Locality) {

   if (Type.is_of_exact_type (_rootType)) {

      ObjectModule *objMod (get_object_module ());
      
      if (_root && objMod) {
         
         objMod->destroy_object (_root);
      }
      
      _root = ObjectHandle;
      
      _log.debug << "Found Fault Tree Root: " << _root << endl;
   }
}


void
dmz::MBRAPluginFaultTreeAutoLayout::destroy_object (
      const UUID &Identity,
      const Handle ObjectHandle) {

   if (ObjectHandle == _root) {
      
      _root = 0;
   }
}


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
dmz::MBRAPluginFaultTreeAutoLayout::_update_tree () {

   ObjectModule *objMod (get_object_module ());
   
   if (!_root) { _root = _create_root (); }
   
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


dmz::Handle
dmz::MBRAPluginFaultTreeAutoLayout::_create_root () {

   Handle root (0);
   
   ObjectModule *objMod (get_object_module ());
   
   if (objMod) {
      
      root = objMod->create_object (_rootType, ObjectLocal);
      
      if (root) {

         objMod->store_text (root, _nameAttrHandle, _rootText);
         objMod->store_position (root, _defaultAttrHandle, Vector (0.0, 0.0, 0.0));
         objMod->activate_object (root);
         
         _log.debug << "Created Fault Tree Root: " << root << endl;
      }
   }
   
   return root;
}


void
dmz::MBRAPluginFaultTreeAutoLayout::_init (Config &local) {

   _canvasModuleName = config_to_string ("module.canvas.name", local);
   
   _defaultAttrHandle = activate_default_object_attribute (
      ObjectCreateMask | ObjectDestroyMask);

   _linkAttrHandle = activate_object_attribute (
      config_to_string ("attribute.link.name", local, "FT_Link"),
      ObjectLinkMask | ObjectUnlinkMask);

   _logicAttrHandle = config_to_named_handle (
      "attribute.logic.name", local, "FT_Logic_Link", get_plugin_runtime_context ());

   _nameAttrHandle = config_to_named_handle (
      "attribute.threat.name", local, "FT_Name", get_plugin_runtime_context ());
         
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
