/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once

#ifndef LUA_ICXX_FUNCREF_INCLUDED
#define LUA_ICXX_FUNCREF_INCLUDED


#include "LuaObjRef.h"
#include "LuaAdapters.h"
#include "luaFuncCalls.h"


class LuaFuncCallParams;
class LuaTableRef;


/** Represent a reference to a Lua object that is a Lua function. 
    It extends LuaObjRef 
    to add call / operator() as well as function environment set/get. 
    The latter is useful to create a "sandbox" for the function. 
    More info is available elsewhere in this documentation. 
*/
class
LuaFuncRef: public LuaObjRef
{
public: 
    /// Reference to Lua nil
    explicit LuaFuncRef( const std::string& id = "nil" ): LuaObjRef(id) {}

    //@{
    /// Reference to object on the Lua stack
    LuaFuncRef( const LuaTempResult& res): LuaObjRef(res) {}
    LuaFuncRef( const LuaTempResult::Item& item): LuaObjRef(item) {}
    LuaFuncRef( lua_State* lua, int stackPos ): LuaObjRef(lua, stackPos) {}
    //@}

    /// Set this function's global environment
    bool setEnv(const LuaTableRef&);
    /** Returns the function's environment table. This is nil if the
        function does not have one; but an error if the function is nil.
        */
    LuaTempResult getEnv() const;

    //@{ 
    /// call methods/operator
    DECL_FN_CALLS(operator());
    DECL_FN_CALLS(call);
    //@}

private: 
    inline LuaFuncCallParams prepCall() const;
};


/** Represent a bound Lua method. Ie, a Lua function that is "in" a Lua object and takes 
    the object as its first argument. It is the C++ way of providing Lua's syntactic 
    sugar, "object:function(...)", the equivalent of "object.function(object, ...)". 

    Note that there is no Lua representation for bound methods (hence LuaBoundMethRef 
    does not derive from LuaObjRef): a bound method is a callable object that consists 
    of a Lua object plus a Lua function, and for which the call operator automatically 
    provides the object as first argument when the function is called. 
    */
class
LuaBoundMethRef
{
public:
    /// Nil bound method
    LuaBoundMethRef() {} // nil
    /// Create from a function fn and an object; function must expect object as first arg
    LuaBoundMethRef( const LuaObjRef & obj, const LuaFuncRef & fn )
        : mMethRef( fn ), mObjRef(obj) {}
    /// create from LuaTempResult; res[1] is function, is object res[2]
    LuaBoundMethRef( const LuaTempResult & res )
        : mObjRef( res[1] ), mMethRef( res[2] ) {}

    //@{
    /// Change what the reference is pointing to
    void resetRef( const LuaObjRef & obj, const LuaFuncRef & fn );
    void resetRef( const LuaTempResult& result ) { resetRef(result[1], result[2]); }
    LuaBoundMethRef& operator=(const LuaTempResult& result )  { resetRef(result); return *this;}
    //@}

    /// true if Lua object OR Lua function is nil
    bool isNil() const { return mObjRef.isNil() || mMethRef.isNil(); }
    /// return a type name for this "new" Lua type
    std::string typeName() const { return isNil() ? "nil" : "boundmethod"; }
    /// returns an ID created from the Lua object ID and function ID
    std::string getID() const { return mObjRef.getID() + ":" + mMethRef.getID(); }

    //@{
    /// Call the bound method. Automatically provides the Lua object as 
    /// first arg to in the call
    DECL_FN_CALLS(operator());
    DECL_FN_CALLS(call);
    //@}

private:
    LuaObjRef   mObjRef;
    LuaFuncRef  mMethRef;
};


/** Represent a reference to a Lua object that is a User Data Type (a class). 
    It extends LuaObjRef
    with callMethod(...) which takes a method name, assumed to be a function that 
    takes the object as first argument. The callMethod() automatically provide the 
    object as first argument to the function. As well, it is possible to create 
    a LuaBoundMethRef from it, given a member function name. 
    */
class
LuaClassObjRef: public LuaObjRef
{
public:
    /// Reference to nil
    explicit LuaClassObjRef( const std::string& id = "nil" ): LuaObjRef(id) {}
    //@{
    /// Create from object on Lua stack
    LuaClassObjRef( const LuaTempResult& res): LuaObjRef(res) {}
    LuaClassObjRef( const LuaTempResult::Item& item): LuaObjRef(item) {}
    LuaClassObjRef( lua_State* lua, int stackPos ): LuaObjRef(lua, stackPos) {}
    //@}

    /// Get one of the methods of the object referenced
    LuaBoundMethRef getBoundMethod(const std::string&) const;

    //@{
    /// Call given method on the object. Automatically provides the 
    /// Lua object as first arg to in the call
    DECL_FN_CALLS(callMethod);
    //@}

private: 
    inline LuaFuncCallParams prepCall(const std::string&) const;
};


// ----------------------------------------------------------------------

/** Prevent function calls that use LuaTempResult as one of the parameters. Function
    calls return a LuaTempResult, so using it as a parameter will lead to wrong 
    results due to stack manipulation conflicts. Use LuaExpr instead (of a call to 
    LuaInterpreter::eval or to whatever Lua_icxx function/method returning a 
    LuaTempResult).

    Class template specializations are declared for classes that must not be used as 
    function calls parameters (such as LuaTempResult, and its Item inner class), 
    but they are left undefined; this leads to a compilation error when attempt is 
    made to pass such object to function call. 

    For instance, in MS VC++ 2005, the error is C2027: use of undefined type 
    'NotAllowedAsFunctionArgument&lt;LuaTempResult>', in file LuaFuncRef.h. To find 
    source code that uses the forbidden LuaTempResult, look in the Output window at 
    the bottom of the error message. 
    */
template <typename TT>
struct NotAllowedAsFunctionArgument 
{ 
    /// used only internally
    static inline const TT& ref(const TT& obj) { return obj; } 
};

/// prevent LuaTempResult
template <>
struct NotAllowedAsFunctionArgument<LuaTempResult>;

/// prevent LuaTempResult::Item
template <>
struct NotAllowedAsFunctionArgument<LuaTempResult::Item>;


/*  Puts the function call parameters on the Lua stack, makes the call, and 
    encapsulates the return values in a LuaTempResult. 
    */
class
LuaFuncCallParams
{
public:
    LuaFuncCallParams(const std::string& funcID, lua_State* lua, int currStackTop, 
        int alreadyOnStack = 0)
        : mFuncID(funcID.empty() ? "<anonymous>" : funcID), mLua(lua), 
        preStackTop(currStackTop), argsOnStack(alreadyOnStack) {}

public:
    template <typename TT>
    inline LuaFuncCallParams& operator<<(const TT& obj)
    { 
        pushValToStack(mLua, NotAllowedAsFunctionArgument<TT>::ref(obj));
        argsOnStack ++;
        return *this;
    }

    LuaTempResult doCall() const 
    {
        assert(mLua);
        const int numRets = LUA_MULTRET;
        const int errCode = lua_pcall(mLua, argsOnStack, numRets, 0);
        return LuaTempResult(mLua, preStackTop, errCode, "Function call " + mFuncID + "()");
    }

    // doesn't work: 
    //operator LuaTempResult() const 

private:
    lua_State * mLua;
	#pragma warning( push )
	#pragma warning( disable : 4251)
    std::string mFuncID; 
	#pragma warning( pop )
    const int preStackTop;
    int argsOnStack;
};


inline LuaFuncCallParams 
LuaFuncRef::prepCall() 
const 
{
    const int curStackTop = (mLua ? lua_gettop(mLua) : 0);
    pushObj();
    return LuaFuncCallParams(getID(), mLua, curStackTop);
}


inline LuaFuncCallParams 
LuaClassObjRef::prepCall(const std::string& methName) 
const 
{
    const int curStackTop = (mLua ? lua_gettop(mLua) : 0);
    // get method of object:
    pushObj();
    lua_getfield(mLua, -1, methName.c_str());
    // move method before object since object must be first arg to function call: 
    lua_insert(mLua, -2);
    assert( lua_gettop(mLua) == curStackTop + 2 );
    return LuaFuncCallParams(getID(), mLua, curStackTop, 1);
}


// ----------- LuaFuncRef::call(...) and operator()(...) for 0 to 9 args of any type ------------------------

DEF_FN_CALL_0(LuaFuncRef::call,   prepCall().doCall() );
DEF_FN_CALL_1(LuaFuncRef::call, ( prepCall() << arg1 ).doCall() );
DEF_FN_CALL_2(LuaFuncRef::call, ( prepCall() << arg1 << arg2 ).doCall() );
DEF_FN_CALL_3(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 ).doCall() );
DEF_FN_CALL_4(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 ).doCall() );
DEF_FN_CALL_5(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 << arg5 ).doCall() );
DEF_FN_CALL_6(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 ).doCall() );
DEF_FN_CALL_7(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 ).doCall() );
DEF_FN_CALL_8(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 ).doCall() );
DEF_FN_CALL_9(LuaFuncRef::call, ( prepCall() << arg1 << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 ).doCall() );

DEF_FN_CALL_0(LuaFuncRef::operator(), call() );
DEF_FN_CALL_1(LuaFuncRef::operator(), call(arg1) );
DEF_FN_CALL_2(LuaFuncRef::operator(), call(arg1, arg2 ) );
DEF_FN_CALL_3(LuaFuncRef::operator(), call(arg1, arg2, arg3 ) );
DEF_FN_CALL_4(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4 ) );
DEF_FN_CALL_5(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4, arg5 ) );
DEF_FN_CALL_6(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4, arg5, arg6 ) );
DEF_FN_CALL_7(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4, arg5, arg6, arg7 ) );
DEF_FN_CALL_8(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8 ) );
DEF_FN_CALL_9(LuaFuncRef::operator(), call(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9 ) );


// same for LuaBoundMethRef: 

DEF_FN_CALL_0(LuaBoundMethRef::call, mMethRef(mObjRef) );
DEF_FN_CALL_1(LuaBoundMethRef::call, mMethRef(mObjRef, arg1) );
DEF_FN_CALL_2(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2) );
DEF_FN_CALL_3(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3) );
DEF_FN_CALL_4(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4) );
DEF_FN_CALL_5(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4, arg5) );
DEF_FN_CALL_6(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4, arg5, arg6) );
DEF_FN_CALL_7(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4, arg5, arg6, arg7) );
DEF_FN_CALL_8(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) );
DEF_FN_CALL_9(LuaBoundMethRef::call, mMethRef(mObjRef, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) );

DEF_FN_CALL_0(LuaBoundMethRef::operator(), call() );
DEF_FN_CALL_1(LuaBoundMethRef::operator(), call( arg1) );
DEF_FN_CALL_2(LuaBoundMethRef::operator(), call( arg1, arg2) );
DEF_FN_CALL_3(LuaBoundMethRef::operator(), call( arg1, arg2, arg3) );
DEF_FN_CALL_4(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4) );
DEF_FN_CALL_5(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4, arg5) );
DEF_FN_CALL_6(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4, arg5, arg6) );
DEF_FN_CALL_7(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4, arg5, arg6, arg7) );
DEF_FN_CALL_8(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) );
DEF_FN_CALL_9(LuaBoundMethRef::operator(), call( arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9) );


// for LuaClassObjRef:

DEF_FN_CALL_1(LuaClassObjRef::callMethod, prepCall(arg1).doCall() );
DEF_FN_CALL_2(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 ).doCall() );
DEF_FN_CALL_3(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 ).doCall() );
DEF_FN_CALL_4(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 ).doCall() );
DEF_FN_CALL_5(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 << arg5 ).doCall() );
DEF_FN_CALL_6(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 << arg5 << arg6 ).doCall() );
DEF_FN_CALL_7(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 ).doCall() );
DEF_FN_CALL_8(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 ).doCall() );
DEF_FN_CALL_9(LuaClassObjRef::callMethod, ( prepCall(arg1) << arg2 << arg3 << arg4 << arg5 << arg6 << arg7 << arg8 << arg9 ).doCall() );


#endif // LUA_ICXX_FUNCREF_INCLUDED
