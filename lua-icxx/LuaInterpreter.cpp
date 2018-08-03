/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


/* Requires Lua 5.1 */

#include "LuaInterpreter.h"
#include "LuaFuncRef.h"
#include "LuaTableRef.h"

#include <stdexcept>
extern "C" {
#include <lua/lualib.h>
}

using namespace std;


LuaInterpreter::LuaInterpreter() : mLua(lua_open()), mOwner(true)
{
    if (mLua == NULL)
        throw runtime_error("Could not initialize LUA interpreter");
    luaL_openlibs(mLua);
}


// Call whatever is in top section of stack (function plus arguments)
// and return the result as a LuaResult instance
inline LuaTempResult 
getResult(lua_State* lua, int stackTop, const std::string& errMsg) 
{
    const int nargs = 0, errFnIdx = 0;
    const int errCall = lua_pcall(lua, nargs, LUA_MULTRET, errFnIdx);
    return LuaTempResult( lua, stackTop, errCall, errMsg );
}


LuaTempResult 
LuaInterpreter::chunkFromString( const std::string& script )
{
    const int stackTop = lua_gettop(mLua);
    assert( lua_checkstack(mLua, 1) ); 
    const int err = luaL_loadstring(mLua, script.c_str());
    return LuaTempResult( mLua, stackTop, err, "Syntax error in" );
}


LuaTempResult 
LuaInterpreter::chunkFromFile( const std::string& filename )
{
    const int stackTop = lua_gettop(mLua);
    assert( lua_checkstack(mLua, 1) ); 
    const int err = luaL_loadfile(mLua, filename.c_str());
    return LuaTempResult(mLua, stackTop, err, "Read error");
}


LuaTempResult 
LuaInterpreter::doFileCommon( const std::string& filename, const LuaTableRef * globalEnv)
{
    const int stackTop = lua_gettop(mLua);

    assert( lua_checkstack(mLua, 1) ); 
    const int err = luaL_loadfile(mLua, filename.c_str());
    if (err != LUA_ERR_NONE)
        return LuaTempResult(mLua, stackTop, err, "Read error");

    if ( globalEnv != NULL )
    {
        globalEnv->pushObj();
        const int ok = lua_setfenv(mLua, -2);
        assert(ok == 1);
    }

    return getResult(mLua, stackTop, "Logic error in");
}


LuaTempResult 
LuaInterpreter::doStringCommon( const std::string& script, const LuaTableRef * globalEnv )
{
    const int stackTop = lua_gettop(mLua);

    assert( lua_checkstack(mLua, 1) ); 
    const int err = luaL_loadstring(mLua, script.c_str());
    if (err != LUA_ERR_NONE)
        return LuaTempResult( mLua, stackTop, err, "Syntax error in" );

    if ( globalEnv != NULL )
    {
        globalEnv->pushObj();
        const int ok = lua_setfenv(mLua, -2);
        assert(ok == 1);
    }

    return getResult(mLua, stackTop, "Logic error in");
}


LuaInterpreter::~LuaInterpreter()
{
    if (mOwner)
        lua_close(mLua);
}


LuaTempResult 
LuaInterpreter::openDynLib( const std::string& libPath, const std::string& entryPoint )
{
    LuaFuncRef loadlib = eval("package.loadlib");
    const int prevStackTop = lua_gettop(mLua);
    string errMsg;
    const int errCode = tryOpenDynLib(loadlib, libPath, entryPoint, errMsg);
    assert( prevStackTop == lua_gettop(mLua)) ;

    if ( errCode != LUA_ERR_NONE )
    {
        assert( lua_checkstack(mLua, 1) ); 
        lua_pushstring(mLua, errMsg.c_str());
    }
    return LuaTempResult(mLua, prevStackTop, errCode, "File error");
}


LuaErrCode
LuaInterpreter::tryOpenDynLib( 
    const LuaFuncRef& loadlib, const std::string& libPath, const std::string& entryPoint, 
    string& errMsg )
{
    LuaErrCode errCode = LUA_ERR_NONE;
    LuaTempResult res = loadlib(libPath, entryPoint);
    if ( res[1].isNil() )
    {
        string errOper = res[2]; //.getAs<string>();
        string errPrefix = "Error opening";
        errCode = LUA_ERR_FILE;
        if ( errOper != "open" )
        {
            errCode = LUA_ERR_RUN;
            if ( errOper == "init" )
                errPrefix = "Error initializing";
            else
                errPrefix = "Unknown error in loading";
        }
        errMsg = errPrefix + " library " + libPath + ": " + res[1].getAs<string>();
    }

    else
    {
        // lib and its entry point function found; now call it:
        LuaFuncRef libEntryFunc = res[1];
        LuaTempResult loadRes = libEntryFunc();
        if ( ! loadRes.ok() )
        {
            errCode = (LuaErrCode) loadRes.getErrCode();
            errMsg = "Error running entry point function " + entryPoint + "() for library " 
                + libPath + ": " + loadRes.getErrMsg();
        }
    }

    return errCode;
}


