/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#ifndef LUA_ICXX_CALL_STATUS_INCLUDED
#define LUA_ICXX_CALL_STATUS_INCLUDED

#include <src/lua.hpp>


/// Enumeration of all the Lua error codes
enum LuaErrCode 
{
    LUA_ERR_NONE   = 0,             ///< NO error
    LUA_ERR_SYNTAX = LUA_ERRSYNTAX, ///< syntax error 
    LUA_ERR_RUN    = LUA_ERRRUN,    ///< runtime error (nil function call, etc)
    LUA_ERR_MEM    = LUA_ERRMEM,    ///< out of memory error
    LUA_ERR_ERR    = LUA_ERRERR,    ///< error in error handler -- never used
    LUA_ERR_FILE   = LUA_ERRFILE,   ///< luaL_loadfile (and luaL_dofile) can't open or read file
    LUA_NO_LUA     = 100,           ///< no Lua interpreter available
};

#endif // LUA_ICXX_CALL_STATUS_INCLUDED