/***************************************************************************//**
* \file WS_SPI.h
* \version 2.0
*
*  This file provides constants and parameter values for the SPI component.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(WS_SPI_CY_SCB_SPI_PDL_H)
#define WS_SPI_CY_SCB_SPI_PDL_H

#include "cyfitter.h"
#include "scb/cy_scb_spi.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*        Function Prototypes
***************************************/
/**
* \addtogroup group_general
* @{
*/
/* Component specific functions. */
void WS_SPI_Start(void);

/* Basic functions. */
__STATIC_INLINE cy_en_scb_spi_status_t WS_SPI_Init(cy_stc_scb_spi_config_t const *config);
__STATIC_INLINE void WS_SPI_DeInit(void);
__STATIC_INLINE void WS_SPI_Enable(void);
__STATIC_INLINE void WS_SPI_Disable(void);

/* Register callback. */
__STATIC_INLINE void WS_SPI_RegisterCallback(cy_cb_scb_spi_handle_events_t callback);

/* Bus state. */
__STATIC_INLINE bool WS_SPI_IsBusBusy(void);

/* Slave select control. */
__STATIC_INLINE void WS_SPI_SetActiveSlaveSelect(cy_en_scb_spi_slave_select_t slaveSelect);
__STATIC_INLINE void WS_SPI_SetActiveSlaveSelectPolarity(cy_en_scb_spi_slave_select_t slaveSelect, 
                                                                   cy_en_scb_spi_polarity_t polarity);

/* Low level: read. */
__STATIC_INLINE uint32_t WS_SPI_Read(void);
__STATIC_INLINE uint32_t WS_SPI_ReadArray(void *buffer, uint32_t size);
__STATIC_INLINE uint32_t WS_SPI_GetRxFifoStatus(void);
__STATIC_INLINE void     WS_SPI_ClearRxFifoStatus(uint32_t clearMask);
__STATIC_INLINE uint32_t WS_SPI_GetNumInRxFifo(void);
__STATIC_INLINE void     WS_SPI_ClearRxFifo(void);

/* Low level: write. */
__STATIC_INLINE uint32_t WS_SPI_Write(uint32_t data);
__STATIC_INLINE uint32_t WS_SPI_WriteArray(void *buffer, uint32_t size);
__STATIC_INLINE void     WS_SPI_WriteArrayBlocking(void *buffer, uint32_t size);
__STATIC_INLINE uint32_t WS_SPI_GetTxFifoStatus(void);
__STATIC_INLINE void     WS_SPI_ClearTxFifoStatus(uint32_t clearMask);
__STATIC_INLINE uint32_t WS_SPI_GetNumInTxFifo(void);
__STATIC_INLINE bool     WS_SPI_IsTxComplete(void);
__STATIC_INLINE void     WS_SPI_ClearTxFifo(void);

/* Master/slave specific status. */
__STATIC_INLINE uint32_t WS_SPI_GetSlaveMasterStatus(void);
__STATIC_INLINE void     WS_SPI_ClearSlaveMasterStatus(uint32_t clearMask);

/* High level: transfer functions. */
__STATIC_INLINE cy_en_scb_spi_status_t WS_SPI_Transfer(void *txBuffer, void *rxBuffer, uint32_t size);
__STATIC_INLINE void     WS_SPI_AbortTransfer(void);
__STATIC_INLINE uint32_t WS_SPI_GetTransferStatus(void);
__STATIC_INLINE uint32_t WS_SPI_GetNumTransfered(void);

/* Interrupt handler */
__STATIC_INLINE void WS_SPI_Interrupt(void);
/** @} group_general */


/***************************************
*    Variables with External Linkage
***************************************/
/**
* \addtogroup group_globals
* @{
*/
extern uint8_t WS_SPI_initVar;
extern cy_stc_scb_spi_config_t const WS_SPI_config;
extern cy_stc_scb_spi_context_t WS_SPI_context;
/** @} group_globals */


/***************************************
*         Preprocessor Macros
***************************************/
/**
* \addtogroup group_macros
* @{
*/
/** The pointer to the base address of the hardware */
#define WS_SPI_HW     ((CySCB_Type *) WS_SPI_SCB__HW)

/** The slave select line 0 constant which takes into account pin placement */
#define WS_SPI_SPI_SLAVE_SELECT0    ( (cy_en_scb_spi_slave_select_t) WS_SPI_SCB__SS0_POSITION)

/** The slave select line 1 constant which takes into account pin placement */
#define WS_SPI_SPI_SLAVE_SELECT1    ( (cy_en_scb_spi_slave_select_t) WS_SPI_SCB__SS1_POSITION)

/** The slave select line 2 constant which takes into account pin placement */
#define WS_SPI_SPI_SLAVE_SELECT2    ( (cy_en_scb_spi_slave_select_t) WS_SPI_SCB__SS2_POSITION)

/** The slave select line 3 constant which takes into account pin placement */
#define WS_SPI_SPI_SLAVE_SELECT3    ((cy_en_scb_spi_slave_select_t) WS_SPI_SCB__SS3_POSITION)
/** @} group_macros */


/***************************************
*    In-line Function Implementation
***************************************/

/*******************************************************************************
* Function Name: WS_SPI_Init
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Init() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE cy_en_scb_spi_status_t WS_SPI_Init(cy_stc_scb_spi_config_t const *config)
{
    return Cy_SCB_SPI_Init(WS_SPI_HW, config, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_DeInit
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_DeInit() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_DeInit(void)
{
    Cy_SCB_SPI_DeInit(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_Enable
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Enable() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_Enable(void)
{
    Cy_SCB_SPI_Enable(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_Disable
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Disable() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_Disable(void)
{
    Cy_SCB_SPI_Disable(WS_SPI_HW, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_RegisterCallback
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_RegisterCallback() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_RegisterCallback(cy_cb_scb_spi_handle_events_t callback)
{
    Cy_SCB_SPI_RegisterCallback(WS_SPI_HW, callback, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_IsBusBusy
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_IsBusBusy() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE bool WS_SPI_IsBusBusy(void)
{
    return Cy_SCB_SPI_IsBusBusy(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_SetActiveSlaveSelect
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_SetActiveSlaveSelect() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_SetActiveSlaveSelect(cy_en_scb_spi_slave_select_t slaveSelect)
{
    Cy_SCB_SPI_SetActiveSlaveSelect(WS_SPI_HW, slaveSelect);
}


/*******************************************************************************
* Function Name: WS_SPI_SetActiveSlaveSelectPolarity
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_SetActiveSlaveSelectPolarity() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_SetActiveSlaveSelectPolarity(cy_en_scb_spi_slave_select_t slaveSelect, 
                                                                   cy_en_scb_spi_polarity_t polarity)
{
    Cy_SCB_SPI_SetActiveSlaveSelectPolarity(WS_SPI_HW, slaveSelect, polarity);
}


/*******************************************************************************
* Function Name: WS_SPI_Read
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Read() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_Read(void)
{
    return Cy_SCB_SPI_Read(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ReadArray
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ReadArray() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_ReadArray(void *buffer, uint32_t size)
{
    return Cy_SCB_SPI_ReadArray(WS_SPI_HW, buffer, size);
}


/*******************************************************************************
* Function Name: WS_SPI_GetRxFifoStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetRxFifoStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetRxFifoStatus(void)
{
    return Cy_SCB_SPI_GetRxFifoStatus(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ClearRxFifoStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ClearRxFifoStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_ClearRxFifoStatus(uint32_t clearMask)
{
    Cy_SCB_SPI_ClearRxFifoStatus(WS_SPI_HW, clearMask);
}


/*******************************************************************************
* Function Name: WS_SPI_GetNumInRxFifo
****************************************************************************//**
*
* Invokes the Cy_SCB_GetNumInRxFifo() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetNumInRxFifo(void)
{
    return Cy_SCB_GetNumInRxFifo(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ClearRxFifo
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ClearRxFifo() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_ClearRxFifo(void)
{
    Cy_SCB_SPI_ClearRxFifo(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_Write
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Write() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_Write(uint32_t data)
{
    return Cy_SCB_SPI_Write(WS_SPI_HW, data);
}


/*******************************************************************************
* Function Name: WS_SPI_WriteArray
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_WriteArray() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_WriteArray(void *buffer, uint32_t size)
{
    return Cy_SCB_SPI_WriteArray(WS_SPI_HW, buffer, size);
}


/*******************************************************************************
* Function Name: WS_SPI_WriteArrayBlocking
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_WriteArrayBlocking() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_WriteArrayBlocking(void *buffer, uint32_t size)
{
    Cy_SCB_SPI_WriteArrayBlocking(WS_SPI_HW, buffer, size);
}


/*******************************************************************************
* Function Name: WS_SPI_GetTxFifoStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetTxFifoStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetTxFifoStatus(void)
{
    return Cy_SCB_SPI_GetTxFifoStatus(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ClearTxFifoStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ClearTxFifoStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_ClearTxFifoStatus(uint32_t clearMask)
{
    Cy_SCB_SPI_ClearTxFifoStatus(WS_SPI_HW, clearMask);
}


/*******************************************************************************
* Function Name: WS_SPI_GetNumInTxFifo
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetNumInTxFifo() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetNumInTxFifo(void)
{
    return Cy_SCB_SPI_GetNumInTxFifo(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_IsTxComplete
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_IsTxComplete() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE bool WS_SPI_IsTxComplete(void)
{
    return Cy_SCB_SPI_IsTxComplete(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ClearTxFifo
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ClearTxFifo() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_ClearTxFifo(void)
{
    Cy_SCB_SPI_ClearTxFifo(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_GetSlaveMasterStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetSlaveMasterStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetSlaveMasterStatus(void)
{
    return Cy_SCB_SPI_GetSlaveMasterStatus(WS_SPI_HW);
}


/*******************************************************************************
* Function Name: WS_SPI_ClearSlaveMasterStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_ClearSlaveMasterStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_ClearSlaveMasterStatus(uint32_t clearMask)
{
    Cy_SCB_SPI_ClearSlaveMasterStatus(WS_SPI_HW, clearMask);
}


/*******************************************************************************
* Function Name: WS_SPI_Transfer
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Transfer() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE cy_en_scb_spi_status_t WS_SPI_Transfer(void *txBuffer, void *rxBuffer, uint32_t size)
{
    return Cy_SCB_SPI_Transfer(WS_SPI_HW, txBuffer, rxBuffer, size, &WS_SPI_context);
}

/*******************************************************************************
* Function Name: WS_SPI_AbortTransfer
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_AbortTransfer() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_AbortTransfer(void)
{
    Cy_SCB_SPI_AbortTransfer(WS_SPI_HW, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_GetTransferStatus
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetTransferStatus() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetTransferStatus(void)
{
    return Cy_SCB_SPI_GetTransferStatus(WS_SPI_HW, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_GetNumTransfered
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_GetNumTransfered() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE uint32_t WS_SPI_GetNumTransfered(void)
{
    return Cy_SCB_SPI_GetNumTransfered(WS_SPI_HW, &WS_SPI_context);
}


/*******************************************************************************
* Function Name: WS_SPI_Interrupt
****************************************************************************//**
*
* Invokes the Cy_SCB_SPI_Interrupt() PDL driver function.
*
*******************************************************************************/
__STATIC_INLINE void WS_SPI_Interrupt(void)
{
    Cy_SCB_SPI_Interrupt(WS_SPI_HW, &WS_SPI_context);
}


#if defined(__cplusplus)
}
#endif

#endif /* WS_SPI_CY_SCB_SPI_PDL_H */


/* [] END OF FILE */
