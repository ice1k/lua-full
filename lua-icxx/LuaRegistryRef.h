/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#ifndef LUA_ICXX_REGREF_INCLUDED
#define LUA_ICXX_REGREF_INCLUDED

#include <cassert>

#include <src/lua.hpp>


/*  Represent a Lua registry reference for a Lua object.  
    The object can be pushed onto the lua stack via pushObj(), or its
    registry key obtained by converting to int. It is automatically 
    a reference to Lua nil if stack pos is invalid.

    WARNING: LuaRegRef is not copyable
    */
class
LuaRegRef
{
private:
    static int getRef(lua_State* lua, int stackPos) 
    { 
        assert(lua);
        if (stackPos > 0 && stackPos > lua_gettop(lua))
            return LUA_NOREF;
        if (stackPos < 0 && -stackPos > lua_gettop(lua))
            return LUA_NOREF;
        if (stackPos == 0)
            return LUA_NOREF;

        assert( lua_checkstack(lua, 1) ); 
        lua_pushvalue(lua, stackPos);
        return luaL_ref(lua, LUA_REGISTRYINDEX);
    }

    static int copyRef(const LuaRegRef& rhs) 
    { 
        if (rhs.isNil())
            return LUA_NOREF;

        assert(rhs.mLua);
        rhs.pushObj();
        return luaL_ref(rhs.mLua, LUA_REGISTRYINDEX);
    }

public:
    // Nil reference
    LuaRegRef(): mLua(NULL), mRegKey(LUA_NOREF) {}
    // ref to top of stack by default, or stackPos
    LuaRegRef(lua_State* lua, int stackPos = -1): mLua(lua), mRegKey( getRef(lua, stackPos) ) {}
    // remove reference to Lua obj
    ~LuaRegRef() { if (mLua) luaL_unref(mLua, LUA_REGISTRYINDEX, mRegKey); }

    // create from existing
    LuaRegRef(const LuaRegRef& rhs): mLua(rhs.mLua), mRegKey( copyRef(rhs) ) {}
    void operator=(const LuaRegRef& rhs) { mLua = rhs.mLua; mRegKey = copyRef(rhs); }

    // Reset the reference to a different Lua object (the one at top of 
    // stack if stackPos not given)
    void reset( lua_State* lua, int stackPos = -1 ) 
    {
        if ( mLua )
            luaL_unref(mLua, LUA_REGISTRYINDEX, mRegKey);
        else
            mLua = lua;
        assert(mLua);
        mRegKey = getRef(mLua, stackPos);
    }

    // make this ref "point to nil"
    void setNil() 
    {
        assert(mLua);
        luaL_unref(mLua, LUA_REGISTRYINDEX, mRegKey);
        mRegKey = LUA_REFNIL;
    }

    // true if this is a reference to Lua NIL
    bool isNil() const { return mRegKey == LUA_NOREF || mRegKey == LUA_REFNIL; }
    
    // get the raw reference key used by Lua
    int getRegKey() const { return mRegKey; }

    // compare two registry refs for equality (ie same registry id)
    bool operator==(const LuaRegRef& rhs) const { return mRegKey == rhs.mRegKey; }
    bool operator!=(const LuaRegRef& rhs) const { return mRegKey != rhs.mRegKey; }

    // get the type of what is referenced
    int getTypeID() const
    { 
        assert(mLua);
        if ( isNil() ) 
            return LUA_TNIL;

        pushObj();
        const int refType = lua_type(mLua, -1);
        lua_pop(mLua, 1);
        return refType;
    }

    // push referenced object onto stack, for use by other lua_icxx functions/methods
    void pushObj() const 
    { 
        assert(mLua);
        lua_rawgeti(mLua, LUA_REGISTRYINDEX, mRegKey); 
    }

private:
    lua_State * mLua;
    int mRegKey;
};


#endif // LUA_ICXX_REGREF_INCLUDED
