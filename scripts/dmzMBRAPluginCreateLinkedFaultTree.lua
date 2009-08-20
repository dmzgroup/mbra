local LinkHandle = dmz.handle.new ("NA_Fault_Tree_Link")

function receive (self, type, data)
   if dmz.data.is_a (data) then
      local handle = data:lookup_handle ("object", 1)
      if dmz.object.is_object (handle) then
         local ft = nil
         local links = dmz.object.sub_links (handle, LinkHandle)
         if links then ft = links[1]
         else
            local name = dmz.object.text (handle, "NA_Node_Name")
            if not name then name = "Root" end
            ft = dmz.object.create ("ft_component_root")
            dmz.object.text (ft, "FT_Name", name)
            dmz.object.activate (ft)
            dmz.object.link (LinkHandle, handle, ft)
         end
         dmz.object.flag (ft, "FT_Active_Fault_Tree", true)
         dmz.input.channel ("NetworkAnalysisChannel", false)
         dmz.input.channel ("FaultTreeChannel", true)
      end
   end
end

function new (config, name)

   local self = {
      log = dmz.log.new ("lua." .. name),
      message = config:to_message ("edit.name", "CreateLinkedFaultTreeMessage"),
      obs = dmz.message_observer.new (name),
   }

   self.log:info ("Creating plugin: " .. name)
   self.obs:register (self.message, receive, self)

   return self
end
