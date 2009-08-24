#ifndef DMZ_MBRA_PLUGIN_FT_VIEW_STATE_DOT_H
#define DMZ_MBRA_PLUGIN_FT_VIEW_STATE_DOT_H

#include <dmzObjectObserverUtil.h>
#include <dmzRuntimeLog.h>
#include <dmzRuntimePlugin.h>

namespace dmz {

   class QtModuleCanvas;

   class MBRAPluginFTViewState :
         public Plugin,
         public ObjectObserverUtil {

      public:
         MBRAPluginFTViewState (const PluginInfo &Info, Config &local);
         ~MBRAPluginFTViewState ();

         // Plugin Interface
         virtual void update_plugin_state (
            const PluginStateEnum State,
            const UInt32 Level);

         virtual void discover_plugin (
            const PluginDiscoverEnum Mode,
            const Plugin *PluginPtr);

         // Object Observer Interface
         virtual void update_object_flag (
            const UUID &Identity,
            const Handle ObjectHandle,
            const Handle AttributeHandle,
            const Boolean Value,
            const Boolean *PreviousValue);

      protected:
         void _init (Config &local);

         Log _log;

         Handle _activeFaultTree;

         Handle _activeAttrHandle;
         Handle _viewAttrHandle;

         QtModuleCanvas *_canvas;
         String _canvasName;

      private:
         MBRAPluginFTViewState ();
         MBRAPluginFTViewState (const MBRAPluginFTViewState &);
         MBRAPluginFTViewState &operator= (const MBRAPluginFTViewState &);

   };
};

#endif // DMZ_MBRA_PLUGIN_FT_VIEW_STATE_DOT_H
