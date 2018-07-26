/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once

#ifndef LUA_ICXX_TABLEREF_INCLUDED
#define LUA_ICXX_TABLEREF_INCLUDED

//////////////////////////////////////////////////////////////////////////
// Includes and declarations
//////////////////////////////////////////////////////////////////////////


#include <string>

#include "LuaObjRef.h"
#include "LuaStackCleaner.h"


//////////////////////////////////////////////////////////////////////////
// Types/Classes
//////////////////////////////////////////////////////////////////////////


/** Represent a reference to a Lua object that is a table. Extends LuaObjRef by 
    adding field set/get methods. 
    */
class
LuaTableRef : public LuaObjRef
{
public:
    // C++ requires all constructors to be duplicated

    /// Reference to nil
    explicit LuaTableRef( const std::string& id = "nil" ): LuaObjRef(id) {}

    //@{
    /// Create from object on Lua stack
    LuaTableRef( const LuaTempResult& res ): LuaObjRef(res) {}
    LuaTableRef( const LuaTempResult::Item& item ): LuaObjRef(item) {}
    LuaTableRef( lua_State* lua, int stackPos ): LuaObjRef(lua, stackPos) {}
    //@}

    template <typename KeyType>
    class Field;

    /// see if table contains a field for given key
    template <typename KeyType> 
        inline bool hasField( const KeyType& key ) const;

    //@{
    /// get table[key]
    template <typename KeyType> 
        inline LuaTempResult getField(const KeyType& key) const;
    template <typename KeyType> 
        inline Field<KeyType> operator[] (const KeyType& key) { return Field<KeyType>(*this, key); }
    //@}

    //@{
    template <typename ValType>
        inline void setField( const char* key, const ValType& obj );
    template <typename ValType>
        inline void setField( const std::string& key, const ValType& obj ) { setField(key.c_str(), obj); }
    template <typename KeyType, typename ValType> 
        inline void setField( const KeyType& key, const ValType& obj );
    //@}

    //@{
    /// remove field from table
    inline void delField( const char* key );
    inline void delField( const std::string& key ) { delField(key.c_str()); }
    template <typename KeyType> 
        inline void delField( const KeyType& key );
    //@}

private:

    // all pushFieldValue methods assume table already on stack

    // push t[key] on top of stack
    void pushFieldValue( int key ) const { assert( lua_checkstack(mLua, 1) ); lua_rawgeti(mLua, -1, key); }
    void pushFieldValue( const char* key ) const { assert( lua_checkstack(mLua, 1) ); lua_getfield(mLua, -1, key); }
    void pushFieldValue( const std::string& key ) const { pushFieldValue(key.c_str()); }
    // when key=bool, float, double, short, etc
    template <typename TT>
        inline void pushFieldValue( const TT& key ) const;
};


//////////////////////////////////////////////////////////////////////////
// LuaTableRef::Field<KeyType>
//////////////////////////////////////////////////////////////////////////


/** Represent a field of a Lua Table reference. This allows getting and setting 
    field values. The LuaTableRef for which fields are required must outlive the 
    Field instance. Same goes for the key used. 
    */
template <typename KeyType>
class LuaTableRef::Field
{
public:
    /// Represent table[key]
    Field(LuaTableRef& table, const KeyType& key): mTable(table), mKey(key) {}
    

    /// Set value for this field to given obj
    template <typename ValType>
        void operator=(const ValType& obj) { mTable.setField(mKey, obj); }
    /// Set value for this field to other field (any table)
    void operator=(const Field& rhs) { mTable.setField(mKey, rhs); }

    /// get table field value
    operator LuaTempResult() const { return mTable.getField(mKey); }
    /// Convert item to a value such as int, float, etc
    template <typename TT>
        operator TT() const { return TT(mTable.getField(mKey)); }

    /// is this field value equal to given object
    template <typename ValType>
        bool operator==(const ValType& obj) const { return mTable.getField(mKey) == obj; }
    /// is this field value Lua nil 
    bool isNil() const { return mTable.getField(mKey).isNil(); }

public:
    /// Push the field value onto the Lua stack
    inline void pushObj() const;

private:
    LuaTableRef& mTable;
    const KeyType& mKey;
};


//////////////////////////////////////////////////////////////////////////
// inlines and templates for LuaTableRef::Field<KeyType>
//////////////////////////////////////////////////////////////////////////


template <typename KeyType>
inline void 
LuaTableRef::Field<KeyType>::pushObj() 
const
{ 
    mTable.pushObj(); 
    mTable.pushFieldValue(mKey); 
    lua_remove(mTable.getLuaState(), -2);     // remove table (just need value)
}


template <typename KeyType>
inline void 
pushValToStack(lua_State* L, const LuaTableRef::Field<KeyType>& field) 
{ 
    field.pushObj();
}


//////////////////////////////////////////////////////////////////////////
// inlines and templates for LuaTableRef
//////////////////////////////////////////////////////////////////////////


// push the value for given key onto stack
template <typename TT>
inline void 
LuaTableRef::pushFieldValue( const TT& key ) 
const
{
    pushValToStack(mLua, key);
    lua_gettable(mLua, -2);      // get t[key] on top of stack
}


template <typename KeyType>
inline bool 
LuaTableRef::hasField( const KeyType& key ) 
const
{
    const LuaStackCleaner s(mLua);
    pushObj();
    pushFieldValue(key);
    assert( s.willPop(2) );
    return ! lua_isnil(mLua, -1);
}


template <typename KeyType>
inline LuaTempResult
LuaTableRef::getField( const KeyType& key ) 
const
{
    const int prevStackTop = lua_gettop(mLua);
    pushObj();                // push table on stack
    pushFieldValue(key);      // get value for key
    lua_remove(mLua, -2);     // remove table (just need value)
    return LuaTempResult(mLua, prevStackTop);
}


template <typename ValType>
inline void 
LuaTableRef::setField( const char* key, const ValType& obj )
{
    const LuaStackCleaner s(mLua);
    pushObj();
    pushValToStack(mLua, obj); 
    lua_setfield(mLua, -2, key);
    assert( s.willPop(1) );
}


template <typename KeyType, typename ValType>
inline void 
LuaTableRef::setField( const KeyType& key, const ValType& obj )
{
    const LuaStackCleaner s(mLua);
    pushObj();
    pushValToStack(mLua, key);
    pushValToStack(mLua, obj); 
    lua_settable(mLua, -3);
    assert( s.willPop(1) );
}


inline void 
LuaTableRef::delField( const char* key )
{
    const LuaStackCleaner s(mLua);
    pushObj();
    assert( lua_checkstack(mLua, 1) ); 
    lua_pushnil(mLua);
    lua_setfield(mLua, -2, key);
    assert( s.willPop(1) );
}


template <typename KeyType> 
inline void 
LuaTableRef::delField( const KeyType& key )
{
    const LuaStackCleaner s(mLua);
    pushObj();
    pushValToStack(mLua, key);
    assert( lua_checkstack(mLua, 1) ); 
    lua_pushnil(mLua);
    lua_settable(mLua, -3);
    assert( s.willPop(1) );
}


#endif // LUA_ICXX_TABLEREF_INCLUDED 

