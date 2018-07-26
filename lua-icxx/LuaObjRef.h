/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once

#ifndef LUA_ICXX_OBJREF_INCLUDED
#define LUA_ICXX_OBJREF_INCLUDED

#include <string>

#include "LuaRegistryRef.h"
#include "LuaTempResult.h"
#include "LuaStackCleaner.h"

class LuaTableRef; 
class LuaFuncRef; 


/** Represent a reference to a Lua object. This can be any object: function, table, 
    number, string, coroutine, user data, etc. 

    \todo: add the constructor, operator= and resetRef taking 1 arg: const LuaObjRef& rhs
    */
class
LuaObjRef
{
public:
    /// reference to Lua nil
    explicit LuaObjRef( const std::string& id = "nil" );

    //@{
    /// Reference a Lua object on the stack
    LuaObjRef( const LuaTempResult& );
    LuaObjRef( const LuaTempResult::Item& );
    LuaObjRef( lua_State*, int stackPos );
    //@}
    virtual ~LuaObjRef() {}

public: 

    //@{
    /// Change what this object reference points to
    void resetRef( const LuaTempResult& result ) { resetRef(result[1]); }
    void resetRef( const LuaTempResult::Item& );
    LuaObjRef& operator=(const LuaTempResult& result )  { resetRef(result[1]); return *this;}
    LuaObjRef& operator=(const LuaTempResult::Item& resultItem )  { resetRef(resultItem); return *this;}
    //@}

    /// Set this reference to point to Lua nil
    void setNil() { mRegKey.setNil(); }
    /// Test if this reference points to a Lua nil
    bool isNil() const { return mErrCode ? true : mRegKey.isNil(); }
    /// Get the Lua type identifier (one of the LUA_T variables)
    int getTypeID() const { return mErrCode ? LUA_TNIL : mRegKey.getTypeID(); }
    /// Get the Lua type name
    std::string typeName() const { return ( mLua ? lua_typename(mLua, getTypeID()) : "nil" ); }
    /// Get error status associated with the Lua operation that got this ObjRef
    bool isError() const { return mErrCode != LUA_ERR_NONE; }
    /// Get error message associated with the Lua op that got this objRef
    std::string getErrMsg() const;

    /// Give this reference a "name". Useful for log output etc.
    void setID(const std::string& id) { mID = id; }
    /// Get the name given to this reference. Returns "" if none given. 
    const std::string& getID() const { return mID; }

    //@{
    /// convert this Lua object to a TT value
    template <typename TT>  TT getAs() const;
    inline operator int   () const { return getAs<int   >(); }
    inline operator long  () const { return getAs<long  >(); }
    inline operator float () const { return getAs<float >(); }
    inline operator double() const { return getAs<double>(); }
    inline operator std::string() const { return getAs<std::string>(); }
    //@}

public: // values
    //@{
    /// Compare this Lua object to a value 
    template <typename TT> inline bool operator==(TT val) const { return getAs<TT>() == typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator!=(TT val) const { return getAs<TT>() != typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator>=(TT val) const { return getAs<TT>() >= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator<=(TT val) const { return getAs<TT>() <= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator>(TT val)  const { return getAs<TT>() >  typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator<(TT val)  const { return getAs<TT>() <  typename ValueType<TT>::Type(val); }
    //@}

public: // meta tables
    // Customize behavior of this object by changing its metatable
    /// Lock/Unlock this object's metatable from being changed by Lua scripts
    bool setMetaProtected( bool val = true );
    /// Returns true if currently has a metatable, false otherwise
    bool hasMetaTable() const;
    /// Replace this object's metatable with given table
    void setMetaTable(const LuaTableRef&);
    /// Gets this object's metatable (or nil if there is none)
    LuaTempResult getMetaTable() const;
    /// Set which table to inherit from
    void setMetaBase(const LuaTableRef&);
    /// True only if this object has a metatable with an __index entry
    bool hasMetaBase() const;

    /// Enumeration of all the possible metamethods of a Lua object
    enum MetaMethod
    {
        MM_ADD,           ///< a+b
        MM_MULTIPLY,      ///< a*b
        MM_SUBTRACT,      ///< a-b
        MM_DIVIDE,        ///< a/b
        MM_UNARY_MINUS,   ///< -a
        MM_NEGATE,        ///< synonym for -a
        MM_CONCATENATE,   ///< a..b

        MM_EQUAL,         ///< a==b
        MM_LESS_THAN,     ///< a<b
        MM_LESS_OR_EQUAL, ///< a<=b

        MM_TOSTRING,      ///< `a`
        MM_INDEX,         ///< get a[]
        MM_FIELD,         ///< synonym for get a[]
        MM_NEW_INDEX,     ///< set a[]
        MM_NEW_FIELD,     ///< synonym for set a[]
    };

    /// set this.metatable[method]=func; method determined by id
    void setMetaMethod(MetaMethod id, const LuaFuncRef& func); 

public: // interact via C API directly
    /// Push the object onto the Lua stack
    void pushObj() const 
    { 
        if (mErrCode) 
        {
            assert( lua_checkstack(mLua, 1) ); 
            lua_pushnil(mLua); 
        }
        else 
            mRegKey.pushObj(); 
    }
    /// Get the Lua state in which this object exists
    lua_State* getLuaState() const { return mLua; }

protected:
    lua_State * mLua; ///< lua state, for use by derived classes

private:
    struct AutoPop;
    template <typename TT> struct DefaultValue;

    const char* getMetaMethodFromID( MetaMethod id );

private:
    LuaRegRef mRegKey;
	#pragma warning( push )
	#pragma warning( disable : 4251)
    std::string mID;
	#pragma warning( pop )
    LuaErrCode mErrCode;
};


inline void 
pushValToStack(lua_State* L, const LuaObjRef& objRef) 
{ 
    objRef.pushObj(); 
}

// -------------------------------------------------------


template <typename TT> 
struct LuaObjRef::DefaultValue
{
    static inline TT value() {return 0;}
};
template <>
struct LuaObjRef::DefaultValue<const char*>
{
    static inline const char* value() {return "";}
};
template <>
struct LuaObjRef::DefaultValue<std::string>
{
    static inline std::string value() {return "";}
};


template <typename TT>
inline TT
LuaObjRef::getAs() 
const 
{ 
    if (mErrCode)
        return DefaultValue<TT>::value();

    LuaStackCleaner s(mLua); 
    mRegKey.pushObj(); 
    return getObjFromStack<TT>(mLua); 
}


#endif // LUA_ICXX_OBJREF_INCLUDED
