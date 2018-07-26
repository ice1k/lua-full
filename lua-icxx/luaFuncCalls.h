/*
This file is part of the Lua_icxx library. 
Copyright 2010 (c) by Oliver Schoenborn. 
License terms in LICENSE.txt.
*/


#pragma once

#ifndef LUA_ICXX_FUNC_CALLS_INCLUDED
#define LUA_ICXX_FUNC_CALLS_INCLUDED


#define DECL_FN_CALL_1(METH, T_ARG)                                                                      template <typename T_ARG>                                                                                                                                          inline LuaTempResult METH  (const T_ARG& ) const
#define DECL_FN_CALL_2(METH, T_ARG1, T_ARG2)                                                             template <typename T_ARG1, typename T_ARG2>                                                                                                                        inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&) const
#define DECL_FN_CALL_3(METH, T_ARG1, T_ARG2, T_ARG3)                                                     template <typename T_ARG1, typename T_ARG2, typename T_ARG3>                                                                                                       inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&) const
#define DECL_FN_CALL_4(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4)                                             template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4>                                                                                      inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&) const
#define DECL_FN_CALL_5(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_ARG5)                                     template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4, typename T_ARG5>                                                                     inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&, const T_ARG5&) const
#define DECL_FN_CALL_6(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_ARG5, T_ARG6)                             template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4, typename T_ARG5, typename T_ARG6>                                                    inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&, const T_ARG5&, const T_ARG6&) const
#define DECL_FN_CALL_7(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_ARG5, T_ARG6, T_ARG7)                     template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4, typename T_ARG5, typename T_ARG6, typename T_ARG7>                                   inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&, const T_ARG5&, const T_ARG6&, const T_ARG7&) const
#define DECL_FN_CALL_8(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_ARG5, T_ARG6, T_ARG7, T_ARG8)             template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4, typename T_ARG5, typename T_ARG6, typename T_ARG7, typename T_ARG8>                  inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&, const T_ARG5&, const T_ARG6&, const T_ARG7&, const T_ARG8&) const
#define DECL_FN_CALL_9(METH, T_ARG1, T_ARG2, T_ARG3, T_ARG4, T_ARG5, T_ARG6, T_ARG7, T_ARG8, T_ARG9)     template <typename T_ARG1, typename T_ARG2, typename T_ARG3, typename T_ARG4, typename T_ARG5, typename T_ARG6, typename T_ARG7, typename T_ARG8, typename T_ARG9> inline LuaTempResult METH  (const T_ARG1&, const T_ARG2&, const T_ARG3&, const T_ARG4&, const T_ARG5&, const T_ARG6&, const T_ARG7&, const T_ARG8&, const T_ARG9&) const
#define DECL_FN_CALLS(METH)  \
    inline LuaTempResult METH () const;                                \
    DECL_FN_CALL_1(METH, TT1);                                         \
    DECL_FN_CALL_2(METH, TT1, TT2);                                    \
    DECL_FN_CALL_3(METH, TT1, TT2, TT3);                               \
    DECL_FN_CALL_4(METH, TT1, TT2, TT3, TT4);                          \
    DECL_FN_CALL_5(METH, TT1, TT2, TT3, TT4, TT5);                     \
    DECL_FN_CALL_6(METH, TT1, TT2, TT3, TT4, TT5, TT6);                \
    DECL_FN_CALL_7(METH, TT1, TT2, TT3, TT4, TT5, TT6, TT7);           \
    DECL_FN_CALL_8(METH, TT1, TT2, TT3, TT4, TT5, TT6, TT7, TT8);      \
    DECL_FN_CALL_9(METH, TT1, TT2, TT3, TT4, TT5, TT6, TT7, TT8, TT9)  


#define DEF_FN_CALL_0(CLASS_METH, BODY) inline LuaTempResult CLASS_METH() const { return BODY; }
#define DEF_FN_CALL_1(CLASS_METH, BODY) template <typename TT1>                                                                                                                 inline LuaTempResult CLASS_METH( const TT1& arg1 ) const                                                                                                                                         { return BODY; }
#define DEF_FN_CALL_2(CLASS_METH, BODY) template <typename TT1, typename TT2>                                                                                                   inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2 ) const                                                                                                                        { return BODY; }
#define DEF_FN_CALL_3(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3>                                                                                     inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3 ) const                                                                                                       { return BODY; }
#define DEF_FN_CALL_4(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4>                                                                       inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4 ) const                                                                                      { return BODY; }
#define DEF_FN_CALL_5(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4, typename TT5>                                                         inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4, const TT5& arg5 ) const                                                                     { return BODY; }
#define DEF_FN_CALL_6(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4, typename TT5, typename TT6>                                           inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4, const TT5& arg5, const TT6& arg6 ) const                                                    { return BODY; }
#define DEF_FN_CALL_7(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4, typename TT5, typename TT6, typename TT7>                             inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4, const TT5& arg5, const TT6& arg6, const TT7& arg7 ) const                                   { return BODY; }
#define DEF_FN_CALL_8(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4, typename TT5, typename TT6, typename TT7, typename TT8>               inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4, const TT5& arg5, const TT6& arg6, const TT7& arg7, const TT8& arg8 ) const                  { return BODY; }
#define DEF_FN_CALL_9(CLASS_METH, BODY) template <typename TT1, typename TT2, typename TT3, typename TT4, typename TT5, typename TT6, typename TT7, typename TT8, typename TT9> inline LuaTempResult CLASS_METH( const TT1& arg1, const TT2& arg2, const TT3& arg3, const TT4& arg4, const TT5& arg5, const TT6& arg6, const TT7& arg7, const TT8& arg8, const TT9& arg9 ) const { return BODY; } 


#endif // LUA_ICXX_FUNC_CALLS_INCLUDED
