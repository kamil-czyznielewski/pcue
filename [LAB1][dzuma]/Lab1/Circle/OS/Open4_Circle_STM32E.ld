/***********************************************************
*
* 
*      Open4 CircleOS linker script for STM32F103_512K_64K
*            Copyright RAISONANCE S.A.S. 2009
*
*
************************************************************/
/*    INCLUDE "STM32F10x_COMMON.ld"     */

/*
Common part of the linker scripts for STR32 devices
Copyright RAISONANCE 2007
You can use, modify and distribute this file freely, but without any waranty.
*/

/* default stack sizes. 

These are used by the startup in order to allocate stacks for the different modes.
*/

/*__Stack_Size = 1024 ;

PROVIDE ( _Stack_Size = __Stack_Size ) ;

__Stack_Init = _estack  - __Stack_Size ;

PROVIDE ( _Stack_Init = __Stack_Init ) ;*/

/*
There will be a link error if there is not this amount of RAM free at the end.
*/
/*_Minimum_Stack_Size = 0x100 ;*/



/*
this sends all unreferenced IRQHandlers to reset
*/


PROVIDE (   Undefined_Handler = 0 ) ;
PROVIDE (   SWI_Handler = 0 ) ;
PROVIDE (   IRQ_Handler = 0 ) ;
PROVIDE (   Prefetch_Handler = 0 ) ;
PROVIDE (   Abort_Handler = 0 ) ;
PROVIDE (   FIQ_Handler = 0 ) ;

PROVIDE (   NMIException = 0 ) ;
PROVIDE (   HardFaultException = 0 ) ;
PROVIDE (   MemManageException = 0 ) ;
PROVIDE (   BusFaultException = 0 ) ;
PROVIDE (   UsageFaultException = 0 ) ;
PROVIDE (   SVCHandler = 0 ) ;
PROVIDE (   DebugMonitor = 0 ) ;
PROVIDE (   PendSVC = 0 ) ;
PROVIDE (   SysTickHandler = 0 ) ;
PROVIDE (   WWDG_IRQHandler = 0 ) ;
PROVIDE (   PVD_IRQHandler = 0 ) ;
PROVIDE (   TAMPER_IRQHandler = 0 ) ;
PROVIDE (   RTC_IRQHandler = 0 ) ;
PROVIDE (   FLASH_IRQHandler = 0 ) ;
PROVIDE (   RCC_IRQHandler = 0 ) ;
PROVIDE (   EXTI0_IRQHandler = 0 ) ;
PROVIDE (   EXTI1_IRQHandler = 0 ) ;
PROVIDE (   EXTI2_IRQHandler = 0 ) ;
PROVIDE (   EXTI3_IRQHandler = 0 ) ;
PROVIDE (   EXTI4_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel1_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel2_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel3_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel4_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel5_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel6_IRQHandler = 0 ) ;
PROVIDE (   DMAChannel7_IRQHandler = 0 ) ;
PROVIDE (   ADC_IRQHandler = 0 ) ;
PROVIDE (   USB_HP_CAN_TX_IRQHandler = 0 ) ;
PROVIDE (   USB_LP_CAN_RX0_IRQHandler = 0 ) ;
PROVIDE (   CAN_RX1_IRQHandler = 0 ) ;
PROVIDE (   CAN_SCE_IRQHandler = 0 ) ;
PROVIDE (   EXTI9_5_IRQHandler = 0 ) ;
PROVIDE (   TIM1_BRK_IRQHandler = 0 ) ;
PROVIDE (   TIM1_UP_IRQHandler = 0 ) ;
PROVIDE (   TIM1_TRG_COM_IRQHandler = 0 ) ;
PROVIDE (   TIM1_CC_IRQHandler = 0 ) ;
PROVIDE (   TIM2_IRQHandler = 0 ) ;
PROVIDE (   TIM3_IRQHandler = 0 ) ;
PROVIDE (   TIM4_IRQHandler = 0 ) ;
PROVIDE (   I2C1_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C1_ER_IRQHandler = 0 ) ;
PROVIDE (   I2C2_EV_IRQHandler = 0 ) ;
PROVIDE (   I2C2_ER_IRQHandler = 0 ) ;
PROVIDE (   SPI1_IRQHandler = 0 ) ;
PROVIDE (   SPI2_IRQHandler = 0 ) ;
PROVIDE (   USART1_IRQHandler = 0 ) ;
PROVIDE (   USART2_IRQHandler = 0 ) ;
PROVIDE (   USART3_IRQHandler = 0 ) ;
PROVIDE (   EXTI15_10_IRQHandler = 0 ) ;
PROVIDE (   RTCAlarm_IRQHandler = 0 ) ;
PROVIDE (   USBWakeUp_IRQHandler = 0 ) ;



/******************************************************************************/
/*                       Peripheral memory map                                */
/******************************************************************************/
/*this allows to compile the ST lib in "non-debug" mode*/


/* Peripheral and SRAM base address in the alias region */
PERIPH_BB_BASE        = 0x42000000;
SRAM_BB_BASE          = 0x22000000;

/* Peripheral and SRAM base address in the bit-band region */
SRAM_BASE             = 0x20000000;
PERIPH_BASE           = 0x40000000;

/* Flash registers base address */
PROVIDE ( FLASH_BASE            = 0x40022000);
/* Flash Option Bytes base address */
PROVIDE ( OB_BASE               = 0x1FFFF800);

/* Peripheral memory map */
APB1PERIPH_BASE      = PERIPH_BASE  ;
APB2PERIPH_BASE      = (PERIPH_BASE + 0x10000) ;
AHBPERIPH_BASE       = (PERIPH_BASE + 0x20000) ;

PROVIDE ( TIM2            = (APB1PERIPH_BASE + 0x0000) ) ;
PROVIDE ( TIM3            = (APB1PERIPH_BASE + 0x0400) ) ;
PROVIDE ( TIM4            = (APB1PERIPH_BASE + 0x0800) ) ;
PROVIDE ( RTC             = (APB1PERIPH_BASE + 0x2800) ) ;
PROVIDE ( WWDG            = (APB1PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( IWDG            = (APB1PERIPH_BASE + 0x3000) ) ;
PROVIDE ( SPI2            = (APB1PERIPH_BASE + 0x3800) ) ;
PROVIDE ( USART2          = (APB1PERIPH_BASE + 0x4400) ) ;
PROVIDE ( USART3          = (APB1PERIPH_BASE + 0x4800) ) ;
PROVIDE ( I2C1            = (APB1PERIPH_BASE + 0x5400) ) ;
PROVIDE ( I2C2            = (APB1PERIPH_BASE + 0x5800) ) ;
PROVIDE ( CAN             = (APB1PERIPH_BASE + 0x6400) ) ;
PROVIDE ( BKP             = (APB1PERIPH_BASE + 0x6C00) ) ;
PROVIDE ( PWR             = (APB1PERIPH_BASE + 0x7000) ) ;

PROVIDE ( AFIO            = (APB2PERIPH_BASE + 0x0000) ) ;
PROVIDE ( EXTI            = (APB2PERIPH_BASE + 0x0400) ) ;
PROVIDE ( GPIOA           = (APB2PERIPH_BASE + 0x0800) ) ;
PROVIDE ( GPIOB           = (APB2PERIPH_BASE + 0x0C00) ) ;
PROVIDE ( GPIOC           = (APB2PERIPH_BASE + 0x1000) ) ;
PROVIDE ( GPIOD           = (APB2PERIPH_BASE + 0x1400) ) ;
PROVIDE ( GPIOE           = (APB2PERIPH_BASE + 0x1800) ) ;
PROVIDE ( ADC1            = (APB2PERIPH_BASE + 0x2400) ) ;
PROVIDE ( ADC2            = (APB2PERIPH_BASE + 0x2800) ) ;
PROVIDE ( TIM1            = (APB2PERIPH_BASE + 0x2C00) ) ;
PROVIDE ( SPI1            = (APB2PERIPH_BASE + 0x3000) ) ;
PROVIDE ( USART1          = (APB2PERIPH_BASE + 0x3800) ) ;

PROVIDE ( DMA             = (AHBPERIPH_BASE + 0x0000) ) ;
PROVIDE ( DMA_Channel1    = (AHBPERIPH_BASE + 0x0008) ) ;
PROVIDE ( DMA_Channel2    = (AHBPERIPH_BASE + 0x001C) ) ;
PROVIDE ( DMA_Channel3    = (AHBPERIPH_BASE + 0x0030) ) ;
PROVIDE ( DMA_Channel4    = (AHBPERIPH_BASE + 0x0044) ) ;
PROVIDE ( DMA_Channel5    = (AHBPERIPH_BASE + 0x0058) ) ;
PROVIDE ( DMA_Channel6    = (AHBPERIPH_BASE + 0x006C) ) ;
PROVIDE ( DMA_Channel7    = (AHBPERIPH_BASE + 0x0080) ) ;
PROVIDE ( RCC             = (AHBPERIPH_BASE + 0x1000) ) ;

/* System Control Space memory map */
SCS_BASE              = 0xE000E000;

PROVIDE ( SysTick         = (SCS_BASE + 0x0010) ) ;
PROVIDE ( NVIC            = (SCS_BASE + 0x0100) ) ;
PROVIDE ( SCB             = (SCS_BASE + 0x0D00) ) ;



/*    INCLUDE "STM32F103_128K_20K_DEF.ld"     */

/*
Linker subscript for STM32F103 definitions with 512K Flash and 64K RAM
Copyright RAISONANCE 2007
!!! This file is automatically generated by RIDE !!!
Do not modify it, as it will be erased at every link.
You can use, copy and distribute this file freely, but without any waranty.
*/

/* Memory Spaces Definitions */
MEMORY
{
 /* !!! WARNING: you cannot change this without changing Circle_platform.h and startup accordingly !!! */
  RAM     (xrw) : ORIGIN = 0x2000D000, LENGTH = 12K
  FLASHB1  (rx) : ORIGIN = 0x08000000, LENGTH = 0
  FLASH    (rx) : ORIGIN = 0x08000000, LENGTH = 48K-256
/* !!! WARNING: you cannot change this without changing Circle_platform.h and startup accordingly !!! */
  FLASHDATA (r) : ORIGIN = 0x08000000+512K-132K, LENGTH = 132K
  EXTMEMB0 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB1 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB2 (rx) : ORIGIN = 0x00000000, LENGTH = 0
  EXTMEMB3 (rx) : ORIGIN = 0x00000000, LENGTH = 0
}


/* higher address of the user mode stack */
_estack = 0x2000D000; 

/*    INCLUDE "sections_FLASH.ld"     */

/*
Common part of the linker scripts for STR71x devices in FLASH mode
(that is, the FLASH is seen at 0)
Copyright RAISONANCE 2005
You can use, modify and distribute thisfile freely, but without any waranty.
*/



/* Sections Definitions */

SECTIONS
{
    /* for Cortex devices, the beginning of the startup code is stored in the .isr_vector section, which goes to FLASH */
    .isr_vector :
    {
	. = ALIGN(4);
        KEEP(*(.isr_vector))            /* Startup code */
	. = ALIGN(4);
    } >FLASH
 
    /* for some STRx devices, the beginning of the startup code is stored in the .flashtext section, which goes to FLASH */
    .flashtext :
    {
	. = ALIGN(4);
        *(.flashtext)            /* Startup code */
	. = ALIGN(4);
    } >FLASH
 

    /* the program code is stored in the .text section, which goes to Flash */
    .rodata :
    {
	    . = ALIGN(4);
	    
        *(.rodata)                 /* read-only data (constants) */
        *(.rodata*)

	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .data secion */
   	 /* _sidata = .; */
    } >FLASHDATA
    
    /* the program code is stored in the .text section, which goes to Flash */
    .non_debuggable_code :
    {
	    . = ALIGN(4);
	    
        *(.non_debuggable_code)                 /* read-only data (constants) */
        *(.non_debuggable_code*)
        *(.text.NVIC_PriorityGroupConfig)       /* ST functions put in non debuggable area */
        *(.text.NVIC_SetVectorTable)        
        *(.text.RCC_RTCCLKConfig)
        *(.text.RCC_RTCCLKCmd)
        *(.text.RTC_WaitForSynchro)
        *(.text.RTC_WaitForLastTask)
        *(.text.RTC_ITConfig)
        *(.text.RTC_SetPrescaler)
        *(.text.RCC_PCLK1Config)
        *(.text.RCC_PCLK2Config)
        *(.text.RCC_ADCCLKConfig)
        *(.text.RCC_USBCLKConfig)
        *(.text.RCC_LSEConfig)
        *(.text.RCC_LSICmd)
        *(.text.RCC_HSEConfig)
        *(.text.RCC_DeInit)
        *(.text.TIM_OC1Init)
        *(.text.TIM_OC2Init)
        *(.text.TIM_ARRPreloadConfig)
        *(.text.TIM_DeInit)
        *(.text.FSMC_NORSRAMInit)
        *(.text.FSMC_NORSRAMCmd)
        *(.text.GPIO_PinRemapConfig)
        *(.text.FLASH_SetLatency)
        *(.text.FLASH_PrefetchBufferCmd)
        *(.text.I2C_Init)      
        *(.text.SPI_Init)
        *(.text.SPI_Cmd)       
        *(.text.ADC_TempSensorVrefintCmd)
        *(.text.SysTick_CLKSourceConfig)
        *(.text.SysTick_SetReload)
        *(.text.SysTick_CounterCmd)
        *(.text.SysTick_ITConfig)
        *(.text.SD_Init)
        *(.text.SD_PowerON)
        *(.text.SD_PowerOFF)
        *(.text.SD_InitializeCards)
        *(.text.SD_GetCardInfo)
        *(.text.SD_EnableWideBusOperation)
        *(.text.SD_SetDeviceMode)
        *(.text.SD_SelectDeselect)
        *(.text.SD_ReadBlock)
        *(.text.SD_ReadMultiBlocks)
        *(.text.SD_WriteBlock)
        *(.text.SD_WriteMultiBlocks)
        *(.text.SD_GetTransferState)
        *(.text.SD_StopTransfer)
        *(.text.SD_Erase)
        *(.text.SD_SendStatus)
        *(.text.SD_SendSDStatus)
        *(.text.SD_ProcessIRQSrc)
        *(.text.CmdError)
        *(.text.CmdResp7Error)
        *(.text.CmdResp1Error)
        *(.text.CmdResp3Error)
        *(.text.CmdResp2Error)
        *(.text.CmdResp6Error)
        *(.text.SDEnWideBus)
        *(.text.IsCardProgramming)
        *(.text.FindSCR)
        *(.text.convert_from_bytes_to_power_of_two)
        *(.text.DMA_TxConfiguration)
        *(.text.DMA_RxConfiguration)
        *(.text.FS_Explorer_Ini)
        *(.text.FS_Explorer)
        *(.text.ADC_RegularChannelConfig)
        *(.text.ADC_ExternalTrigConvCmd)

	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .data section */
   	 _sidata = .;
    } >FLASHDATA
     
    /* the program code is stored in the .text section, which goes to Flash */
    .text :
    {
	    . = ALIGN(4);
	    
        *(.text)                   /* remaining code */
        *(.text.*)                   /* remaining code */
        /* *(.rodata) */                 /* read-only data (constants) */
        /* *(.rodata*) */
        *(.glue_7)
        *(.glue_7t)

	    . = ALIGN(4);
   	 _etext = .;
	    /* This is used by the startup in order to initialize the .data section */
   	 /* _sidata = _etext; */
    } >FLASH
    
 

    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    
    .data  : AT ( _sidata )
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data secion */
        _sdata = . ;
        
        /* We first place the vectors table in RAM */
        KEEP(*(.ram_isr_vector))
        
       
        /* Then the remaining data */
        *(.data)
        *(.data.*)

	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .data section */
   	 _edata = . ;
    } >RAM
    
    _sizedata = _edata - _sdata ;
    _eidata = _sidata + _sizedata ;

    ._checkidata _eidata (NOLOAD) :
    {
        . = . + 1 ;
    } >FLASHDATA

/*
    ._idata :
    {
	    . = ALIGN(4);
        _scheckidata = . ;
        
        . = . + _sizedata ;
        
	    . = ALIGN(4);
        _echeckidata = . ;
    } >FLASHDATA
*/

    /* This is the uninitialized data section */
    .bss (NOLOAD) :
    {
	    . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss secion */
        _sbss = .;
        
        *(.bss)
        *(COMMON)
        
	    . = ALIGN(4);
	    /* This is used by the startup in order to initialize the .bss secion */
   	 _ebss = . ;
    } >RAM
    
    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    
    /* This is the user stack section 
    This is just to check that there is enough RAM left for the User mode stack
    It should generate an error if it's full.
     */
     /*
    ._usrstack :
    {
	    . = ALIGN(4);
        _susrstack = . ;
        
        . = . + _Minimum_Stack_Size ;
        
	    . = ALIGN(4);
        _eusrstack = . ;
    } >RAM
    */

   
    /* this is the FLASH Bank1 */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    .b1text (NOLOAD) :
    {
        FILL ( 0xFFFFFFFF );
        /**(.b1text)*/                 /* remaining code */
        /**(.b1rodata)*/              /* read-only data (constants) */
        /**(.b1rodata*)*/
    } >FLASHB1
    
    /* this is the EXTMEM */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    
    /* EXTMEM Bank0 */
    .eb0text :
    {
        *(.eb0text)                   /* remaining code */
        *(.eb0rodata)                 /* read-only data (constants) */
        *(.eb0rodata*)
    } >EXTMEMB0
    
    /* EXTMEM Bank1 */
    .eb1text :
    {
        *(.eb1text)                   /* remaining code */
        *(.eb1rodata)                 /* read-only data (constants) */
        *(.eb1rodata*)
    } >EXTMEMB1
    
    /* EXTMEM Bank2 */
    .eb2text :
    {
        *(.eb2text)                   /* remaining code */
        *(.eb2rodata)                 /* read-only data (constants) */
        *(.eb2rodata*)
    } >EXTMEMB2
    
    /* EXTMEM Bank0 */
    .eb3text :
    {
        *(.eb3text)                   /* remaining code */
        *(.eb3rodata)                 /* read-only data (constants) */
        *(.eb3rodata*)
    } >EXTMEMB3
    
    
    
    /* after that it's only debugging information. */
    
    /* remove the debugging information from the standard libraries */
    /DISCARD/ :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }
     

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    /* DWARF debug sections.
       Symbols in the DWARF debugging sections are relative to the beginning
       of the section so we begin them at 0.  */
    /* DWARF 1 */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2 */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2 */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
}


