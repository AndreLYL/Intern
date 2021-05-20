/***************************************************************************//**
* \file WS_DMA.c
* \version 2.0
*
*  This file provides the source code to the API for the
*  WS_DMA component.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "WS_DMA.h"

#if defined(__cplusplus)
extern "C" {
#endif

/* Generated code */
const cy_stc_dma_descriptor_config_t WS_DMA_Descriptor_1_config =
{
    .retrigger       = CY_DMA_RETRIG_IM,
    .interruptType   = CY_DMA_DESCR_CHAIN,
    .triggerOutType  = CY_DMA_1ELEMENT,
    .channelState    = CY_DMA_CHANNEL_ENABLED,
    .triggerInType   = CY_DMA_1ELEMENT,
    .dataSize        = CY_DMA_BYTE,
    .srcTransferSize = CY_DMA_TRANSFER_SIZE_DATA,
    .dstTransferSize = CY_DMA_TRANSFER_SIZE_WORD,
    .descriptorType  = CY_DMA_1D_TRANSFER,
    .srcAddress      = NULL,
    .dstAddress      = NULL,
    .srcXincrement   = 1L,
    .dstXincrement   = 0L,
    .xCount          = 256UL,
    .srcYincrement   = 0L,
    .dstYincrement   = 0L,
    .yCount          = 1UL,
    .nextDescriptor  = NULL
};

cy_stc_dma_descriptor_t WS_DMA_Descriptor_1 =
{
    .ctl = 0UL,
    .src = 0UL,
    .dst = 0UL,
    .xCtl = 0UL,
    .yCtl = 0UL,
    .nextPtr = 0UL
};


/** WS_DMA_initVar indicates whether the WS_DMA 
*  component has been initialized. The variable is initialized to 0 
*  and set to 1 the first time WS_DMA_Start() is called. This allows 
*  the component to restart without reinitialization after the first 
*  call to the WS_DMA_Start() routine.
*
*  If re-initialization of the component is required, then the 
*  WS_DMA_Init() function can be called before the 
*  WS_DMA_Start() or WS_DMA_ChEnable() function.
*/
uint8 WS_DMA_initVar = 0u;


/*******************************************************************************
* Function Name: WS_DMA_Start
****************************************************************************//**
*
* Based on the settings for descriptor in the customizer this function runs the
* DMA_Descriptor_Init() and then initializes the channel using
* DMA_Chnl_Init(). Enables the WS_DMA block using the DMA_Chnl_Enable().
*  
*******************************************************************************/
void WS_DMA_Start(void const * srcAddress, void const * dstAddress)
{
    if (0U == WS_DMA_initVar)
    {
        WS_DMA_Init();
        WS_DMA_initVar = 1u;
    }
    
    Cy_DMA_Descriptor_SetSrcAddress(&WS_DMA_Descriptor_1, srcAddress);
    Cy_DMA_Descriptor_SetDstAddress(&WS_DMA_Descriptor_1, dstAddress);
    Cy_DMA_Channel_Enable(WS_DMA_HW, WS_DMA_DW_CHANNEL);
}


/*******************************************************************************
* Function Name: WS_DMA_Init
****************************************************************************//**
*
* Based on the settings for the descriptor in the customizer this function runs the
* DMA_Descriptor_Init() and then initializes the channel using
* DMA_Chnl_Init().
*  
*******************************************************************************/
void WS_DMA_Init(void)
{
    cy_stc_dma_channel_config_t channelConfig;

    /* Init all descriptors */
    (void)Cy_DMA_Descriptor_Init(&WS_DMA_Descriptor_1, &WS_DMA_Descriptor_1_config);


    channelConfig.descriptor  = &WS_DMA_Descriptor_1;
    channelConfig.preemptable = WS_DMA_PREEMPTABLE;
    channelConfig.priority    = WS_DMA_PRIORITY;
    channelConfig.enable      = false;
    channelConfig.bufferable  = WS_DMA_BUFFERABLE;

    (void)Cy_DMA_Channel_Init(WS_DMA_HW, WS_DMA_DW_CHANNEL, &channelConfig);

    Cy_DMA_Enable(WS_DMA_HW);
}


#if defined(__cplusplus)
}
#endif

/* [] END OF FILE */
