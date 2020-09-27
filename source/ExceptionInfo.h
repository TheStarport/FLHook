#pragma once

/* Inspired by Dr. Dobb's "Visual C++ Exception-Handling Instrumentation" article.
 * https://www.drdobbs.com/windows/visual-c-exception-handling-instrumentat/184416600
 *
 * Additional struct information from Geoff Chappell's website.
 * http://www.geoffchappell.com/studies/msvc/language/predefined/
 *
 * Modifications based on latest CRT source
 */

#include <windows.h>

struct __vcrt_ptd
{
    // C++ Exception Handling (EH) state
    unsigned long      _NLG_dwCode;      // Required by NLG routines
    unexpected_handler _unexpected;      // unexpected() routine
    void*              _translator;      // S.E. translator
    void*              _purecall;        // called when pure virtual happens
    void*              _curexception;    // current exception
    void*              _curcontext;      // current exception context
    int                _ProcessingThrow; // for uncaught_exception
    void*              _curexcspec;      // for handling exceptions thrown from std::unexpected
    int                _cxxReThrow;      // true if it's a rethrown C++ exception

    #if defined _M_X64 || defined _M_ARM || defined _M_ARM64 || defined _M_HYBRID
    void*              _pExitContext;
    void*              _pUnwindContext;
    void*              _pFrameInfoChain;
    uintptr_t          _ImageBase;
    uintptr_t          _ThrowImageBase;
    void*              _pForeignException;
    int                _CatchStateInParent;   // Used to link together the catch funclet with the parent. During dispatch contains state associated
                                              // with catch in the parent. During unwind represents the current unwind state that is resumed to
                                              // during collided unwind and used to look for handlers of the throwing dtor.
    #elif defined _M_IX86
    void*              _pFrameInfoChain;
    #endif

};

extern "C" __vcrt_ptd* __cdecl __vcrt_getptd();

inline const EXCEPTION_RECORD * GetCurrentExceptionRecord()
{
    auto p = __vcrt_getptd();
    return (EXCEPTION_RECORD *)p->_curexception;
}

inline const _CONTEXT * GetCurrentExceptionContext()
{
    auto p = __vcrt_getptd();
	return (_CONTEXT*)p->_curcontext;
}

//-----------------------------------------------------------------------------------------------------------
// These definitions are based on assembly listings produced by the compiler (/FAs) rather than built-in ones
//-----------------------------------------------------------------------------------------------------------

#pragma pack (push, ehdata, 4)

struct msvc__PMD
{
    int mdisp;
    int pdisp;
    int vdisp;
};

typedef void (*msvc__PMFN) (void);

#pragma warning (disable:4200)
#pragma pack (push, _TypeDescriptor, 8)
struct msvc__TypeDescriptor
{
    const void *pVFTable;
    void *spare;
    char name [];
};
#pragma pack (pop, _TypeDescriptor)
#pragma warning (default:4200)

struct msvc__CatchableType {
    unsigned int properties;
    msvc__TypeDescriptor *pType;
    msvc__PMD thisDisplacement;
    int sizeOrOffset;
    msvc__PMFN copyFunction;
};

#pragma warning (disable:4200)
struct msvc__CatchableTypeArray {
    int nCatchableTypes;
    msvc__CatchableType *arrayOfCatchableTypes [];
};
#pragma warning (default:4200)

struct msvc__ThrowInfo {
    unsigned int attributes;
    msvc__PMFN pmfnUnwind;
    int (__cdecl *pForwardCompat) (...);
    msvc__CatchableTypeArray *pCatchableTypeArray;
};

#pragma pack (pop, ehdata)
