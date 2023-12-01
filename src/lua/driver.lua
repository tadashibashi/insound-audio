---User can provide a lua script that will be driven inside of a sandbox
---environment as defined in this file.

---Environment for the sandbox
env = {}
function reset_env()
    env = {
        coroutine = coroutine,
        math = math,
        string = string,
        utf8 = utf8,
        table = table,
        _VERSION = _VERSION,
        collectgarbage = collectgarbage,
        error = error,
        getmetatable = getmetatable,
        ipairs = ipairs,
        next = next,
        pairs = pairs,
        pcall = pcall,
        print = print,
        rawequal = rawequal,
        rawget = rawget,
        rawset = rawset,
        select = select,
        setmetatable = setmetatable,
        tonumber = tonumber,
        tostring = tostring,
        type = type,
        warn = warn,
        xpcall = xpcall,
    }
end

---Load a script and its sandbox environment
---@param untrusted_code string
function load_script(untrusted_code)
    local untrusted_func, message = load(untrusted_code, nil, 't', env)

    if not untrusted_func then
        error(message)
    end

    local res, err = pcall(untrusted_func)

    if not res then
        error(err)
    end

    return res
end

--Callback that fires immediately after script load
local function on_init()
    if env.on_init ~= nil then
        env.on_init()
    end
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
local function on_trackend()
    if env.on_track_end ~= nil then
        env.on_track_end()
    end
end

local function on_paramset(name, value)
    if env.on_paramset ~= nil then
        env.on_paramset(name, value)
    end
end

local event_table = {
    [Event.Init] = on_init,
    [Event.Update] = on_update,
    [Event.SyncPoint] = on_syncpoint,
    [Event.Load] = on_load,
    [Event.Unload] = on_unload,
    [Event.TrackEnd] = on_trackend,
    [Event.ParamSet] = on_paramset,
}

function process_event(ev_type, ...)
    event_table[ev_type](...)
end
