////////////////////////////////
/// usage : 1.	switches for functionalities.
///         2.	every header file should include this file first (except files in Lib).
/// 
/// note  : 1.	tag macros with [on/off] to indicate prefered state.
////////////////////////////////

#ifndef AGAIN_SZX_MP_SOLVER_CONFIG_H
#define AGAIN_SZX_MP_SOLVER_CONFIG_H


#pragma region PlatformCheck
#ifdef _MSC_VER // use (_WIN16 || _WIN32 || _WIN64)?
#define _OS_MS_WINDOWS  1
#define _CC_MSVC  1
#else
#define _OS_MS_WINDOWS  0
#define _CC_MSVC  0
#endif // _MSC_VER

#ifdef __unix__
#define _OS_UNIX  1
#else
#define _OS_UNIX  0
#endif // __unix__

#ifdef __linux__
#define _OS_GNU_LINUX  1
#else
#define _OS_GNU_LINUX  0
#endif // __linux__

#ifdef __MACH__ // use __APPLE__?
#define _OS_APPLE_MAC  1
#else
#define _OS_APPLE_MAC  0
#endif // __MACH__

#ifdef __GNUC__
#define _CC_GNU_GCC  1
#else
#define _CC_GNU_GCC  0
#endif // __GNUC__

#ifdef __llvm__
#define _CC_LLVM  1
#else
#define _CC_LLVM  0
#endif // __llvm__

#ifdef __clang__
#define _CC_CLANG  1
#else
#define _CC_CLANG  0
#endif // __clang__
#pragma endregion PlatformCheck

#pragma region LinkLibraryCheck
#if (_DLL || _SHARED) && !(_STATIC) // prefer static when both are (un)defined.
#define _LL_DYNAMIC  1
#else
#define _LL_STATIC  1
#endif // _DLL

#if (_DEBUG || DEBUG) && !(NDEBUG || _NDEBUG || RELEASE || _RELEASE) // prefer release when both are (un)defined.
#define _DR_DEBUG  1
#else
#define _DR_RELEASE  1
#endif // _DEBUG
#pragma endregion LinkLibraryCheck


#pragma region DebugHelper
// [off] activate test codes.
#define SZX_DEBUG  1

#if SZX_DEBUG
// [off] print additional debug information. check DebugLog.h for fine-grained control.
#define SZX_DEBUG_LOG  1

// [off] additional check for data consistency.
#define SZX_DEBUG_TEST  1
#endif // SZX_DEBUG
#pragma endregion DebugHelper


#pragma region SolverBehavior
#pragma endregion SolverBehavior


#endif // AGAIN_SZX_MP_SOLVER_CONFIG_H
