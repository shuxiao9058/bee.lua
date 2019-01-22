local lm = require 'luamake'

lm.rootdir = '3rd/lua/'

lm:shared_library 'lua54' {
    sources = {
        "src/*.c",
        "!src/lua.c",
        "!src/luac.c",
        "utf8/utf8_crt.c",
    },
    defines = {
        "LUA_BUILD_AS_DLL",
        "LUAI_MAXCCALLS=200"
    }
}
lm:executable 'lua' {
    deps = "lua54",
    sources = {
        "utf8/utf8_lua.c",
    }
}

lm.rootdir = ''

if lm.plat == 'msvc' then
    lm:build "embed_clean" {
        "$luamake", "lua", "project/embed.lua", "bootstrap/main.lua", "bee/nonstd/embed_detail.h"
    }
    lm:build "embed_make" {
        "$luamake", "lua", "project/embed.lua", "bootstrap/main.lua", "bee/nonstd/embed_detail.h", "binding/lua_embed.cpp",
        deps = "embed_clean"
    }
end

lm:shared_library 'bee' {
    deps = {
        "lua54",
        (lm.plat == 'msvc') and "embed_make",
    },
    includes = {
        "3rd/lua/src",
        "3rd/lua-seri",
        "3rd/incbin",
        "."
    },
    defines = {
        "_WIN32_WINNT=0x0601",
        "BEE_EXPORTS",
        "_CRT_SECURE_NO_WARNINGS"
    },
    sources = {
        "3rd/lua-seri/*.c",
        "bee/*.cpp",
        "binding/*.cpp",
        "!bee/*_osx.cpp",
        "!bee/*_linux.cpp",
        "!bee/*_posix.cpp",
        "!binding/lua_posixfs.cpp",
    },
    links = {
        "advapi32",
        "ws2_32",
        "version",
        "stdc++fs",
        "stdc++"
    }
}

lm:executable 'bootstrap' {
    deps = "lua54",
    includes = {
        "3rd/lua/src"
    },
    sources = {
        "bootstrap/*.cpp",
    },
}

if lm.plat == 'msvc' then
    lm:build "copy_script" {
        "cmd.exe", "/c", "copy", "/Y", "bootstrap\\main.lua", lm.bindir:gsub('/', '\\') .. "\\main.lua"
    }
else
    lm:build "copy_script" {
        "cp", "bootstrap/main.lua", "$bin/main.lua"
    }
end

lm:build "test" {
    "$bin/bootstrap.exe", "test/test.lua",
    deps = { "bootstrap", "copy_script", "bee" },
}