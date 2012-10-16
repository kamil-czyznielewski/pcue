/****************** COPYRIGHT (C) 2007-2010 RAISONANCE S.A.S. *****************/
/**
*
* @file     list_spe.c
* @brief    Harware specif functions to handle list with touchscreen.
* @author   YRT
* @date     09/2009
* @Note     Split from list.c
**/
/******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

/// @cond Internal

/* Private defines -----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
DMA_InitTypeDef   DMA_InitStructureRead;
DMA_InitTypeDef   DMA_InitStructureWrite;


/* External variables ---------------------------------------------------------*/
extern color_t bmpTmp [LCD_DMA_SIZE];

/* Private function prototypes -----------------------------------------------*/

/* Public function prototype -------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
*
*                                InitListDMA
*
*******************************************************************************/
/**
*
*  Inititalize the DMA for tranfer LCD <-> memory
*
*  @warning    It's necessary to load the stucture each time before start tranfer
**/
/******************************************************************************/
NODEBUG void InitListDMA( void )
{
    /* DMA Channel 2 Configuration for LCD Reads --------------------------------------------*/
    DMA_DeInit( DMA1_Channel2 );
    DMA_InitStructureRead.DMA_PeripheralBaseAddr  = LCD_DATA_MODE_ADDR;
    DMA_InitStructureRead.DMA_MemoryBaseAddr      = ( u32 ) &bmpTmp;
    DMA_InitStructureRead.DMA_DIR                 = DMA_DIR_PeripheralSRC;
    DMA_InitStructureRead.DMA_BufferSize          = LCD_DMA_SIZE;
    DMA_InitStructureRead.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructureRead.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructureRead.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructureRead.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructureRead.DMA_Mode                = DMA_Mode_Normal;
    DMA_InitStructureRead.DMA_Priority            = DMA_Priority_Low;
    DMA_InitStructureRead.DMA_M2M                 = DMA_M2M_Enable;
    DMA_Init( DMA1_Channel2, &DMA_InitStructureRead );

    /* DMA Channel 3 Configuration for LCD Writes --------------------------------------------*/
    DMA_DeInit( DMA1_Channel3 );
    DMA_InitStructureWrite.DMA_PeripheralBaseAddr  = LCD_DATA_MODE_ADDR;
    DMA_InitStructureWrite.DMA_MemoryBaseAddr      = ( u32 ) &bmpTmp;
    DMA_InitStructureWrite.DMA_DIR                 = DMA_DIR_PeripheralDST;
    DMA_InitStructureWrite.DMA_BufferSize          = LCD_DMA_SIZE;
    DMA_InitStructureWrite.DMA_PeripheralInc       = DMA_PeripheralInc_Disable;
    DMA_InitStructureWrite.DMA_MemoryInc           = DMA_MemoryInc_Enable;
    DMA_InitStructureWrite.DMA_PeripheralDataSize  = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructureWrite.DMA_MemoryDataSize      = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructureWrite.DMA_Mode                = DMA_Mode_Normal;
    DMA_InitStructureWrite.DMA_Priority            = DMA_Priority_Low;
    DMA_InitStructureWrite.DMA_M2M                 = DMA_M2M_Enable;
    DMA_Init( DMA1_Channel3, &DMA_InitStructureWrite );

}

/*******************************************************************************
*
*                                LIST_LCD_RectRead
*
*******************************************************************************/
/**
*
*  Save the pixels of a rectangle part of the LCD into a RAM variable.
*  Use of DMA tranfer.
*
*  @param[in]  x        The horizontal coordinate of the rectangle low left corner.
*  @param[in]  y        The vertical coordinate of the rectangle low left corner.
*  @param[in]  width    The rectangle width in pixels.
*  @param[in]  height   The rectangle height in pixels.
*
*  @warning    One pixel weights 2 bytes.
*  @warning    The (0x0) point in on the low left corner.
*  @warning    The transfer is made by DMA
*  @warning    The transfer should be done in 16 bits format due to the 4 bits shift of the bus
*
**/
/******************************************************************************/
NODEBUG void LIST_LCD_RectRead( coord_t x, coord_t y, coord_t width, coord_t height )
{

    /* Select LCD screen area. */
    LCD_SetRect_For_Cmd( x, y, width, height );

    /* Restore 18 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x66 );

    /* Send LCD RAM read command. */
    LCD_SendLCDCmd( ST7732_RAMRD );

    /* First read byte is dummy!*/
    LCD_ReadLCDData();

    /* Read pixels from LCD screen to memory by DMA*/
    DMA_Cmd( DMA1_Channel2, ENABLE );

    /* Wait end of transfer*/
    while ( !DMA_GetFlagStatus( DMA1_FLAG_TC2 ) )
        {};

    DMA_ClearFlag( DMA1_FLAG_TC2 );
    DMA_Cmd( DMA1_Channel2, DISABLE );
    DMA_Init( DMA1_Channel2, &DMA_InitStructureRead );

    /* Restore 16 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x55 );
}


/*******************************************************************************
*
*                                LIST_DRAW_SetImage
*
*******************************************************************************/
/**
*
*  The provided bitmap is made width * height 2 byte words. Each 2 byte word contains
*  the RGB color of a pixel. Use of DMA transfer.
*
*  @brief      Draw a color bitmap at the provided coordinates.
*  @param[in]  x           The horizontal coordinate of the low left corner of the bitmap.
*  @param[in]  y           The vertical coordinate of the low left corner of the bitmap.
*  @param[in]  width       The bitmap width.
*  @param[in]  height      The bitmap height.
*
*  @warning    The (0x0) point in on the low left corner.
*  @warning    The transfer is made by DMA
*  @warning    The transfer should be done in 16 bits format due to the 4 bits shift of the bus
*
**/
/******************************************************************************/
NODEBUG void LIST_DRAW_SetImage( coord_t x, coord_t y, coord_t width, coord_t height )
{

    /* Select screen area to access.*/
    LCD_SetRect_For_Cmd( x, y, width, height );

    /* Restore 18 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x66 );

    /* Send LCD RAM write command. */
    LCD_SendLCDCmd( ST7732_RAMWR );

    /* Write pixels from memory to LCD screen by DMA*/
    DMA_Cmd( DMA1_Channel3, ENABLE );

    /* Wait end of transfer*/
    while ( !DMA_GetFlagStatus( DMA1_FLAG_TC3 ) )
        {};

    DMA_ClearFlag( DMA1_FLAG_TC3 );
    DMA_Cmd( DMA1_Channel3, DISABLE );
    DMA_Init( DMA1_Channel3, &DMA_InitStructureWrite );

    /* Restore 16 bit color mode */
    LCD_SendLCDCmd( ST7732_COLMOD );
    LCD_SendLCDData( 0x55 );

}


/// @endcond

