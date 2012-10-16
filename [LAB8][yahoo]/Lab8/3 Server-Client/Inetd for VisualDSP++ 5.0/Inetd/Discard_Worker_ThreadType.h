/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread Discard_Worker_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _Discard_Worker_ThreadType_H_
#define _Discard_Worker_ThreadType_H_

#include "VDK.h"

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void Discard_Worker_ThreadType_InitFunction(void**, VDK::Thread::ThreadCreationBlock *);
#else
extern "C" void Discard_Worker_ThreadType_InitFunction(void**, VDK_ThreadCreationBlock *);
#endif
extern "C" void Discard_Worker_ThreadType_DestroyFunction(void**);
extern "C" int  Discard_Worker_ThreadType_ErrorFunction(void**);
extern "C" void Discard_Worker_ThreadType_RunFunction(void**);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>

class Discard_Worker_ThreadType_Wrapper : public VDK::Thread
{
public:
    Discard_Worker_ThreadType_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { 
    // extract the struct address passed in from the main thread
    // and save it in m_DataPtr
      m_DataPtr = t.user_data_ptr;
      Discard_Worker_ThreadType_InitFunction(&m_DataPtr, &t); }

    ~Discard_Worker_ThreadType_Wrapper()
    { Discard_Worker_ThreadType_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return Discard_Worker_ThreadType_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { Discard_Worker_ThreadType_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) Discard_Worker_ThreadType_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _Discard_Worker_ThreadType_H_ */

/* ========================================================================== */
