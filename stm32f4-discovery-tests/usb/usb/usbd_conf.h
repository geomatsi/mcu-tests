/**
  ******************************************************************************
  * @file    usbd_conf.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    19-September-2011
  * @brief   USB Device configuration file
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__


#include "stm32f4_discovery.h"

#define USBD_CFG_MAX_NUM		1
#define USBD_ITF_MAX_NUM		1

#define USB_MAX_STR_DESC_SIZ	255

#define CDC_IN_EP		0x81
#define CDC_OUT_EP		0x01
#define CDC_CMD_EP		0x82

#define CDC_DATA_MAX_PACKET_SIZE	64
#define CDC_CMD_PACKET_SZE			8
#define CDC_IN_FRAME_INTERVAL		5
#define APP_RX_DATA_SIZE			2048

#define APP_FOPS	VCP_fops

#endif /* __USBD_CONF__H__ */
