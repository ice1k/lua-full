/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/

#pragma once

#include <lua/lua.hpp>


/** Maintains Lua stack: Automatically resets, when destroyed, the stack to where 
    it was at construction. 
    Can also verify how many stack items would be popped if it were destroyed now. 
    */
class
LuaStackCleaner
{
public:
    /// Capture current stack level on L
    LuaStackCleaner(lua_State* L): mLua(L), mStackTop(lua_gettop(L)) { assert(L); }
    /// Restore stack to level at construction
    ~LuaStackCleaner() { lua_settop(mLua, mStackTop); }
    /// returns true if it would pop n items off the Lua stack
    bool willPop(int n) const { return n == (lua_gettop(mLua) - mStackTop); }
    /// get stack level at construction
    int getStackLevelOrig() const { return mStackTop; }

private:
    lua_State* mLua;
    int mStackTop;
};


