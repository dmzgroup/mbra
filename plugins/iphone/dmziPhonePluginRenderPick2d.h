#ifndef DMZ_IPHONE_PLUGIN_RENDER_PICK_2D_DOT_H
#define DMZ_IPHONE_PLUGIN_RENDER_PICK_2D_DOT_H

#include <dmzRenderPickUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimePlugin.h>


namespace dmz {

   class iPhoneModuleCanvas;


   class iPhonePluginRenderPick2d :
      public Plugin,
      private RenderPickUtil {

      public:
         iPhonePluginRenderPick2d (const PluginInfo &Info, Config &local);
         ~iPhonePluginRenderPick2d ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level) {;}

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // RenderModule2dPick Interface
         virtual Boolean screen_to_world (
            const Int32 ScreenPosX,
            const Int32 ScreenPosY,
            Vector &worldPosition,
            Handle &objectHandle);

         virtual Boolean world_to_screen (
            const Vector &WorldPosition,
            Int32 &screenPosX,
            Int32 &screenPosY);

         virtual Boolean source_to_world (
            const Int32 SourcePosX,
            const Int32 SourcePosY,
            Vector &worldPosition,
            Handle &objectHandle);

         virtual Boolean world_to_source (
            const Vector &WorldPosition,
            Int32 &sourcePosX,
            Int32 &sourcePosY);

      protected:
         void _init (Config &local);

         Log _log;
         iPhoneModuleCanvas *_canvasModule;
         String _canvasModuleName;

      private:
         iPhonePluginRenderPick2d ();
         iPhonePluginRenderPick2d (const iPhonePluginRenderPick2d &);
         iPhonePluginRenderPick2d &operator= (const iPhonePluginRenderPick2d &);
   };
};


#endif // DMZ_IPHONE_PLUGIN_RENDER_PICK_2D_DOT_H

