#include <dmzRuntimeConfig.h>
#include <dmzRuntimePlugin.h>
#include <dmzRuntimePluginContainer.h>
#include <dmzRuntimePluginInfo.h>

extern "C" {
dmz::Plugin *create_dmzArchiveModuleBasic (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzArchivePluginAutoLoad (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzArchivePluginAutoSave (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzArchivePluginObject (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzObjectModuleBasic (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzObjectPluginCleanup (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzRenderModulePickBasic (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputModuleBasic (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzLuaModuleBasic (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzLuaExtInput (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzLuaExtObject (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzLuaExtPick (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputPluginMouseEventToMessage (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputPluginMouseEventToMessage (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputPluginMouseEventToMessage (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputPluginMouseEventToMessage (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzInputPluginMouseEventToMessage (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmzMBRAModuleiPhone (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmziPhonePluginCanvasObject (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmziPhonePluginRenderPick2d (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
dmz::Plugin *create_dmziPhonePluginNodeProperties (const dmz::PluginInfo &Info, dmz::Config &local, dmz::Config &global);
}

void
dmz_create_plugins (
      dmz::RuntimeContext *context,
      dmz::Config &config,
      dmz::Config &global,
      dmz::PluginContainer &container) {

   dmz::PluginInfo *info (0);
   dmz::Config local;

   info = new dmz::PluginInfo ("dmzArchiveModuleBasic", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzArchiveModuleBasic", local);
   container.add_plugin (info, create_dmzArchiveModuleBasic (*info, local, global));

   info = new dmz::PluginInfo ("dmzArchivePluginAutoLoad", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzArchivePluginAutoLoad", local);
   container.add_plugin (info, create_dmzArchivePluginAutoLoad (*info, local, global));

   info = new dmz::PluginInfo ("dmzArchivePluginAutoSave", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzArchivePluginAutoSave", local);
   container.add_plugin (info, create_dmzArchivePluginAutoSave (*info, local, global));

   info = new dmz::PluginInfo ("dmzArchivePluginObject", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzArchivePluginObject", local);
   container.add_plugin (info, create_dmzArchivePluginObject (*info, local, global));

   info = new dmz::PluginInfo ("dmzObjectModuleBasic", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzObjectModuleBasic", local);
   container.add_plugin (info, create_dmzObjectModuleBasic (*info, local, global));

   info = new dmz::PluginInfo ("dmzObjectPluginCleanup", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzObjectPluginCleanup", local);
   container.add_plugin (info, create_dmzObjectPluginCleanup (*info, local, global));

   info = new dmz::PluginInfo ("dmzRenderModulePickBasic", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzRenderModulePickBasic", local);
   container.add_plugin (info, create_dmzRenderModulePickBasic (*info, local, global));

   info = new dmz::PluginInfo ("dmzInputModuleBasic", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzInputModuleBasic", local);
   container.add_plugin (info, create_dmzInputModuleBasic (*info, local, global));

   info = new dmz::PluginInfo ("dmzLuaModuleBasic", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzLuaModuleBasic", local);
   container.add_plugin (info, create_dmzLuaModuleBasic (*info, local, global));

   info = new dmz::PluginInfo ("dmzLuaExtInput", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzLuaExtInput", local);
   container.add_plugin (info, create_dmzLuaExtInput (*info, local, global));

   info = new dmz::PluginInfo ("dmzLuaExtObject", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzLuaExtObject", local);
   container.add_plugin (info, create_dmzLuaExtObject (*info, local, global));

   info = new dmz::PluginInfo ("dmzLuaExtPick", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzLuaExtPick", local);
   container.add_plugin (info, create_dmzLuaExtPick (*info, local, global));

   info = new dmz::PluginInfo ("CreateObjectEvent", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("CreateObjectEvent", local);
   container.add_plugin (info, create_dmzInputPluginMouseEventToMessage (*info, local, global));

   info = new dmz::PluginInfo ("DestroyObjectEvent", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("DestroyObjectEvent", local);
   container.add_plugin (info, create_dmzInputPluginMouseEventToMessage (*info, local, global));

   info = new dmz::PluginInfo ("LinkObjectsEvent", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("LinkObjectsEvent", local);
   container.add_plugin (info, create_dmzInputPluginMouseEventToMessage (*info, local, global));

   info = new dmz::PluginInfo ("UnlinkObjectsEvent", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("UnlinkObjectsEvent", local);
   container.add_plugin (info, create_dmzInputPluginMouseEventToMessage (*info, local, global));

   info = new dmz::PluginInfo ("DefaultEvent", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("DefaultEvent", local);
   container.add_plugin (info, create_dmzInputPluginMouseEventToMessage (*info, local, global));

   info = new dmz::PluginInfo ("NACanvas", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("NACanvas", local);
   container.add_plugin (info, create_dmzMBRAModuleiPhone (*info, local, global));

   info = new dmz::PluginInfo ("NACanvasObject", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("NACanvasObject", local);
   container.add_plugin (info, create_dmziPhonePluginCanvasObject (*info, local, global));

   info = new dmz::PluginInfo ("NARenderPick2d", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("NARenderPick2d", local);
   container.add_plugin (info, create_dmziPhonePluginRenderPick2d (*info, local, global));

   info = new dmz::PluginInfo ("dmzMBRAPluginNodeProperties", dmz::PluginDeleteModeDelete, context, 0);
   local.set_config_context (0);
   config.lookup_all_config_merged ("dmzMBRAPluginNodeProperties", local);
   container.add_plugin (info, create_dmziPhonePluginNodeProperties (*info, local, global));
}
