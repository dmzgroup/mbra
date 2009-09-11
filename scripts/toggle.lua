local TKey = dmz.input.get_key_value ("t")

local function receive_key_event (self, channel, key)
   if key.state then
      if TKey == key.value then
         local data = dmz.data.new ()
         data:store_boolean ("toggle", 1, self.toggle)
         self.toggle = not self.toggle
         self.message:send ("dmzMBRAPluginNALabelFormatter", data)
      end
   end
end

local function start (self)
   self.inputObs:register (
      self.config,
      { receive_key_event = receive_key_event, },
      self);
end


local function stop (self)
   self.inputObs:release_all ()
end


function new (config, name)
   local self = {
      start_plugin = start,
      stop_plugin = stop,
      name = name,
      log = dmz.log.new ("lua." .. name),
      inputObs = dmz.input_observer.new (),
      --message = config:to_message ("toggle-message.name", "ToggleNodeLabelMessage"),
      message = dmz.message.new ("ToggleNodeLabelMessage"),
      toggle = false,
   }

   self.log:info ("Creating plugin: " .. name)
   
   return self
end

