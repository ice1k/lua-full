/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/

#pragma  once

#include <cassert>
#include <lua/lua.hpp>

/** Checker class to verify that stack size unchanged. It tracks the 
    stack top when constructed, and asserts that it is the same when 
    destructed. Use as 

    \code
        {
            LuaStackSizeChecker checker(L);
            ... use Lua C API that manipulates stack...
        } // checker destroyed, asserts no change in stack size

        // or if you don't want to use C++ block:
        LuaStackSizeChecker checker(L);
        ... use Lua C API that manipulates stack...
        checker.checkNow();
    \endcode

    Note that the check only occurs in a Debug build (if NDEBUG not defined. 
    */
class // LUA_ICXX_CPP_API
LuaStackSizeChecker
{
public:
    /// capture current stack top level
    LuaStackSizeChecker(lua_State* lua)
        : mLua(lua), mStackTop( lua_gettop(mLua) ) { assert(mLua); }
    /// do the check upon destruction
    inline ~LuaStackSizeChecker() { checkNow(); }
    /// returns true if stack top currently same as at construction
    inline void checkNow() const { assert( mStackTop == lua_gettop(mLua) ); }

private:
    lua_State * mLua;
    const int mStackTop;
};
