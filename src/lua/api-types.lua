---@metatracknd

track = {
    preset={},
    marker={},
}

---
---Starts playing the track - equivalent to tracknd.paused(false)`
---
---@return nil
function track.play() end

---
---Pauses the track - equivalent to `trackd(true)`
---@return nil
function track.pause() end

---
---Pause or unpause the track. If no value is passed to `pause`, it will
---instead return the current paused status of the track.
---
---@param pause? boolean - whether to pause track
---
---@return boolean - whether or not track is paused.
function track.paused(pause) end

---
---Set/get the current playhead position
---
---@param seconds? number - seconds to seek to, if provided
---
---@return number - number of seconds track playhead is at
function track.position(seconds) end

---
---Set the main master volume. If no value is passed to `volume`, it will
---instead return the current main volume level.
---
---@param volume? number - volume level (0=off, 1=100%, max=2)
---@return number - current main volume level
function track.main_volume(volume) end

---
---Set an individual channel's volume. If not value is passed to `volume, it
---will instead return the current volume level for channel.
---
---@param channel number - channel to set volume of, ranging from 1 to
---                        max channels
---@param volume? number - volume level (0=off, 1=100%, max=2)
---@return number - current volume level of `channel`
function track.channel_volume(channel, volume) end

---
---Get the number of channels on the current track
---
---@return number - number of channels on the current track.
function track.channel_count() end


---------- marker namespace ---------------------------------------------------

---
---Get the number of markers
---
---@return number - number of markers
function track.marker.count() end

---
---Get whether marker container is empty (zero markers available)
---
---@return boolean - whether marker container is empty
function track.marker.empty() end

---
---Get marker info
---
---@param index number - the index of the marker
---@return {name: string, seconds: number}
function track.marker.get(index) end


---------- preset namespace ---------------------------------------------------


---
---Add a preset to the current track
---
---@param name string - name of the preset
---@param values number[] - volume levels for each channel, make sure the
---                         length is equal to the number of channels of track
function trackd.preset.add(name, values) end

---
---Apply a preset's volume levels to the current track
---
---@param index_or_name number|string - index or name of preset (indices begin
---                                     begin at 1)
---@param transition_seconds? number - time to transition in seconds
---@return nil
function trackd.preset.apply(index_or_name, transition_seconds) end

---
---Get a preset's volume values. Check the number of presets with
---`trackd.preset.count()` before indexing to ensure index is in range.
---
---@param index_or_name number|string - index or name of preset (indices begin
---                                     at 1)
---@return number[] - volume values for preset
function trackd.preset.get(index_or_name) end

---
---Get number of presets
---
---@return number - number of presets for current track
function trackd.preset.count() end

---
---Get whether preset container is empty (zero presets)
---
---@return boolean - whether preset container is empty.
function trackd.preset.empty() end
