/******************** (C) COPYRIGHT 2009 STMicroelectronics ********************
* File Name          : mass_mal.c
* Author             : MCD Application Team
* Version            : V3.1.0
* Date               : 10/30/2009
* Description        : Medium Access Layer interface
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "circle.h"

#if SDCARD_SDIO
 #include "sdcard.h"
#else
 #include "msd.h"
#endif 
#include "mass_mal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t Mass_Memory_Size[2];
uint32_t Mass_Block_Size[2];
uint32_t Mass_Block_Count[2];
__IO uint32_t MassStatus = 0;

#if SDCARD_SDIO
SD_CardInfo SDCardInfo;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
NODEBUG uint16_t MAL_Init(uint8_t lun)
{
  uint16_t status = MAL_OK;

  switch (lun)
  {
    case 0:
#if SDCARD_SDIO
      MassStatus = SD_Init();
      MassStatus = SD_GetCardInfo(&SDCardInfo);
      MassStatus = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      MassStatus = SD_EnableWideBusOperation(SDIO_BusWide_4b);
      MassStatus = SD_SetDeviceMode(SD_DMA_MODE);
#else
      MSD_Init();
#endif
      break;

    default:
      return MAL_FAIL;
  }
  return status;
}
/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
NODEBUG uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case 0:
#if SDCARD_SDIO
      MassStatus = SD_WriteBlock(Memory_Offset, Writebuff, Transfer_Length);
      if ( MassStatus != SD_OK )
      {
        return MAL_FAIL;
      }      
#else
      MSD_WriteBlock((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
#endif
      break;

    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
NODEBUG uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{

  switch (lun)
  {
    case 0:
#if SDCARD_SDIO
      MassStatus = SD_ReadBlock(Memory_Offset, Readbuff, Transfer_Length);
      if ( MassStatus != SD_OK )
      {
        return MAL_FAIL;
      }
#else
      MSD_ReadBlock((uint8_t*)Readbuff, Memory_Offset, Transfer_Length);
#endif
      break;
    default:
      return MAL_FAIL;
  }
  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
NODEBUG uint16_t MAL_GetStatus (uint8_t lun)
{
#if SDCARD_SDIO
  uint32_t DeviceSizeMul = 0, NumberOfBlocks = 0;
#else
  uint32_t temp_block_mul = 0;
  sMSD_CSD MSD_csd;
  uint32_t DeviceSizeMul = 0;
#endif


  if (lun == 0)
  {
#if SDCARD_SDIO
    if (SD_Init() == SD_OK)
    {
      SD_GetCardInfo(&SDCardInfo);
      SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16));
      DeviceSizeMul = (SDCardInfo.SD_csd.DeviceSizeMul + 2);

      if(SDCardInfo.CardType == SDIO_HIGH_CAPACITY_SD_CARD)
      {
        Mass_Block_Count[0] = (SDCardInfo.SD_csd.DeviceSize + 1) * 1024;
      }
      else
      {
        NumberOfBlocks  = ((1 << (SDCardInfo.SD_csd.RdBlockLen)) / 512);
        Mass_Block_Count[0] = ((SDCardInfo.SD_csd.DeviceSize + 1) * (1 << DeviceSizeMul) << (NumberOfBlocks/2));
      }
      Mass_Block_Size[0]  = 512;

      MassStatus = SD_SelectDeselect((uint32_t) (SDCardInfo.RCA << 16)); 
      MassStatus = SD_EnableWideBusOperation(SDIO_BusWide_4b); 
      if ( MassStatus != SD_OK )
      {
        return MAL_FAIL;
      }
       
      MassStatus = SD_SetDeviceMode(SD_DMA_MODE);         
      if ( MassStatus != SD_OK )
      {
        return MAL_FAIL;
      } 
     
#else
    MSD_GetCSDRegister(&MSD_csd);
    DeviceSizeMul = MSD_csd.DeviceSizeMul + 2;
    temp_block_mul = (1 << MSD_csd.RdBlockLen)/ 512;
    Mass_Block_Count[0] = ((MSD_csd.DeviceSize + 1) * (1 << (DeviceSizeMul))) * temp_block_mul;
    Mass_Block_Size[0] = 512;
    Mass_Memory_Size[0] = (Mass_Block_Count[0] * Mass_Block_Size[0]);
#endif
      Mass_Memory_Size[0] = Mass_Block_Count[0] * Mass_Block_Size[0];
//      STM_EVAL_LEDOn(LED2);
      return MAL_OK;

#if SDCARD_SDIO
    }
#endif
  }

  return MAL_FAIL;
}

/******************* (C) COPYRIGHT 2009 STMicroelectronics *****END OF FILE****/
