---User can provide a lua script that will be driven inside of a sandbox
---environment as defined in this file.

---Environment for the sandbox
env = {
    math = math,
    require = require,
    print = print,
}

---Loads a script and its sandbox environment
---@param untrusted_code string
local function load_script(untrusted_code)
    local untrusted_func, message = load(untrusted_code, nil, 't', env)



    if not untrusted_func then return false, message end

    local res = pcall(untrusted_func)

    if env.on_init ~= nil then
        env.on_init()
    end

    return res
end

---Callback that fires when the sound bank has been loaded
local function on_load()
    if env.on_load ~= nil then
        env.on_load()
    end
end

---Callback that fires when the sound bank has been unloaded
local function on_unload()
    if env.on_unload ~= nil then
        env.on_unload()
    end
end

---Callback that fires on each update tick. Occurs ~60+ Hz
---`seconds` is the current time position of the track.
---@param seconds number
local function on_update(seconds)
    if env.on_update ~= nil then
        env.on_update(seconds)
    end
end

---Callback that fires when a sync point has been reached
---Calls function `on_syncpoint` only if user provided it in their script
---
---@param label string
---@param seconds number
local function on_syncpoint(label, seconds)
    if env.on_syncpoint ~= nil then
        env.on_syncpoint(label, seconds)
    end
end

---Callback that fires when a track has reached its maximum time
---Calls function `on_track_end` only if user provided it in their script
local function on_track_end()
    if env.on_track_end ~= nil then
        env.on_track_end()
    end
end
