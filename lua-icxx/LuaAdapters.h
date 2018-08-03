/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#ifndef LUA_ICXX_ADAPTERS_INCLUDED
#define LUA_ICXX_ADAPTERS_INCLUDED


#include <lua/lua.hpp>
#include <string>


// Use a template class so that get/put of number/string can be based on 
// type specified by caller
template <typename TT>
inline TT getObjFromStack(lua_State* lua, int stackPos = -1);

static const int LUA_UNACCEPTABLE_STACK_POS = 0;

// use specializations for individual types for getting values from stack (but no pop)
#define LUA_C_TYPE_ADAPTER_TO(TT, FN) \
    template <> \
    inline TT \
    getObjFromStack<TT>(lua_State* lua, int stackPos) {\
        assert(lua); \
        return stackPos == LUA_UNACCEPTABLE_STACK_POS ? TT(0) : (TT)FN(lua, stackPos);\
    }

#pragma warning(push)
#pragma warning(disable:4800)
LUA_C_TYPE_ADAPTER_TO(bool,   lua_toboolean)
#pragma warning(pop)
LUA_C_TYPE_ADAPTER_TO(int,    lua_tointeger)
LUA_C_TYPE_ADAPTER_TO(long,   lua_tointeger)
LUA_C_TYPE_ADAPTER_TO(float,  lua_tonumber)
LUA_C_TYPE_ADAPTER_TO(double, lua_tonumber)
// char* / string need special handling
template <> inline 
const char* 
getObjFromStack<const char*>(lua_State* lua, int stackPos) 
{
    assert(lua);
    if (stackPos == LUA_UNACCEPTABLE_STACK_POS) return "";
    const char* str = lua_tostring(lua, stackPos);
    return ( str == NULL ? "" : str );
}
template <> inline 
std::string 
getObjFromStack<std::string>(lua_State* lua, int stackPos) 
{
    return getObjFromStack<const char*>(lua, stackPos);
}


inline void pushValToStack(lua_State* L, bool   value) { assert( lua_checkstack(L, 1) ); lua_pushboolean(L, value); }
inline void pushValToStack(lua_State* L, int    value) { assert( lua_checkstack(L, 1) ); lua_pushinteger(L, value); }
inline void pushValToStack(lua_State* L, long   value) { assert( lua_checkstack(L, 1) ); lua_pushinteger(L, value); }
inline void pushValToStack(lua_State* L, float  value) { assert( lua_checkstack(L, 1) ); lua_pushnumber (L, value); }
inline void pushValToStack(lua_State* L, double value) { assert( lua_checkstack(L, 1) ); lua_pushnumber (L, value); }
inline void pushValToStack(lua_State* L, const char* str)         { assert( lua_checkstack(L, 1) ); lua_pushstring (L, str); }
inline void pushValToStack(lua_State* L, const std::string& str)  { assert( lua_checkstack(L, 1) ); lua_pushstring (L, str.c_str()); }


#endif // LUA_ICXX_ADAPTERS_INCLUDED
