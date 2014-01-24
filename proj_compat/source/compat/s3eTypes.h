/*
 * (C) 2001-2012 Marmalade. All Rights Reserved.
 *
 * This document is protected by copyright, and contains information
 * proprietary to Marmalade.
 *
 * This file consists of source code released by Marmalade under
 * the terms of the accompanying End User License Agreement (EULA).
 * Please do not use this program/source code before you have read the
 * EULA and have agreed to be bound by its terms.
 */
#ifndef S3E_TYPES_H
#define S3E_TYPES_H

/**
 * @addtogroup s3egroup
 * @{
 */

/**
 * @defgroup typesapigroup S3E Types API Reference
 *
 * Defines standard types used throughout the S3E module.
 *
 * For more information, see the "S3E Types Overview" section of the S3E API
 * Documentation.
 *
 * @{
 */

/**
 * @name Scalar Types, with s3e prefix
 */
/** @{ */
typedef unsigned char s3e_uint8_t;
typedef signed char s3e_int8_t;
typedef unsigned short int s3e_uint16_t;
typedef signed short int s3e_int16_t;
typedef unsigned int s3e_uint32_t;
typedef signed int s3e_int32_t;
#if defined _MSC_VER && (_MSC_VER > 1000)
  typedef __int64 s3e_int64_t;
  typedef unsigned __int64 s3e_uint64_t;
#else
  #ifdef I3D_ARCH_AMD64
  typedef signed long s3e_int64_t;
  typedef unsigned long s3e_uint64_t;
  #else
  typedef signed long long s3e_int64_t;
  typedef unsigned long long s3e_uint64_t;
  #endif
#endif
/** @} */

/**
 * @name C99 fixed-width data types, based on s3e types
 */
/** @{ */
#if defined I3D_OS_NACL || defined __QNXNTO__
#include <stdint.h>
#else
typedef s3e_uint64_t uint64_t;
typedef s3e_int64_t int64_t;
typedef s3e_uint32_t uint32_t;
typedef s3e_int32_t int32_t;
typedef s3e_uint16_t uint16_t;
typedef s3e_int16_t int16_t;
typedef s3e_uint8_t uint8_t;
typedef s3e_int8_t int8_t;
#endif

// \cond HIDDEN_DEFINES
#if !defined __intptr_t_defined && !defined _INTPTR_T
#define __intptr_t_defined
#if !defined _INTPTR_T_DEFINED && !defined _INTPTR_T
#define _INTPTR_T_DEFINED
#define _INTPTR_T
#define _UINTPTR_T
#define _UINTPTR_T_DEFINED
#ifdef I3D_ARCH_AMD64
    typedef long int intptr_t;
    typedef unsigned long int uintptr_t;
#else
    typedef int intptr_t;
    typedef unsigned int uintptr_t;
#endif
#endif
#endif
// \endcond
/** @} */

/**
 * @name Non-standard fixed-width types without _t suffix or s3e prefix
 * @{
 */
typedef unsigned int uint;
typedef unsigned short int ushort;
typedef unsigned long int ulong;
typedef unsigned char uint8;
typedef signed char int8;

#ifndef _INT64
typedef int64_t int64;
typedef uint64_t uint64;
#define _INT64
#endif

#if !defined _UINT32 || defined I3D_OS_QNX
typedef unsigned int uint32;
#ifndef _UINT32
#define _UINT32
#endif
#endif

#if !defined _INT32 || defined I3D_OS_QNX
typedef signed int int32;
#ifndef _INT32
#define _INT32
#endif
#endif

#ifndef _UINT16
typedef uint16_t uint16;
#define _UINT16
#endif

#ifndef _INT16
typedef int16_t int16;
#define _INT16
#endif
/*
 * @}
 *
 * @name Scalar limits
 * @{
 */
#ifndef UINT64_MAX
 #ifdef __GNUC__
    #define UINT64_MAX  0xffffffffffffffffLL
 #else
    #define UINT64_MAX  0xffffffffffffffff
 #endif
#endif
#ifndef UINT32_MAX
    #define UINT32_MAX  0xffffffffUL
#endif
#ifndef UINT16_MAX
    #define UINT16_MAX  0xffff
#endif
#ifndef UINT8_MAX
    #define UINT8_MAX   0xff
#endif

#ifndef INT64_MAX
 #ifdef __GNUC__
    #define INT64_MAX  0x7fffffffffffffffLL
 #else
    #define INT64_MAX  0x7fffffffffffffff
 #endif
#endif
#ifndef INT32_MAX
    #define INT32_MAX   0x7fffffffL
#endif
#ifndef INT16_MAX
    #define INT16_MAX   0x7fff
#endif
#ifndef INT8_MAX
    #define INT8_MAX    0x7f
#endif


#ifndef INT64_MIN
    #define INT64_MIN   (-INT64_MAX - 1)
#endif
#ifndef INT32_MIN
    #define INT32_MIN   (-INT32_MAX - 1)
#endif
#ifndef INT16_MIN
    #define INT16_MIN   (-INT16_MAX - 1)
#endif
#ifndef INT8_MIN
    #define INT8_MIN    (-INT8_MAX  - 1)
#endif

#define S3E_DBL_MIN 2.22507385850720138e-308
#define S3E_DBL_MAX 1.79769313486231571e+308
#define S3E_FLT_MAX 3.402823466e+38F

#ifndef NULL
    #if defined __GNUG__
        #define NULL __null
    #else
        #ifndef __cplusplus
            #define NULL ((void *)0)
        #else
            #define NULL 0
        #endif
    #endif
#endif
/** @} */

/**
 * Generic S3E result type.
 * @par Required Header Files
 * s3eTypes.h
 */
typedef enum s3eResult
{
    S3E_RESULT_SUCCESS = 0,  //!< The operation completed successfully
    S3E_RESULT_ERROR = 1     //!< An error occurred during the operation
} s3eResult;

/**
 * Generic S3E boolean type.
 */
typedef uint8 s3eBool;

/**
 * S3E wide-char type.
 *
 * s3eWChar is defined to be interchangable with the wchar_t built-in type.
 * Under S3E sizeof(wchar_t) is always 2.
 */
#if defined __cplusplus && defined I3D_OS_S3E
typedef wchar_t s3eWChar;
#else
typedef unsigned short s3eWChar;
#endif

/**
 * Callback function type.
 * @param systemData callback-specific system supplied data.
 * @param userData data supplied by user at callback registration.
 * @par Required Header Files
 * s3eTypes.h
 */
typedef int32 (*s3eCallback) (void* systemData, void* userData);

#define S3E_WEOF  ((s3eWChar)-1)
#define S3E_FALSE (0)
#define S3E_TRUE  (1)

// \cond HIDDEN_DEFINES
// this is needed since makmake is dumb and undefs all the compiler builtins
// while doing it dependancy checks
#if defined __ARMCC__ && !defined __ARMCC_VERSION
    #define __ARMCC_VERSION 220000
#endif

// this is needed since makmake is dumb and undefs all the compiler builtins
// while doing it dependancy checks
#if !defined __GNUC__ && (defined __GCC32__ || defined __GCCE__)
    #define __GNUC__ 1
#endif

// Assume that we have RTTI available unless GCC has been run with
// the -fno-rtti flag which can be detected by the lack of __GXX_RTTI.

#if defined __cplusplus && (!defined __GNUC__ || defined __GXX_RTTI || defined __llvm__)
#define S3E_HAVE_RTTI
#endif

// Definition of platform/compiler independent S3E_INLINE macro
#if defined I3D_OS_ARM_MBX
    // we're using the arm compiler so do as in epoc arm
    #define S3E_INLINE          __inline
    #define INLINE_MEMBER   __inline
#elif defined __ARMCC_VERSION
    // we're using the arm compiler so do as in epoc arm. When we use inline,
    // we mean it.
    #if __ARMCC_VERSION >= 200000
        #define S3E_INLINE       __forceinline
        #define INLINE_MEMBER __forceinline
    #else
        #define S3E_INLINE       __inline
        #define INLINE_MEMBER __inline
    #endif
#elif defined I3D_OS_EPOC
    // This has to be "static" because otherwise the linker will have about a
    // million "multiply declared function" errors
    #ifdef __ARMCC_VERSION
        #define S3E_INLINE __inline
        #define INLINE_MEMBER __inline
    #else
        #ifdef __VC32__
            #define S3E_INLINE __inline
            #define INLINE_MEMBER __inline
        #else
            #define S3E_INLINE static inline
            #define INLINE_MEMBER __inline
        #endif
    #endif
#elif defined _MSC_VER
    #define S3E_INLINE __forceinline
    #define INLINE_MEMBER __forceinline
#elif defined __GNUC__
    #define S3E_INLINE static inline
    #define INLINE_MEMBER inline
#else
    #define S3E_INLINE inline
    #define INLINE_MEMBER inline
#endif

#if !defined INLINE && !defined S3E_NO_INLINE
    #define INLINE S3E_INLINE
#endif


/*
 * Compiler Optimisations
 */
#ifdef __ARMCC_VERSION
    #define S3E_FUNCTION_PURE  __pure
#elif _MSC_VER
    #define S3E_FUNCTION_PURE
#elif defined __GNUC__
    #define S3E_FUNCTION_PURE /*__attribute__ ((pure))*/
#else
    #define S3E_FUNCTION_PURE
#endif

// Optimisation which improves stack usage
// on function returning 4 words or fewer.
// This is pertinent for vectors.
// This also improves the chance of inlining.
#ifdef __ARMCC_VERSION
    // Function qualifiers
    #define VALUE_IN_REGS  __value_in_regs
#elif _MSC_VER
    #define VALUE_IN_REGS
#elif defined __GNUC__
    #define VALUE_IN_REGS /*todo*/
#else
    #define VALUE_IN_REGS
#endif

#ifdef __cplusplus
    #define S3E_EXTERN_C extern "C"
    #define S3E_DEFAULT(X) =X
    #define S3E_BEGIN_C_DECL extern "C" {
    #define S3E_END_C_DECL }
#else
    #define S3E_EXTERN_C
    #define S3E_DEFAULT(X)
    #define S3E_BEGIN_C_DECL
    #define S3E_END_C_DECL
#endif

#if __GNUC__ >= 4
    #define S3E_DLL_EXPORT __attribute__ ((visibility("default")))
    #define S3E_DLL_IMPORT
#elif defined _MSC_VER
    #define S3E_DLL_EXPORT __declspec(dllexport)
    #define S3E_DLL_IMPORT __declspec(dllimport)
#else
    #define S3E_DLL_EXPORT
    #define S3E_DLL_IMPORT
#endif


/**
 * everything built into the same binary
 * @par Required Header Files
 * s3eTypes.h
 */
#define S3E_API S3E_EXTERN_C
#define OS_API S3E_EXTERN_C

#define S3E_EAPI S3E_EXTERN_C

#define S3E_EXTAPI S3E_API

#define S3E_MAIN_DECL S3E_EXTERN_C

#ifdef __ARMCC_VERSION
    #define IW_UNALIGNED __packed
    #define IW_UNALIGNED_END
#elif __GNUC__
    #define IW_UNALIGNED
    #define IW_UNALIGNED_END __attribute__ ((packed))
#else
    #define IW_UNALIGNED
    #define IW_UNALIGNED_END
#endif

#ifdef __GNUC__
    #define IW_ALIGNED(X) __attribute__ ((aligned (X)))
    #define IW_PACKED(X) X __attribute__ ((packed))
#elif __ARMCC_VERSION
    #define IW_ALIGNED(X) __align(X)
    #define IW_PACKED(X) __packed X
#else
    // VC 2005 supports this: #define IW_ALIGNED(X) __declspec(align(X))
    #define IW_ALIGNED(X)
    #define IW_PACKED(X) X
#endif

/* for printf-style compiler warnings */
#if defined __GNUC__ && !defined printf
    #define PRINTFLIKE __attribute__((format(printf, 1, 2)))
    #define PRINTFLIKE2 __attribute__((format(printf, 2, 3)))
    #define PRINTFLIKE3 __attribute__((format(printf, 3, 4)))
    #define PRINTFLIKE4 __attribute__((format(printf, 4, 5)))
#else
    #define PRINTFLIKE
    #define PRINTFLIKE2
    #define PRINTFLIKE3
    #define PRINTFLIKE4
#endif /* !__GNUC__ */

#define STATIC static

/* for functions that don't return */
#if __GNUC__ >= 3 && !defined S3E_USE_PTRACE && !defined I3D_OS_BADA && !defined I3D_OS_NACL
    #define NO_RETURN __attribute__ ((noreturn))
#else
    #define NO_RETURN
#endif

#if 1
#define TRACE_HERE() printf(__FILE__ " Line %d\n", __LINE__)
#else
#define TRACE_HERE()
#endif

// \endcond
/** @} */
/** @} */
/** @defgroup legacydeprecations */

#endif /* !S3E_TYPES_H */
