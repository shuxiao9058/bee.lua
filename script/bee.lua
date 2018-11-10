local sp = require 'bee.subprocess'
local fs = require 'bee.filesystem'

local function fork(script, args, options)
    -- TODO
    local function getexe()
        local i = 0
        while arg[i] ~= nil do
            i = i - 1
        end
        return arg[i + 1]
    end
    local init = {
        getexe(),
        '-E',
        '-e', ('package.path=[[%s]]'):format(package.path)
        '-e', ('package.cpath=[[%s]]'):format(package.cpath)
        script,
        args,
        cwd = fs.current_path(),
    }
    if options then
        for k, v in pairs(options) do
            if type(k) == 'string' then
                init[k] = v
            end
        end
    end
    return sp.swapn(init)
end

return {
    fork = fork,
}