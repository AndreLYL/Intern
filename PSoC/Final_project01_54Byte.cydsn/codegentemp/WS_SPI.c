/***************************************************************************//**
* \file WS_SPI.c
* \version 2.0
*
*  This file provides the source code to the API for the SPI Component.
*
********************************************************************************
* \copyright
* Copyright 2016-2017, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "WS_SPI.h"
#include "sysint/cy_sysint.h"
#include "cyfitter_sysint.h"
#include "cyfitter_sysint_cfg.h"

#if defined(__cplusplus)
extern "C" {
#endif

/***************************************
*     Global variables
***************************************/

/** WS_SPI_initVar indicates whether the WS_SPI
*  component has been initialized. The variable is initialized to 0
*  and set to 1 the first time WS_SPI_Start() is called.
*  This allows  the component to restart without reinitialization
*  after the first call to the WS_SPI_Start() routine.
*
*  If re-initialization of the component is required, then the
*  WS_SPI_Init() function can be called before the
*  WS_SPI_Start() or WS_SPI_Enable() function.
*/
uint8_t WS_SPI_initVar = 0U;

/** The instance-specific configuration structure.
* The pointer to this structure should be passed to Cy_SCB_SPI_Init function
* to initialize component with GUI selected settings.
*/
cy_stc_scb_spi_config_t const WS_SPI_config =
{
    .spiMode  = CY_SCB_SPI_MASTER,
    .subMode  = CY_SCB_SPI_MOTOROLA,
    .sclkMode = CY_SCB_SPI_CPHA1_CPOL1,

    .oversample = 4UL,

    .rxDataWidth              = 8UL,
    .txDataWidth              = 8UL,
    .enableMsbFirst           = true,
    .enableInputFilter        = false,

    .enableFreeRunSclk        = false,
    .enableMisoLateSample     = false,
    .enableTransferSeperation = false,
    .ssPolarity               = ((((uint32_t) CY_SCB_SPI_ACTIVE_LOW) << WS_SPI_SPI_SLAVE_SELECT0) | \
                                 (((uint32_t) CY_SCB_SPI_ACTIVE_LOW) << WS_SPI_SPI_SLAVE_SELECT1) | \
                                 (((uint32_t) CY_SCB_SPI_ACTIVE_LOW) << WS_SPI_SPI_SLAVE_SELECT2) | \
                                 (((uint32_t) CY_SCB_SPI_ACTIVE_LOW) << WS_SPI_SPI_SLAVE_SELECT3)),

    .enableWakeFromSleep = false,

    .rxFifoTriggerLevel  = 0UL,
    .rxFifoIntEnableMask = 0x0UL,

    .txFifoTriggerLevel  = 63UL,
    .txFifoIntEnableMask = 0x0UL,

    .masterSlaveIntEnableMask = 0x0UL
};

/** The instance-specific context structure.
* It is used while the driver operation for internal configuration and
* data keeping for the SPI. The user should not modify anything in this 
* structure.
*/
cy_stc_scb_spi_context_t WS_SPI_context;


/*******************************************************************************
* Function Name: WS_SPI_Start
****************************************************************************//**
*
* Invokes WS_SPI_Init() and WS_SPI_Enable().
* Also configures interrupt if it is internal.
* After this function call the component is enabled and ready for operation.
* This is the preferred method to begin component operation.
*
* \globalvars
* \ref WS_SPI_initVar - used to check initial configuration,
* modified  on first function call.
*
*******************************************************************************/
void WS_SPI_Start(void)
{
    if (0U == WS_SPI_initVar)
    {
        /* Configure component */
        (void) Cy_SCB_SPI_Init(WS_SPI_HW, &WS_SPI_config, &WS_SPI_context);

        /* Set active slave select to line 0 */
        Cy_SCB_SPI_SetActiveSlaveSelect(WS_SPI_HW, WS_SPI_SPI_SLAVE_SELECT0);

        /* Hook interrupt service routine */
    #if defined(WS_SPI_SCB_IRQ__INTC_ASSIGNED)
        (void) Cy_SysInt_Init(&WS_SPI_SCB_IRQ_cfg, &WS_SPI_Interrupt);
    #endif /* (WS_SPI_SCB_IRQ__INTC_ASSIGNED) */

        /* Component is configured */
        WS_SPI_initVar = 1U;
    }

    /* Enable interrupt in NVIC */
#if defined(WS_SPI_SCB_IRQ__INTC_ASSIGNED)
    NVIC_EnableIRQ((IRQn_Type) WS_SPI_SCB_IRQ_cfg.intrSrc);
#endif /* (WS_SPI_SCB_IRQ__INTC_ASSIGNED) */

    Cy_SCB_SPI_Enable(WS_SPI_HW);
}

#if defined(__cplusplus)
}
#endif


/* [] END OF FILE */
