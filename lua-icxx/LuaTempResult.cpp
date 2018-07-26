/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#include "LuaTempResult.h"


LuaTempResult::LuaTempResult( const LuaTempResult& rhs ) 
: mLua(NULL), mPrevStackTop(0), mNumRetVal(0), mErrCode(rhs.mErrCode)
{
    assert(false);
}


