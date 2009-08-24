#include "dmzMBRAPluginFTViewState.h"
#include <dmzObjectAttributeMasks.h>
#include <dmzObjectModule.h>
#include <dmzQtModuleCanvas.h>
#include <dmzRuntimeConfigToNamedHandle.h>
#include <dmzRuntimeConfigToTypesBase.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>

#include <QtCore/QtCore>

dmz::MBRAPluginFTViewState::MBRAPluginFTViewState (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      ObjectObserverUtil (Info, local),
      _log (Info),
      _activeFaultTree (0),
      _activeAttrHandle (0),
      _viewAttrHandle (0),
      _canvas (0),
      _canvasName ("FTCanvas") {

   _init (local);
}


dmz::MBRAPluginFTViewState::~MBRAPluginFTViewState () {

}


// Plugin Interface
void
dmz::MBRAPluginFTViewState::update_plugin_state (
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
dmz::MBRAPluginFTViewState::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvas) { _canvas = QtModuleCanvas::cast (PluginPtr, _canvasName); }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvas && (_canvas = QtModuleCanvas::cast (PluginPtr, _canvasName))) {

         _canvas = 0;
      }
   }
}


// Object Observer Interface
void
dmz::MBRAPluginFTViewState::update_object_flag (
      const UUID &Identity,
      const Handle ObjectHandle,
      const Handle AttributeHandle,
      const Boolean Value,
      const Boolean *PreviousValue) {

   ObjectModule *objMod = get_object_module ();

   if (objMod && _canvas && Value) {

      if (_activeFaultTree) {

         const QPointF Center = _canvas->get_center ();
         const Float32 Zoom = _canvas->get_zoom ();

         Vector view (Center.x (), Center.y (), Zoom);

         objMod->store_vector (_activeFaultTree, _viewAttrHandle, view);
      }

      _activeFaultTree = ObjectHandle;
      Vector view;

      if (objMod->lookup_vector (_activeFaultTree, _viewAttrHandle, view)) {

         _canvas->set_zoom (view.get_z ());
         QPointF point (view.get_x (), view.get_y ());
         _canvas->center_on (point);
      }
      else {

         _canvas->set_zoom (1.0f);
         QPointF point (0.0f, 0.0f);
         _canvas->center_on (point);
      }
   }
}


void
dmz::MBRAPluginFTViewState::_init (Config &local) {

   _activeAttrHandle =  activate_object_attribute (
      config_to_string ("attribute.flag", local, "FT_Active_Fault_Tree"),
      ObjectFlagMask);

   _viewAttrHandle = config_to_named_handle (
      "attribute.view",
      local,
      "FT_Canvas_View",
      get_plugin_runtime_context ());
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmzMBRAPluginFTViewState (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::MBRAPluginFTViewState (Info, local);
}

};
