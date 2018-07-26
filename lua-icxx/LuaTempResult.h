/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once

#ifndef LUA_ICXX_TEMPRESULT_INCLUDED
#define LUA_ICXX_TEMPRESULT_INCLUDED

#include <string>
#include <cassert>
#include <stdlib.h>

#include "LuaCallStatus.h"
#include "LuaAdapters.h"


// used by operators for safe comparison of char*
template <typename TT> 
struct ValueType 
{ 
    typedef TT Type; 
};


template <> 
struct ValueType<const char*> 
{ 
    typedef std::string Type; 
};


template <int N> 
struct ValueType<char[N]> 
{ 
    typedef std::string Type; 
};


/** Manage the result of a call to a Lua interpreter, still on the Lua stack. 
    This should only be created by Lua_icxx classes. The result is a tuple of 
    0 to N items, depending on what the call returned. The result also has a 
    status that indicates whether the call succeeded, and associated error message. 

    Note that as the name indicates, these are meant to be used as temporary 
    wrappers for values on the Lua stack; they must not be stored as data members
    (use LuaObjRef or a derived class for this). Local storage is fine: 

    \code 
    LuaTempResult res = lua.eval("...");
    LuaTempResult res2 = lua.eval("...");
    // automatically cleans up stack when goes out of scope
    \endcode 

    Also, LuaTempResult cannot be reset, because this would mess up the stack:
    \code 
    LuaTempResult res = lua.eval("..."); // OK, same as res( lua.eval... )
    res = lua.eval("..."); // compilation ERROR!
    \endcode 
    */
class
LuaTempResult
{
public:
    /// only to be called by Lua_icxx classes
    LuaTempResult( lua_State*, int prevStackTop, int errCode = 0, const std::string& errMsgPrefix = "");
    ~LuaTempResult();

    /// false if error, true otherwise:
    bool ok() const { return ! isError(); }
    /// true if error, false otherwise
    bool isError() const { return mErrCode != LUA_ERR_NONE; }
    /// err code for call (undefined if ok() is true)
    int getErrCode() const { return mErrCode; }
    /// err msg for call (empty if ok() is true)
    inline std::string getErrMsg() const;

    /// how many items in result; result.getItem() can take index=0 to result.getNumItems() - 1
    size_t getNumItems() const { return mNumRetVal; }
    /// getItem indexing starts at this array offset
    static const int FIRST_ITEM_INDEX = 1;
    /// nested class
    class Item;
    //@{
    /// Get specific result getItem; index must be between 1 and numIntems(), 
    /// ie getItem(1) or (*this)[1] is first getItem; getItem(-1) or (*this)[-1] is 
    /// last getItem; index < 1 or > getNumItems() is undefined (assertion will fail)
    inline Item getItem(int index = FIRST_ITEM_INDEX) const;
    inline Item operator[](int index) const;
    //@}

    /// returns true if result getItem at given index (or 0 if none given, ie first getItem) is Lua nil
    inline bool isNil(int indx = FIRST_ITEM_INDEX) const;

    //@{
    /// Convert item to a value such as int, float, etc
    template <typename TT> 
    inline TT getAs(int indx = FIRST_ITEM_INDEX) const;
    inline operator int   () const { return getAs<int   >(); }
    inline operator long  () const { return getAs<long  >(); }
    inline operator float () const { return getAs<float >(); }
    inline operator double() const { return getAs<double>(); }
    inline operator std::string() const { return getAs<std::string>(); }
    //@}

    //@{
    /// Compare first getItem of result to another value such as int, float etc
    template <typename TT> inline bool operator == (TT val) const { return getAs<TT>() == typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator != (TT val) const { return getAs<TT>() != typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator >= (TT val) const { return getAs<TT>() >= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator <= (TT val) const { return getAs<TT>() <= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator >  (TT val) const { return getAs<TT>() >  typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator <  (TT val) const { return getAs<TT>() <  typename ValueType<TT>::Type(val); }
    //@}

    /// Get the position of given item on stack; return 0 
    /// (invalid stack pos) if index out of range
    inline int getStackPos(int index) const;

    /// Get Lua state used by this result
    lua_State* getLuaState() const { return mLua; }

    /// Push one of the items on the stack; pushes nil if 
    /// index out of range or isError() is true
    inline void push(int index = FIRST_ITEM_INDEX) const;

public:
    /// this is forbidden, but must be made available to compiler
    LuaTempResult(const LuaTempResult& rhs);

private:
    // prevent resetting
    void operator=(const LuaTempResult& rhs);

private:
    lua_State * mLua;

    //LuaCallStatus mStatus;
    LuaErrCode mErrCode; 
    const int mPrevStackTop;
    const int mNumRetVal;
};


/** Represent one of the items of a LuaTempResult. This getItem is still on the Lua
    stack. Items are created by LuaTempResult. 
    */
class
LuaTempResult::Item
{
public:
    /** Construct an getItem to refer to the Lua object at given stack pos of given 
        Lua state. This is typically called by LuaTempResult (though it is ok to 
        call directly if working directly with a Lua state). Note that stackPos can
        be LUA_UNACCEPTABLE_STACK_POS, in which case this Item is equiv to nil. 
        */
    Item(lua_State* lua, int stackPos, LuaErrCode errCode)
        : mLua(lua), mStackPos(stackPos), mErrCode(errCode) { assert(lua); }

public:
    /// Get the type of Lua object referenced by this Item (returns one of the LUA_T enum)
    int getTypeID() const { return isInvalid() ? LUA_TNIL : lua_type(mLua, mStackPos); }
    /// Test if Lua nil
    bool isNil() const { return isInvalid() ? true : lua_isnil(mLua, mStackPos); }

    /// Push the Item onto the Lua stack
    inline void push() const;
    /// Test if Lua error
    bool isError() const { return mErrCode != LUA_ERR_NONE; }

    //@{
    /// Convert Item to a value such as int, float, etc
    template <typename TT> 
    inline TT getAs() const { return 
        getObjFromStack<TT>(mLua, mErrCode ? LUA_UNACCEPTABLE_STACK_POS : mStackPos); } 
    inline operator int   () const { return getAs<int   >(); }
    inline operator long  () const { return getAs<long  >(); }
    inline operator float () const { return getAs<float >(); }
    inline operator double() const { return getAs<double>(); }
	inline operator std::string() const { return getAs<std::string>(); }
	//template <typename TT>
	//	inline operator TT() const { return getAs<TT>(); }
    //@}

    //@{
    /// Compare a result Item to another value such as int, float etc
    template <typename TT> inline bool operator == (TT val) const { return getAs<TT>() == typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator != (TT val) const { return getAs<TT>() != typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator >= (TT val) const { return getAs<TT>() >= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator <= (TT val) const { return getAs<TT>() <= typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator >  (TT val) const { return getAs<TT>() >  typename ValueType<TT>::Type(val); }
    template <typename TT> inline bool operator <  (TT val) const { return getAs<TT>() <  typename ValueType<TT>::Type(val); }
    //@}

private:
    bool isInvalid() const { return mErrCode || mStackPos == LUA_UNACCEPTABLE_STACK_POS; }

public:
    lua_State * const mLua; ///< Lua state in which this getItem resides
    int const mStackPos;    ///< position of this getItem on stack for Lua state
    LuaErrCode const mErrCode; ///< err code for this item
};


inline 
LuaTempResult::LuaTempResult( 
     lua_State* lua, int prevStackTop, int errCode, const std::string& errMsgPrefix)
     : mLua(lua)
     , mErrCode(LuaErrCode(errCode))
     , mPrevStackTop(prevStackTop)
     , mNumRetVal(lua_gettop(lua) - prevStackTop)
{
    assert( lua );
    assert( mNumRetVal >= 0 );
    // if error, only one value returned: the error message
    assert( (mErrCode == LUA_ERR_NONE) || (mNumRetVal == 1));
    if (mErrCode)
    {
        // replace err message that is on stack with the correct one
        const std::string errMsg 
            = errMsgPrefix + ": " + lua_tostring(lua, prevStackTop+1);
        assert( lua_checkstack(mLua, 1) );
        lua_pushstring(lua, errMsg.c_str());
        lua_replace(lua, prevStackTop+1);
    }
}


inline 
LuaTempResult::~LuaTempResult()
{
    assert( lua_gettop(mLua) == mPrevStackTop +  mNumRetVal );
    lua_settop(mLua, mPrevStackTop);
}


/*  Get the stack position of item[index] (index=1 is first, -1 last, etc).
    If index=0 or > getNumItems() or < -getNumItems(), returns 0 (invalid stack
    position). Note that if isError(), returns the stack position of error
    message, regardless of index. 
    */
int 
LuaTempResult::getStackPos(int index) 
const
{
    if (index > 0 && index <= mNumRetVal)
        return mErrCode 
            ? mPrevStackTop + 1
            : mPrevStackTop + index + (1 - FIRST_ITEM_INDEX);

    if (index < 0 && index >= -mNumRetVal)
        return mErrCode 
            ? mPrevStackTop + 1
            : mPrevStackTop + mNumRetVal + 1 + index;

    return LUA_UNACCEPTABLE_STACK_POS;
}


template <typename TT> 
inline TT 
LuaTempResult::getAs(int index) 
const 
{ 
    return getObjFromStack<TT>( mLua, getStackPos(index) ); 
}


inline bool 
isStackPosAcceptable(lua_State* lua, int pos)
{
    return pos != 0 && abs(pos) <= lua_gettop(lua);
    //(pos > 0 && pos <= LUA_STACK_SIZE) || (pos < 0 && abs(pos) <= lua_gettop(lua));
}


inline void 
pushValToStack(lua_State* L, const LuaTempResult::Item& objRef) 
{ 
    assert( L == objRef.mLua );
    if ( objRef.mErrCode )
    {
        assert( lua_checkstack(L, 1) );
        lua_pushnil(L);
    }
    else
        objRef.push();
}


inline void 
pushValToStack(lua_State* L, const LuaTempResult& res) 
{ 
    res.push();
}


inline bool 
LuaTempResult::isNil(int indx) 
const 
{ 
    if ( mErrCode ) return true;
    const int stackPos = getStackPos(indx);
    return isStackPosAcceptable(mLua, stackPos) ? lua_isnil(mLua, stackPos) : true; 
}


inline void 
LuaTempResult::push(int index) 
const 
{ 
    if ( mErrCode ) 
        lua_pushnil(mLua); 
    else 
    {
        const int stackPos = getStackPos(index);
        assert( lua_checkstack(mLua, 1) );
        if ( isStackPosAcceptable(mLua, stackPos) )
            lua_pushvalue(mLua, stackPos);
        else
            lua_pushnil(mLua); 
    }
}


inline std::string 
LuaTempResult::getErrMsg() 
const 
{
    return mErrCode ? lua_tostring(mLua, mPrevStackTop + 1) : "";
}


LuaTempResult::Item 
LuaTempResult::getItem( int index ) const
{
    return Item(mLua, getStackPos(index), mErrCode);
}


inline LuaTempResult::Item 
LuaTempResult::operator[](int index) 
const 
{ 
    return getItem(index); 
}


inline void 
LuaTempResult::Item::push() 
const
{
    assert(lua_checkstack(mLua, 1));
    if ( isInvalid() )
        lua_pushnil(mLua);
    else
        lua_pushvalue(mLua, mStackPos);
}

#endif // LUA_ICXX_TEMPRESULT_INCLUDED
