#include <dmziPhoneModuleCanvas.h>
#include "dmziPhonePluginRenderPick2d.h"
#include "dmzMBRAModuleiPhone.h"
#include <dmzRuntimeConfigRead.h>
#include <dmzRuntimePluginFactoryLinkSymbol.h>
#include <dmzRuntimePluginInfo.h>
#include <dmzTypesVector.h>


dmz::iPhonePluginRenderPick2d::iPhonePluginRenderPick2d (
      const PluginInfo &Info,
      Config &local) :
      Plugin (Info),
      RenderPick2dUtil (Info, local),
      _log (Info),
      _canvasModule (0),
      _canvasModuleName () {

   _init (local);
}


dmz::iPhonePluginRenderPick2d::~iPhonePluginRenderPick2d () {

}


// Plugin Interface
void
dmz::iPhonePluginRenderPick2d::discover_plugin (
      const PluginDiscoverEnum Mode,
      const Plugin *PluginPtr) {

   if (Mode == PluginDiscoverAdd) {

      if (!_canvasModule) {

         _canvasModule = iPhoneModuleCanvas::cast (PluginPtr, _canvasModuleName);
      }
   }
   else if (Mode == PluginDiscoverRemove) {

      if (_canvasModule && (_canvasModule == iPhoneModuleCanvas::cast (PluginPtr))) {

         _canvasModule = 0;
      }
   }
}


// RenderPick2d Interface
dmz::Boolean
dmz::iPhonePluginRenderPick2d::screen_to_world (
      const Int32 ScreenPosX,
      const Int32 ScreenPosY,
      Vector &worldPosition,
      Handle &objectHandle) {

   Boolean retVal (False);

   retVal = source_to_world (ScreenPosX, ScreenPosY, worldPosition, objectHandle);
   
   return retVal;
}


dmz::Boolean
dmz::iPhonePluginRenderPick2d::world_to_screen (
      const Vector &WorldPosition,
      Int32 &screenPosX,
      Int32 &screenPosY) {

   Boolean retVal (False);

   if (world_to_source (WorldPosition, screenPosX, screenPosY)) {

      retVal = True;
   }

   return retVal;
}


dmz::Boolean
dmz::iPhonePluginRenderPick2d::source_to_world (
      const Int32 SourcePosX,
      const Int32 SourcePosY,
      Vector &worldPosition,
      Handle &objectHandle) {

   Boolean retVal (False);

   if (_canvasModule) {

      UIView *view (_canvasModule->get_view ());

      if (view) {

         CGPoint sourcePoint = CGPointMake (SourcePosX, SourcePosY);
         CGPoint worldPoint = [view convertPoint:sourcePoint fromView:nil];

         worldPosition.set_x (worldPoint.x);
         worldPosition.set_y (worldPoint.y);

         for (UIView *sv in [view subviews]) {
            
            if (sv.tag) {
               
               CGPoint location = [view convertPoint:worldPoint toView:sv];
               
               if ([sv pointInside:location withEvent:nil]) {
                  
                  objectHandle = sv.tag;
                  break;
               }
            }
         }
         
         retVal = True;
      }
   }

   return retVal;
}


dmz::Boolean
dmz::iPhonePluginRenderPick2d::world_to_source (
      const Vector &WorldPosition,
      Int32 &sourcePosX,
      Int32 &sourcePosY) {

   Boolean retVal (False);

   if (_canvasModule) {
      
      UIView *view (_canvasModule->get_view ());

      if (view) {

         CGPoint worldPoint = CGPointMake (WorldPosition.get_x (), WorldPosition.get_y ());
         CGPoint sourcePoint = [view convertPoint:worldPoint fromView:nil];

         sourcePosX = sourcePoint.x;
         sourcePosY = sourcePoint.y;

         retVal = True;
      }
   }

   return retVal;
}


void
dmz::iPhonePluginRenderPick2d::_init (Config &local) {

   _canvasModuleName = config_to_string ("module.canvas.name", local);
}


extern "C" {

DMZ_PLUGIN_FACTORY_LINK_SYMBOL dmz::Plugin *
create_dmziPhonePluginRenderPick2d (
      const dmz::PluginInfo &Info,
      dmz::Config &local,
      dmz::Config &global) {

   return new dmz::iPhonePluginRenderPick2d (Info, local);
}

};
