---User can provide a lua script that will be driven inside of a sandbox
---environment as defined in this file.

---Environment for the sandbox
env = {}

test_val = 12

function table_length(t)
    local count = 0
    for _ in pairs(t) do count = count + 1 end
    return count
end

local function to_json_string_array(arr)
    local res = "["
    for i, v in ipairs(arr) do
        res = res .. '"'..v..'"'
        if i < #arr then
            res = res .. ","
        end
    end
    res = res .. "]"

    return res
end

local function add_param(config)
    if env.track.param.raw_add == nil then
        error("Internal error: missing env.param.raw_add in Lua driver code")
    end
    if config == nil then
        error("parameter config was `nil`")
    end

    local type = config.type or error("parameter config is missing `type`")
    local name = config.name or error("parameter config is missing `name`")
    local default_value = config.default_value or 0
    local json = '{"name": "'..name..'", "type": "'..type..
        '", "defaultValue": '..default_value

    if type == "strings" then
        json = json..',"strings": '..to_json_string_array(config.strings or {})..""
    elseif type == "float" or type == "int" then
        if config.number then
            json = json..',"number": {'
                ..'"min": '..(config.number.min == nil and 'null' or config.number.min)..','
                ..'"max": '..(config.number.max == nil and 'null' or config.number.max)..','
            if config.number.step ~= nil and type == "float" then
                json = json..'"step": '..config.number.step
            elseif type == "int" then
                json = json..'"step": 1'
            else
                json = json..'"step": null'
            end
            json = json..'}'
        end
    end

    json = json..'}'

    env.track.param.raw_add(json)
end

local function add_strings(name, strings, default_value)
    default_value = default_value or 0
    strings = strings or {}
    if not name or #name == 0 then
        error("cannot create strings parameter: missing `name`")
    end

    add_param({
        name=name,
        type="strings",
        strings=strings,
        default_value=default_value
    })
end


function reset_env()
    local printFunc = function(...)
        local args = {...}
        if env.raw_print == nil then return end

        local res = ""
        for i, v in ipairs(args) do
            res = res..tostring(v).."\t"
        end

        env.raw_print(1, "LOG", res);
    end

    local warnFunc = function(...)
        if env.raw_print == nil then return end
        local args = {...}
        local res = ""
        for i, v in ipairs(args) do
            res = res..tostring(v).."\t"
        end

        env.raw_print(2, "WARN", res)
    end

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
        print = printFunc,
        rawequal = rawequal,
        rawget = rawget,
        rawset = rawset,
        select = select,
        setmetatable = setmetatable,
        tonumber = tonumber,
        tostring = tostring,
        type = type,
        warn = warnFunc,
        xpcall = xpcall,

        track = {
            param = {
                add = add_param,
                add_strings=add_strings
            },
        },
    }

end

---Load a script and its sandbox environment
---@param untrusted_code string
function load_script(untrusted_code)
    local untrusted_func <const>, message <const> =
        load(untrusted_code, nil, 't', env)

    if not untrusted_func then
        error(message)
        return false
    end

    local res <const>, err <const> = pcall(untrusted_func)

    if not res then
        error(err)
        return false
    end

    return res
end

function execute_string(untrusted_code)
    local untrusted_func, message = load(untrusted_code, nil, 't', env)

    if not untrusted_func then
        error(message)
        return false
    end

    local did_suceed, res = pcall(untrusted_func)

    if not did_succeed then -- function had an error
        error(res)
        return false
    end

    return tostring(res)
end

--Callback that fires immediately after script load
local function on_init()
    if env.on_init ~= nil then
        env.on_init()
    end
end

---Callback that fires when the sound bank has been loaded
local function on_load()
    if env.on_ready ~= nil then
        env.on_ready()
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
---@param delta number milliseconds between this call to update and the last
---@param totalTime number milliseconds from script load until this update
local function on_update(delta, totalTime)
    if env.on_update ~= nil then
        env.on_update(delta, totalTime)
    end
end

---Callback that fires when a sync point has been reached
---Calls function `on_syncpoint` only if user provided it in their script
---
---@param label string
---@param seconds number
local function on_syncpoint(label, seconds)
    if env.on_marker ~= nil then
        env.on_marker(label, seconds)
    end
end

---Callback that fires when a track has reached its maximum time
---Calls function `on_track_end` only if user provided it in their script
local function on_trackend()
    if env.on_track_end ~= nil then
        env.on_track_end()
    end
end

local function on_param(name, value)
    if env.on_param ~= nil then
        env.on_param(name, value)
    end
end

-- move later?
local Event = {
    Init=0,
    Update=1,
    SyncPoint=2,
    Load=3,
    Unload=4,
    TrackEnd=5,
    ParamSet=6
}

local EV_TABLE <const> = {
    [Event.Init] = on_init,
    [Event.Update] = on_update,
    [Event.SyncPoint] = on_syncpoint,
    [Event.Load] = on_load,
    [Event.Unload] = on_unload,
    [Event.TrackEnd] = on_trackend,
    [Event.ParamSet] = on_param,
}

function process_event(ev_type, ...)
    local callback <const> = EV_TABLE[ev_type]
    if callback ~= nil then
        callback(...)
    else
        error("Internal error: event type " .. ev_type ..
            " is not recognized")
    end
end
