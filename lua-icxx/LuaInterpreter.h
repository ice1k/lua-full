/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#ifndef LUA_ICXX_INTERP_INCLUDED
#define LUA_ICXX_INTERP_INCLUDED


// Only tested with Lua 5.1

#include <string>
#include <cassert>
#include <lua/lua.hpp>

#include "LuaTempResult.h"
#include "LuaObjRef.h"
class LuaFuncRef;


/** Lua Interpreter. Can evaluate Lua expressions (eval()), run Lua 
    statements from strings (doString) or files (doFile), get and set global
    variables, and open Lua libraries and extensions. Examples: 

    \code
    Lua lua;

    LuaTempResult res = lua.doFile('someInitScript.lua');
    if ( ! res.ok )
        ... decide what to do: log error, throw exception, etc...

    LuaObjRef obj = lua.eval("someTable[someGlobal].child");
    ...

    // create new table:
    LuaTableRef tt = lua.eval( "{}" ); 
    ...
    \endcode
    */
class
LuaInterpreter
{
public: // construct/destroy

    /** Create a Lua interpreter from a new Lua state. Makes all the builtin Lua 
        libraries (table, string, debug, etc) available to scripts that will be 
        run via this interpreter. Note: memory overhead for this is negligible: 
        the code is already present in memory, it is just the symbols that are 
        made available in scripts.
    */
    LuaInterpreter();

    /** Create a Lua interpreter from an existing Lua state. Used as-is (no 
        libs loaded), and caller retains ownership of state.
        */
    LuaInterpreter(lua_State* lua): mLua(lua), mOwner(false) {}
    ~LuaInterpreter();

public: // methods

    //@{
    /** Execute given string as a Lua expression and return result.
        To run the expression sandboxed, give it a ref to a Lua table, it will be 
        used as the global environment for the expression.
        */
    LuaTempResult eval(const std::string& expr);
    LuaTempResult operator()(const std::string& expr);

    LuaTempResult eval(const std::string& expr, const LuaTableRef& globalEnv);
    LuaTempResult operator()(const std::string& expr, const LuaTableRef& globalEnv);
    //@}

private:
    LuaTempResult doStringCommon( const std::string& script, const LuaTableRef * globalEnv = NULL);
    LuaTempResult doFileCommon( const std::string& script, const LuaTableRef * globalEnv = NULL);

public:

    //@{
    /** Run the given item as a Lua script. If the 
        string contains a return statement, the return values are 
        available in the returned LuaTempResult. Some of the methods 
        take a table used as the global environment of the file/chunk
        (useful to run it sandboxed). 
        */
    LuaTempResult doString(const std::string& script);
    LuaTempResult doFile(const std::string& filename) { return doFileCommon(filename); }
    LuaTempResult doString(const std::string& script, const LuaTableRef& globalEnv);
    LuaTempResult doFile(const std::string& filename, const LuaTableRef& globalEnv);
    //@}

    //@{
    LuaTempResult chunkFromString(const std::string& script);
    LuaTempResult chunkFromFile(const std::string& filename);
    //LuaTempResult loadString(const std::string& script, const LuaTableRef& globalEnv);
    //LuaTempResult loadFile(const std::string& script, const LuaTableRef& globalEnv);
    //@}

    /// set global variable to reference given item, which could be POD or a Lua ref
    template <typename TT> void setGlobal(const std::string& name, const TT& item);
    /// get global variable; this is same as eval(name), but faster
    LuaTempResult getGlobal(const std::string& name);
    /// create a new table (shortcut for eval("{}"))
    LuaTempResult newTable();

    /// load a DLL/SO and run its entryPoint function
    LuaTempResult openDynLib(const std::string& libPath, const std::string& entryPoint);
    /// do same as require(moduleName) from Lua; there is currently no way to do this sandboxed
    LuaTempResult require(const std::string& moduleName);

public: // methods to use Lua C API directly

    //@{
    /// get the lua_State for this LuaInterpreter
    lua_State* getLuaState() const { return mLua; }
    operator lua_State*() const { return mLua; }
    //@}

private: // methods

    LuaErrCode tryOpenDynLib( 
        const LuaFuncRef&, const std::string&, const std::string&, std::string& errMsg );

private: // data

    lua_State * mLua;
    bool mOwner;
};


inline LuaTempResult 
LuaInterpreter::eval( const std::string& expr )
{
    return doStringCommon("return " + expr);
}


inline LuaTempResult 
LuaInterpreter::eval( const std::string& expr, const LuaTableRef& globalEnv )
{
    return doStringCommon("return " + expr, & globalEnv);
}


inline LuaTempResult 
LuaInterpreter::operator()( const std::string& expr )
{
    return eval(expr);
}


inline LuaTempResult 
LuaInterpreter::operator()( const std::string& expr, const LuaTableRef& globalEnv )
{
    return eval(expr, globalEnv);
}


inline LuaTempResult 
LuaInterpreter::doString( const std::string& script )
{
    return doStringCommon(script);
}


inline LuaTempResult 
LuaInterpreter::doString( const std::string& script, const LuaTableRef& globalEnv )
{
    return doStringCommon(script, & globalEnv);
}


inline LuaTempResult 
LuaInterpreter::doFile( const std::string& filename, const LuaTableRef& globalEnv )
{
    return doFileCommon(filename, & globalEnv);
}


inline LuaTempResult 
LuaInterpreter::require( const std::string& moduleName )
{
    return eval( "require('" + moduleName + "')" );
}


template <typename TT>
inline void 
LuaInterpreter::setGlobal( const std::string& name, const TT& item )
{
    pushValToStack(mLua, item);
    lua_setglobal(mLua, name.c_str());
}


inline LuaTempResult 
LuaInterpreter::getGlobal( const std::string& name )
{
    const int stackTop = lua_gettop(mLua);
    assert( lua_checkstack(mLua, 1) ); 
    lua_getglobal(mLua, name.c_str());
    return LuaTempResult(mLua, stackTop);
}


inline LuaTempResult 
LuaInterpreter::newTable()
{
    const int stackTop = lua_gettop(mLua);
    assert( lua_checkstack(mLua, 1) ); 
    lua_newtable(mLua);
    return LuaTempResult(mLua, stackTop);
}

#endif // LUA_ICXX_INTERP_INCLUDED

