---@meta snd

snd = {
    param={},
    preset={},
    marker={},
}

---
---Starts playing the track - equivalent to `snd.paused(false)`
---
---@return nil
function snd.play() end

---
---Pauses the track - equivalent to `snd.paused(true)`
---@return nil
function snd.pause() end

---
---Pause or unpause the track. If no value is passed to `pause`, it will
---instead return the current paused status of the track.
---
---@param pause? boolean - whether to pause track
---
---@return boolean - whether or not track is paused.
function snd.paused(pause) end

---
---Set the main master volume. If no value is passed to `volume`, it will
---instead return the current main volume level.
---
---@param volume? number - volume level (0=off, 1=100%, max=2)
---@return number - current main volume level
function snd.main_volume(volume) end

---
---Set an individual channel's volume. If not value is passed to `volume, it
---will instead return the current volume level for channel.
---
---@param channel number - channel to set volume of, ranging from 1 to
---                        max channels
---@param volume? number - volume level (0=off, 1=100%, max=2)
---@return number - current volume level of `channel`
function snd.channel_volume(channel, volume) end

---
---Get the number of channels on the current track
---
---@return number - number of channels on the current track.
function snd.channel_count() end


---------- param namespace ----------------------------------------------------

---
---Set a parameter's value
---
---@param index_or_name number | string - index of the parameter or its name
---@param value         number          - parameter value
---@return nil
function snd.param.set(index_or_name, value) end

---
---Get a parameter's value
---
---@param index_or_name number | string - index of the parameter or its name
---@return number - parameter's current value.
function snd.param.get(index_or_name) end

---
---Get the number of parameters available
---
---@return number - number of parameters
function snd.param.count() end

---
---Get whether the parameter container is empty
---
---@return boolean - whether there are no parameters available
function snd.param.empty() end

---
---Add an integer parameter (step-wise by whole numbers)
---
---@param name string - name of the parameter to create
---@param min number - minimum value of the parameter
---@param max number - maximum value of the parameter
---@param default_val number - default value to set the number
---@return nil
function snd.param.add_int(name, min, max, default_val) end

---
---Add a floating point parameter (numbers with decimal point)
---
---@param name string - name of the parameter to create
---@param min number - minimum value of the parameter
---@param max number - maximum value of the parameter
---@param step number - how fine-grained to increment values by
---@param default_val number - default value to set the number
---@return nil
function snd.param.add_float(name, min, max, step, default_val) end

---
---Add a checkbox parameter (true or false boolean value)
---
---@param name string - name of the parameter to create
---@param default_val boolean - default value to set the parameter
---@return nil
---
function snd.param.add_checkbox(name, min, max, step, default_val) end

---
---Add a dropdown menu of text options
---
---@param name string - name of the parameter to create
---@param options string[] - options in dropdown menu to select
---@param default_val number - index of the default option - ranging from 1
---                            until total options count
---@return nil
function snd.param.add_options(name, options, default_val) end


---------- marker namespace ---------------------------------------------------

---
---Get the number of markers
---
---@return number - number of markers
function snd.marker.count() end

---
---Get whether marker container is empty (zero markers available)
---
---@return boolean - whether marker container is empty
function snd.marker.empty() end

---
---Get marker info
---
---@param index number - the index of the marker
---@return {name: string, seconds: number}
function snd.marker.get(index) end


---------- preset namespace ---------------------------------------------------


---
---Add a preset to the current track
---
---@param name string - name of the preset
---@param values number[] - volume levels for each channel, make sure the
---                         length is equal to the number of channels of track
function snd.preset.add(name, values) end

---
---Apply a preset's volume levels to the current track
---
---@param index_or_name number|string - index or name of preset (indices begin
---                                     begin at 1)
---@param transition_seconds? number - time to transition in seconds
---@return nil
function snd.preset.apply(index_or_name, transition_seconds) end

---
---Get a preset's volume values. Check the number of presets with
---`snd.preset.count()` before indexing to ensure index is in range.
---
---@param index_or_name number|string - index or name of preset (indices begin
---                                     at 1)
---@return number[] - volume values for preset
function snd.preset.get(index_or_name) end

---
---Get number of presets
---
---@return number - number of presets for current track
function snd.preset.count() end

---
---Get whether preset container is empty (zero presets)
---
---@return boolean - whether preset container is empty.
function snd.preset.empty() end
