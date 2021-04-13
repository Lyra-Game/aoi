#if __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "grid.h"

#ifndef LUA_LIB_API
#define LUA_LIB_API extern
#endif

static IntList out_put;

#define MT_NAME ("_grid_metatable")

static inline float
getfield(lua_State *L, const char *f) {
    if (lua_getfield(L, -1, f) != LUA_TNUMBER) {
        return luaL_error(L, "invalid type %s", f);
    }
    float v = lua_tonumber(L, -1);
    lua_pop(L, 1);
    return v;
}

static int
gd_insert(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    int handle = luaL_checkinteger(L, 2);
    float x = luaL_checknumber(L, 3);
    float y = luaL_checknumber(L, 4);
    grid_insert(grid, handle, x, y);
    return 0;
}

static int
gd_remove(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    int handle = luaL_checkinteger(L, 2);
    float x = luaL_checknumber(L, 3);
    float y = luaL_checknumber(L, 4);
    grid_remove(grid, handle, x, y);
    return 0;
}

static int
gd_move(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    int handle = luaL_checkinteger(L, 2);
    float px = luaL_checknumber(L, 3);
    float py = luaL_checknumber(L, 4);
    float x = luaL_checknumber(L, 5);
    float y = luaL_checknumber(L, 6);
    grid_move(grid, handle, px, py, x, y);
    return 0;
}

static int
gd_query(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float w = luaL_checknumber(L, 4);
    float h = luaL_checknumber(L, 5);
    grid_query(grid, &out_put, x, y, w, h);
    lua_newtable(L);
    for (int i = 0; i < il_size(&out_put); i++) {
        lua_pushinteger(L, il_get(&out_put, i, 0));
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int
gd_query_cell(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    grid_query_cell(grid, &out_put, x, y);
    lua_newtable(L);
    for (int i = 0; i < il_size(&out_put); i++) {
        lua_pushinteger(L, il_get(&out_put, i, 0));
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int
gd_in_bound(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    lua_pushinteger(L, grid_in_bound(grid, x, y));
    return 1;
}

static int
gd_optimize(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    grid_optimize(grid);
    return 0;
}

static int
gc(lua_State *L) {
    Grid *grid = luaL_checkudata(L, 1, MT_NAME);
    grid_destroy(grid);
    return 0;
}

static int
lmetatable(lua_State *L) {
    if (luaL_newmetatable(L, MT_NAME)) {
        luaL_Reg l[] = {
            { "insert", gd_insert },
            { "remove", gd_remove },
            { "move", gd_move },
            { "query", gd_query },
            { "query_cell", gd_query_cell },
            { "in_bound", gd_in_bound },
            { "optimize", gd_optimize },
            { NULL, NULL }
        };
        luaL_newlib(L, l);
        lua_setfield(L, -2, "__index");

        lua_pushcfunction(L, gc);
        lua_setfield(L, -2, "__gc");
    }
    return 1;
}

static int
lnew(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_settop(L, 1);
    float cell_w = getfield(L, "cell_w");
    float cell_h = getfield(L, "cell_h");
    float l = getfield(L, "l");
    float t = getfield(L, "t");
    float r = getfield(L, "r");
    float b = getfield(L, "b");
    lua_assert(cell_w > 0 && cell_h > 0);
#if LUA_VERSION_NUM == 504
    Grid *grid = lua_newuserdatauv(L, sizeof(Grid), 0);
#else
    Grid *grid = lua_newuserdata(L, sizeof(Grid));
#endif
    grid_create(grid, cell_w, cell_h, l, t, r, b);
    lmetatable(L);
    lua_setmetatable(L, -2);
    return 1;
}

LUA_LIB_API int
luaopen_grid(lua_State* L) {
    il_create(&out_put, 1);
    luaL_checkversion(L);
    luaL_Reg l[] = {
        { "new", lnew },
        { NULL, NULL },
    };
    luaL_newlib(L, l);
    return 1;
}

#if __cplusplus
}
#endif
