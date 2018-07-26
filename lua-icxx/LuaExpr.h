/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once


#include <string>


class LuaExpr
{
public:
    LuaExpr(const std::string& expr): mExpr(expr) {}

public:
    const std::string mExpr;
};
