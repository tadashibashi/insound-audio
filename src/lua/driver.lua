---User can provide a lua script that will be driven inside of a sandbox
---environment as defined in this file.

---Environment for the sandbox
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

    if env.on_init ~= nil then
        env.on_init()
    end

    return res
end

---Callback that fires when the sound bank has been loaded
function on_load()
    if env.on_load ~= nil then
        env.on_load()
    end
end

---Callback that fires when the sound bank has been unloaded
function on_unload()
    if env.on_unload ~= nil then
        env.on_unload()
    end
end

---Callback that fires on each update tick. Occurs ~60+ Hz
---`seconds` is the current time position of the track.
---@param seconds number
function on_update(seconds)
    if env.on_update ~= nil then
        env.on_update(seconds)
    end
end

---Callback that fires when a sync point has been reached
---Calls function `on_syncpoint` only if user provided it in their script
---
---@param label string
---@param seconds number
function on_syncpoint(label, seconds)
    if env.on_syncpoint ~= nil then
        env.on_syncpoint(label, seconds)
    end
end

---Callback that fires when a track has reached its maximum time
---Calls function `on_track_end` only if user provided it in their script
function on_track_end()
    if env.on_track_end ~= nil then
        env.on_track_end()
    end
end
