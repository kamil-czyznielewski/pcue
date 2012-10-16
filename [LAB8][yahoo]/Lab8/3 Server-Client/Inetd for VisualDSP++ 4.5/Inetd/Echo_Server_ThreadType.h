/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread Echo_Server_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _Echo_Server_ThreadType_H_
#define _Echo_Server_ThreadType_H_

#include "VDK.h"

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void Echo_Server_ThreadType_InitFunction(void**, VDK::Thread::ThreadCreationBlock *);
#else
extern "C" void Echo_Server_ThreadType_InitFunction(void**, VDK_ThreadCreationBlock *);
#endif
extern "C" void Echo_Server_ThreadType_DestroyFunction(void**);
extern "C" int  Echo_Server_ThreadType_ErrorFunction(void**);
extern "C" void Echo_Server_ThreadType_RunFunction(void**);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>

class Echo_Server_ThreadType_Wrapper : public VDK::Thread
{
public:
    Echo_Server_ThreadType_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { Echo_Server_ThreadType_InitFunction(&m_DataPtr, &t); }

    ~Echo_Server_ThreadType_Wrapper()
    { Echo_Server_ThreadType_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return Echo_Server_ThreadType_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { Echo_Server_ThreadType_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) Echo_Server_ThreadType_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _Echo_Server_ThreadType_H_ */

/* ========================================================================== */
