/* =============================================================================
 *
 *  Description: This is a C++ to C Thread Header file for Thread CharGen_Worker_ThreadType
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

#ifndef _CharGen_Worker_ThreadType_H_
#define _CharGen_Worker_ThreadType_H_

#include "VDK.h"

#ifdef __ECC__	/* for C/C++ access */
#ifdef __cplusplus
extern "C" void CharGen_Worker_ThreadType_InitFunction(void**, VDK::Thread::ThreadCreationBlock *);
#else
extern "C" void CharGen_Worker_ThreadType_InitFunction(void**, VDK_ThreadCreationBlock *);
#endif
extern "C" void CharGen_Worker_ThreadType_DestroyFunction(void**);
extern "C" int  CharGen_Worker_ThreadType_ErrorFunction(void**);
extern "C" void CharGen_Worker_ThreadType_RunFunction(void**);
#endif /* __ECC__ */

#ifdef __cplusplus
#include <new>

class CharGen_Worker_ThreadType_Wrapper : public VDK::Thread
{
public:
    CharGen_Worker_ThreadType_Wrapper(VDK::ThreadCreationBlock &t)
        : VDK::Thread(t)
    { 
    // extract the struct address passed in from the main thread
    // and save it in m_DataPtr
      m_DataPtr = t.user_data_ptr;
      CharGen_Worker_ThreadType_InitFunction(&m_DataPtr, &t); }

    ~CharGen_Worker_ThreadType_Wrapper()
    { CharGen_Worker_ThreadType_DestroyFunction(&m_DataPtr); }

    int ErrorHandler()
    { 
      return CharGen_Worker_ThreadType_ErrorFunction(&m_DataPtr);
     }

    void Run()
    { CharGen_Worker_ThreadType_RunFunction(&m_DataPtr); }

    static VDK::Thread* Create(VDK::Thread::ThreadCreationBlock &t)
    { return new (t) CharGen_Worker_ThreadType_Wrapper(t); }
};

#endif /* __cplusplus */
#endif /* _CharGen_Worker_ThreadType_H_ */

/* ========================================================================== */
