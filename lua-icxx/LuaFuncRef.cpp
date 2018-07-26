/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#include "LuaFuncRef.h"
#include "LuaTableRef.h"
#include "LuaStackCleaner.h"


bool
LuaFuncRef::setEnv( const LuaTableRef& envTable )
{
    LuaStackCleaner s(mLua);
    pushObj();
    envTable.pushObj();
    const bool ok = lua_setfenv(mLua, -2) != 0;
    assert( s.willPop(1) );
    return ok;
}


LuaTempResult
LuaFuncRef::getEnv()
const 
{
    const int preStack = lua_gettop(mLua);
    pushObj();
    assert( lua_checkstack(mLua, 1) ); 
    lua_getfenv(mLua, -1);
    return LuaTempResult(mLua, preStack);
}


void 
LuaBoundMethRef::resetRef( const LuaObjRef & obj, const LuaFuncRef & fn )
{
    mObjRef = obj;
    mMethRef = fn; 
}


LuaBoundMethRef 
LuaClassObjRef::getBoundMethod( const std::string& name ) const
{
    const int prevStackTop = lua_gettop(mLua);
    pushObj(); // push "table" (the object)
    assert( lua_checkstack(mLua, 1) ); 
    lua_getfield(mLua, -1, name.c_str());
    return LuaTempResult(mLua, prevStackTop);
}


