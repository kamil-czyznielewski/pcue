/* =============================================================================
 *
 *  Description: This is a C implementation for Thread lwip_sysboot_threadtype
 *
 * -----------------------------------------------------------------------------
 *  Comments:
 *
 * ===========================================================================*/

/* Get access to any of the VDK features & datatypes used */
#include "lwip_sysboot_threadtype.h"
#include <lwip/cglobals.h>
#include <lwip/kernel_abs.h>
#include <lwip/ADI_TOOLS_IOEThreadType.h>
#include <lwip/sockets.h>
#include <lwip/inet.h>
#include <sys/exception.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <stdio.h>
#include <string.h>

/* Size of the memory block to allocate to the stack */
#define ETHER_STACK_SIZE    1024*1024*1

#define DEBUG_PRINT(msg,result)		{									\
										if(result)						\
											{							\
												printf(msg);			\
												return -1;				\
										}								\
									}
                      
								
/* Initializes the Stack */
int system_init(void);




extern struct net_config_info user_net_config_info[];
//extern  struct network_info  netinfo;




/*
 *  Global static data for various SSL managers 
 */
static ADI_INT_IMASK    imask_storage;
#pragma alignment_region (4)
static u8 devmgr_storage[ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * 2)];
static u8 intmgr_storage[                      (ADI_INT_SECONDARY_MEMORY * 8)];
#pragma alignment_region_end

/*
 *  ADSP-BF533 EZ-KIT Lite w/ USBLAN specific settings 
 */
 
#ifdef __ADSPBF533__
#include <lwip/ADI_ETHER_USBLAN.h>
#define NO_RCVES 1
#define NO_XMITS 1
#pragma alignment_region (4)
char BaseMemSize[ADI_ETHER_MEM_USBLAN_BASE_SIZE];
#pragma alignment_region_end
static int p_period =10;
#endif /* __ADSPBF533__ */

/*
 *  ADSP-BF537 EZ-KIT specific settings 
 */
#ifdef __ADSPBF537__
#include <lwip/ADI_ETHER_BF537.h>
static char hwaddr[6] = { 0xcd, 0xcd, 0xcd, 0xcd, 0xcd, 0xcd };
#define NO_RCVES 8 
#define NO_XMITS 10
#pragma alignment_region (32)
char BaseMemSize[ADI_ETHER_MEM_BF537_BASE_SIZE];
char MemRcve[NO_RCVES*ADI_ETHER_MEM_BF537_PER_RECV];
char MemXmit[NO_XMITS*ADI_ETHER_MEM_BF537_PER_XMIT];
#pragma alignment_region_end
static int p_period =40;
#endif /* __ADSPBF537__ */

ADI_ETHER_SUPPLY_MEM memtable = {
#ifdef __ADSPBF537__
    MemRcve,sizeof(MemRcve),0,
    MemXmit,sizeof(MemXmit),0,
#endif
#ifdef __ADSPBF533__
    0,0,0,
    0,0,0,
#endif
    BaseMemSize,sizeof(BaseMemSize)};

void
lwip_sysboot_threadtype_RunFunction(void **inPtr)
{
	char  ip[32];
	int i;
	
	/* Initializes the TCP/IP Stack and returns */
	if(system_init() == -1)
	{
		printf("Failed to initialize system\n");
		return;
	}
	
	
	user_net_config_info[0].use_dhcp = 0;
	
	/* start stack */
	start_stack();
	
//	sethostaddr(0,"192.168.0.5");
	
	/*
	 * For debug purposes, printf() the IP address to the VisualDSP++
	 * console window.  Likely not needed in final application.
	 */
	 
	memset(ip,0,sizeof(ip));
	if(gethostaddr(0,ip))
	{
		printf("IP ADDRESS: %s\n",ip);
	}
	
	/**
	 *  Add Application Code here 
	 **/
   {
  struct VDK_ThreadCreationBlock TCB = {
                                         (VDK_ThreadType)0,
                                         (VDK_ThreadID)0,
                                         0,
                                         (VDK_Priority)0
                                        };
    // Create application threads.
    TCB.template_id = kCharGen_Server_ThreadType;
    if (VDK_CreateThreadEx(&TCB) == UINT_MAX)
    {
      fprintf(stderr, "boot: failed to create CharGen server thread\n");
      exit(905);
    }
    TCB.template_id = kDiscard_Server_ThreadType;
    if (VDK_CreateThreadEx(&TCB) == UINT_MAX)
    {
      fprintf(stderr, "boot: failed to create Discard server thread\n");
      exit(906);
    }
    TCB.template_id = kEcho_Server_ThreadType;
    if (VDK_CreateThreadEx(&TCB) == UINT_MAX)
    {
      fprintf(stderr, "boot: failed to create Echo server thread\n");
      exit(907);
    }
  }

    /* Put the thread's exit from "main" HERE */
    /* A thread is automatically Destroyed when it exits its run function */
}

int
lwip_sysboot_threadtype_ErrorFunction(void **inPtr)
{

    /* TODO - Put this thread's error handling code HERE */

      /* The default ErrorHandler kills the thread */

	VDK_DestroyThread(VDK_GetThreadID(), false);
	return 0;
}

void
lwip_sysboot_threadtype_InitFunction(void **inPtr, VDK_ThreadCreationBlock *pTCB)
{
    /* Put code to be executed when this thread has just been created HERE */

    /* This routine does NOT run in new thread's context.  Any non-static thread
     *   initialization should be performed at the beginning of "Run()."
     */
}

void
lwip_sysboot_threadtype_DestroyFunction(void **inPtr)
{
    /* Put code to be executed when this thread is destroyed HERE */

    /* This routine does NOT run in the thread's context.  Any VDK API calls
     *   should be performed at the end of "Run()."
     */
}

/******************************************************************
 * 
 *  Function: system_init()
 *  Description:
 *  		Initializes Device Manager, Interrupt
 *  		Manager and the Stack. 
 *   Return:
 *   		Returns 1 upon success -1 upon failure.
 *
 ******************************************************************/
int system_init()
{
	ADI_DEV_MANAGER_HANDLE devmgr_handle;
	unsigned int result;
	u32 response_count,critical_reg;
	ADI_DEV_DEVICE_HANDLE lan_handle;
	char *ether_stack_block;
	u16 phyregs[32];
#ifdef __ADSPBF537__
	const unsigned int mac_address_in_flash = 0x203F0000;
#endif

  
	/* initialize the interrupt manager */

	result = adi_int_Init(
                           intmgr_storage,
                           sizeof(intmgr_storage),
                           &response_count,
                           &critical_reg);
	 
	DEBUG_PRINT("Failed to Initialize interrupt manager\n",result != ADI_DEV_RESULT_SUCCESS);

	/* initialize the device manager */

	result = adi_dev_Init(
                            devmgr_storage,
                            sizeof(devmgr_storage),
                            &response_count,
                            &devmgr_handle,
                            &imask_storage);
	
	DEBUG_PRINT("Failed to Initilize device manager\n",result != ADI_DEV_RESULT_SUCCESS);
  
	/* Initialize the kernel */

	ker_init((void*)0);
  
	/* set thread type for the stack threads */

	ker_set_auxdata((void*)kADI_TOOLS_IOEThreadType);


	
	/* open lan-device */

	result = adi_dev_Open(
                           devmgr_handle,
#if defined(__ADSPBF533__)
                           &ADI_ETHER_USBLAN_Entrypoint,
#elif defined(__ADSPBF537__)
                           &ADI_ETHER_BF537_Entrypoint,
#endif
                           0,
                           NULL,
                           &lan_handle,
                           ADI_DEV_DIRECTION_BIDIRECTIONAL,
                           NULL,
                           NULL,
                           (ADI_DCB_CALLBACK_FN)stack_callback_handler);

	DEBUG_PRINT("Failed to open the lan-device\n",result != ADI_DEV_RESULT_SUCCESS);
 
	/* set the services with in stack */

	set_pli_services(1,&lan_handle);

#ifdef __ADSPBF537__

	/*
	 * Read the EZ-KIT Lite's assigned MAC address, found at address 0x203F0000.
	 * We need to first set the AMGCTL register to allow access to asynchronous
	 * memory. 
	 * 
	 * DMA gets more priority than the processor while accessing SDRAM.
	 */

	*pEBIU_AMGCTL = 0xFF | 0x100;
	

	memcpy ( &hwaddr, (unsigned char *) mac_address_in_flash, sizeof ( hwaddr ) );
	
	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SET_MAC_ADDR,
                              (void*)&hwaddr);
                              
	DEBUG_PRINT("Failed set MAC address\n",result != ADI_DEV_RESULT_SUCCESS);
#endif 
	
	/* supply some memory for the driver */

	result = adi_dev_Control(
                              lan_handle,
                              ADI_ETHER_CMD_SUPPLY_MEM,
                              &memtable);

	DEBUG_PRINT("Failed to supply memory to driver\n",result != ADI_DEV_RESULT_SUCCESS);

	result = adi_dev_Control(
                              lan_handle,
                              ADI_DEV_CMD_SET_DATAFLOW_METHOD,
                              (void*)TRUE);


	/* Initialze the stack with user specified configuration priority -3 and
	 * poll period of p_period msec.  The stack is allocated a memory buffer as well.
	 */
   
	ether_stack_block = (char *) malloc ( ETHER_STACK_SIZE );

	DEBUG_PRINT("Failed to malloc stack \n",!ether_stack_block);
   
	init_stack ( 3, p_period, ETHER_STACK_SIZE, ether_stack_block );
	
	/* Start the MAC */

	result = adi_dev_Control (
								lan_handle,
								ADI_ETHER_CMD_START,
								NULL);

	DEBUG_PRINT("Failed to start the driver\n",result != ADI_DEV_RESULT_SUCCESS);
	
	/* read the PHY controller registers */
	adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);
	
	DEBUG_PRINT("PHY Controller has failed and the board needs power cycled\n",phyregs[1]==0xFFFF);
	
	/* wait for the link to be up */
	if ( (phyregs[1]&0x4) ==0)
	{
		printf("Waiting for the link to be established\n");
		while ( (phyregs[1]&0x4) ==0)
		{		
			adi_dev_Control(lan_handle,ADI_ETHER_CMD_GET_PHY_REGS,phyregs);
		}
	}
	
	printf("Link established\n");
    
	return 1;
}
/* ========================================================================== */
