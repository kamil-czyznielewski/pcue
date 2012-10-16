/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread Echo_Worker_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _Echo_Worker_ThreadType_H_
#define _Echo_Worker_ThreadType_H_

#include "VDK.h"

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void Echo_Worker_ThreadType_InitFunction(void**, VDK::Thread::ThreadCreationBlock *);
#else
extern "C" void Echo_Worker_ThreadType_InitFunction(void**, VDK_ThreadCreationBlock *);
#endif
extern "C" void Echo_Worker_ThreadType_DestroyFunction(void**);
extern "C" int  Echo_Worker_ThreadType_ErrorFunction(void**);
extern "C" void Echo_Worker_ThreadType_RunFunction(void**);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>

class Echo_Worker_ThreadType_Wrapper : public VDK::Thread
{
public:
    Echo_Worker_ThreadType_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { 
    // extract the struct address passed in from the main thread
    // and save it in m_DataPtr
      m_DataPtr = t.user_data_ptr;
      Echo_Worker_ThreadType_InitFunction(&m_DataPtr, &t); }

    ~Echo_Worker_ThreadType_Wrapper()
    { Echo_Worker_ThreadType_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return Echo_Worker_ThreadType_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { Echo_Worker_ThreadType_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) Echo_Worker_ThreadType_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _Echo_Worker_ThreadType_H_ */

/* ========================================================================== */
