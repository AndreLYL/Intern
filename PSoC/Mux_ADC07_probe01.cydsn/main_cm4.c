/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include "Thermistor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define WS_ZOFFSET (1)
#define WS_ONE3  (0b110<<24)
#define WS_ZERO3 (0b100<<24)
#define WS_NUM_PIXELS (64)   //changed by lyl
#define WS_SPI_BIT_PER_BIT (3)
#define WS_COLOR_PER_PIXEL (3)
#define WS_BYTES_PER_PIXEL (WS_SPI_BIT_PER_BIT * WS_COLOR_PER_PIXEL)

static uint8_t WS_frameBuffer[WS_NUM_PIXELS*WS_BYTES_PER_PIXEL+WS_ZOFFSET];

// This is the interrupt handler for the WS DMA
// It doesnt do anything... and is just a stub
static void WS_DMAComplete(void)
{
    Cy_DMA_Channel_ClearInterrupt(WS_DMA_HW, WS_DMA_DW_CHANNEL);   
}

// Function WS_DMAConfiguration
// This function sets up the DMA and the descriptors

#define WS_NUM_DESCRIPTORS (sizeof(WS_frameBuffer) / 256 + 1)
static cy_stc_dma_descriptor_t WSDescriptors[WS_NUM_DESCRIPTORS];
static void WS_DMAConfigure(void)
{
    // I copies this structure from the PSoC Creator Component configuration 
    // in generated source
    const cy_stc_dma_descriptor_config_t WS_DMA_Descriptors_config =
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
    .nextDescriptor  = 0
    };

    for(unsigned int i=0;i<WS_NUM_DESCRIPTORS;i++)
    {
        Cy_DMA_Descriptor_Init(&WSDescriptors[i], &WS_DMA_Descriptors_config);
        Cy_DMA_Descriptor_SetSrcAddress(&WSDescriptors[i], (uint8_t *)&WS_frameBuffer[i*256]);
        Cy_DMA_Descriptor_SetDstAddress(&WSDescriptors[i], (void *)&WS_SPI_HW->TX_FIFO_WR);
        Cy_DMA_Descriptor_SetXloopDataCount(&WSDescriptors[i],256); // the last
        Cy_DMA_Descriptor_SetNextDescriptor(&WSDescriptors[i],&WSDescriptors[i+1]);
    }
    
    // The last one needs a bit of change
    Cy_DMA_Descriptor_SetXloopDataCount(&WSDescriptors[WS_NUM_DESCRIPTORS-1],sizeof(WS_frameBuffer)-256*(WS_NUM_DESCRIPTORS-1)); // the last
    Cy_DMA_Descriptor_SetNextDescriptor(&WSDescriptors[WS_NUM_DESCRIPTORS-1],0);
    Cy_DMA_Descriptor_SetChannelState(&WSDescriptors[WS_NUM_DESCRIPTORS-1],CY_DMA_CHANNEL_DISABLED);
 
    
     /* Initialize and enable the interrupt from WS_DMA */
    Cy_SysInt_Init(&WS_DMA_INT_cfg, &WS_DMAComplete);
    NVIC_EnableIRQ(WS_DMA_INT_cfg.intrSrc);
    Cy_DMA_Channel_SetInterruptMask(WS_DMA_HW, WS_DMA_DW_CHANNEL, WS_DMA_INTR_MASK);
    
    Cy_DMA_Enable(WS_DMA_HW);    
}

// Function: WS_DMATrigger
// This function sets up the channel... then enables it to dump the frameBuffer to pixels
void WS_DMATrigger()
{
    cy_stc_dma_channel_config_t channelConfig;	
    channelConfig.descriptor  = &WSDescriptors[0];
    channelConfig.preemptable = WS_DMA_PREEMPTABLE;
    channelConfig.priority    = WS_DMA_PRIORITY;
    channelConfig.enable      = false;
    Cy_DMA_Channel_Init(WS_DMA_HW, WS_DMA_DW_CHANNEL, &channelConfig);
    Cy_DMA_Channel_Enable(WS_DMA_HW,WS_DMA_DW_CHANNEL);
}

// Function: WS_SysTickHandler
// This function is called by the systick timer.  It counts up to 33 and then calls the update
// It also makes sure that the channel is inactive
void WS_SysTickHandler()
{
    static int count=0;
    if((Cy_DMA_Channel_GetStatus(WS_DMA_HW,WS_DMA_DW_CHANNEL) & CY_DMA_INTR_CAUSE_COMPLETION) && count++>32)
    {
        WS_DMATrigger();
        count = 0;
    }
}

// Function: convert3Code
// This function takes an 8-bit value representing a color
// and turns it into a WS2812 bit code... where 1=110 and 0=011
// 1 input byte turns into three output bytes of a uint32_t
uint32_t WS_convert3Code(uint8_t input)
{
    uint32_t rval=0;
    for(int i=0;i<8;i++)
    {
        if(input%2)
        {
            rval |= WS_ONE3;
        }
        else
        {
            rval |= WS_ZERO3;
        }
        rval = rval >> 3;
        
        input = input >> 1;
    }
    return rval;
}

// Function: WS_setRGB
// Takes a position and a three byte rgb value and updates the WS_frameBuffer with the correct 9-bytes
void WS_setRGB(int led,uint8_t red, uint8_t green, uint8_t blue)
{
    
    typedef union {
    uint8_t bytes[4];
    uint32_t word;
    } WS_colorUnion;
    
    WS_colorUnion color;
    color.word = WS_convert3Code(green);
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+WS_ZOFFSET] = color.bytes[2];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+1+WS_ZOFFSET] = color.bytes[1];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+2+WS_ZOFFSET] = color.bytes[0];
    
    color.word = WS_convert3Code(red);
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+3+WS_ZOFFSET] = color.bytes[2];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+4+WS_ZOFFSET] = color.bytes[1];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+5+WS_ZOFFSET] = color.bytes[0];

    color.word = WS_convert3Code(blue);
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+6+WS_ZOFFSET] = color.bytes[2];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+7+WS_ZOFFSET] = color.bytes[1];
    WS_frameBuffer[led*WS_BYTES_PER_PIXEL+8+WS_ZOFFSET] = color.bytes[0];
}

// Function WS_setRange
// Sets all of the pixels from start to end with the red,green,blue value
void WS_setRange(int start, int end, uint8_t red,uint8_t green ,uint8_t blue)
{
    CY_ASSERT(start >= 0);
    CY_ASSERT(start < end);
    CY_ASSERT(end <= WS_NUM_PIXELS-1);
   
    WS_setRGB(start,red,green,blue);
    for(int i=1;i<=end-start;i++)
    {
        memcpy(&WS_frameBuffer[start*WS_BYTES_PER_PIXEL+i*WS_BYTES_PER_PIXEL+WS_ZOFFSET],
        &WS_frameBuffer[start*WS_BYTES_PER_PIXEL+WS_ZOFFSET],WS_BYTES_PER_PIXEL);
    }
}
// Function: WS_runTest
// This function just runs test-asserts against the pixel calculation functions
void WS_runTest()
{
    printf("Size of WS_frameBuffer = %d\n",sizeof(WS_frameBuffer));
 
    // Some unit tests for the convert3Code Function
    printf("Test 0x00 = %0X\n",(unsigned int)WS_convert3Code(0));
    printf("Test 0xFF = %0X\n",(unsigned int)WS_convert3Code(0xFF));
    printf("Test 0x80 = %0X\n",(unsigned int)WS_convert3Code(0x80));
    
    // Make sure that WS_convert3Code does the right thing
    CY_ASSERT(WS_convert3Code(0x00) == 0b00000000100100100100100100100100);
    CY_ASSERT(WS_convert3Code(0x80) == 0b00000000110100100100100100100100);
    CY_ASSERT(WS_convert3Code(0xFF) == 0b00000000110110110110110110110110);
    
 
    CY_ASSERT(WS_NUM_PIXELS>=3); // we are goign to test 3 locations
    // Test the WS_setRGB
    WS_setRGB(0,0x80,0,0xFF);
    
    // 0
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+0] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+1] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+2] == 0b00100100);
    
    // 0x80
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+3] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+4] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+5] == 0b00100100);
    
    // 0xFF
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+6] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+7] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[0*9+WS_ZOFFSET+8] == 0b10110110);

    WS_setRGB(1,0,0xFF,0x80);
 
    // 0xFF
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+0] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+1] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+2] == 0b10110110);

    // 0
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+3] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+4] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+5] == 0b00100100);
    
    // 0x80
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+6] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+7] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[1*9+WS_ZOFFSET+8] == 0b00100100);
    
    WS_setRGB(2,0xFF,0x80,0);
    
    // 0x80
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+0] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+1] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+2] == 0b00100100);
    
    // 0xFF
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+3] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+4] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+5] == 0b10110110);

    // 0
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+6] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+7] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+8] == 0b00100100);

    
    // change the values of the range
    WS_setRGB(2,0,0,0);  
    WS_setRGB(3,0,0,0);  
    WS_setRGB(4,0,0,0);  
    
    // Test the WS_setRange
    WS_setRange(2,4,0xFF,0x80,0);
    
    // 0x80
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+0] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+1] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+2] == 0b00100100);
    
    // 0xFF
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+3] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+4] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+5] == 0b10110110);

    // 0
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+6] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+7] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[2*9+WS_ZOFFSET+8] == 0b00100100);
    
    // 0x80
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+0] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+1] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+2] == 0b00100100);
    
    // 0xFF
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+3] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+4] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+5] == 0b10110110);

    // 0
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+6] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+7] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[3*9+WS_ZOFFSET+8] == 0b00100100);

    // 0x80
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+0] == 0b11010010); 
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+1] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+2] == 0b00100100);
    
    // 0xFF
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+3] == 0b11011011);
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+4] == 0b01101101);
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+5] == 0b10110110);

    // 0
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+6] == 0b10010010);
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+7] == 0b01001001);
    CY_ASSERT(WS_frameBuffer[4*9+WS_ZOFFSET+8] == 0b00100100);

  
    for(int i=0;i<10;i++)
    {
        printf("%02X ",WS_frameBuffer[i]);
    }
    printf("\n");
    
}

// Initializes the RGB frame buffer to RGBRGBRGB...RGB
void WS_initMixColorRGB()
{
    for(int i=0;i<WS_NUM_PIXELS;i++)
    {
        switch(i%3)
        {
            case 0:
                WS_setRGB(i,0x80,0x00,0x00); // red
                break;
            case 1:
                WS_setRGB(i,0x00,0x80,0x00); // green
                break;
            case 2:
                WS_setRGB(i,0x00,0x00,0x80); // blue
                break;
        }
    }
}

// Function: WS_Start
// This function intiaizlies everything... 
void WS_Start()
{
    WS_runTest();
    WS_frameBuffer[0] = 0x00;
    WS_setRange(0,WS_NUM_PIXELS-1,0,0,0); // Initialize everything OFF
    Cy_SCB_SPI_Init(WS_SPI_HW, &WS_SPI_config, &WS_SPI_context);
    Cy_SCB_SPI_Enable(WS_SPI_HW);
    WS_DMAConfigure();
}

//Function: WS_matrixInit
// This function will initialize Everything for the 6 * 6 LED Matrix
//void WS_matrixInit(){
//    WS_Start();
//    WS_DMATrigger();
//    Cy_SysTick_SetCallback(0,WS_SysTickHandler);
//}

// Function: LEDMat_Update
// This function will update the color and brightness of LED Matrix
void Matrix_Update(uint8_t mat_value[6][6][3])
{
    int ledPos=0;
    for(int i=0; i<6; i++)
        for(int j=0; j<6; j++)
        {
            ledPos = i * 8 + j;
            WS_setRGB(ledPos,mat_value[i][j][0], mat_value[i][j][1],mat_value[i][j][2]);
        }
            
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint8_t * Wheel(uint8_t WheelPos) 
{
  static uint8_t RGB[3];
  if(WheelPos < 85) 
  {
    RGB[0] = WheelPos * 3;
    RGB[1] = 255 - WheelPos * 3;
    RGB[2] = 0;
  } 
  else 
  {
    if(WheelPos < 170) 
    {
        WheelPos -= 85;
        RGB[0] = 255 - WheelPos * 3;
        RGB[1] = 0;
        RGB[2] = WheelPos * 3;
    } 
    else 
    {
        WheelPos -= 170;
        RGB[0] = 0;
        RGB[1] = WheelPos * 3;
        RGB[2] = 255 - WheelPos * 3;
    }
  }
   return RGB;
}



uint8_t  mat[6][6][3]; // mat is used to storage RGB value
uint8_t *RGB, R, G, B, base, mode=0;
int main(void)
{
    cy_en_mcwdt_status_t mcwdt_init_status;
    
    __enable_irq(); /* Enable global interrupts. */

    UART_1_Start();
    setvbuf( stdin, NULL, _IONBF, 0 ); 
    printf("Started\n");  
    ADC_1_Start();
    AMux_1_Start();
    AMux_2_Start();
    mcwdt_init_status = MCWDT_0_Init(&MCWDT_0_config);
    if(mcwdt_init_status==CY_MCWDT_SUCCESS)
    {
        printf("Wach dog Started\n"); 
    }
    
    /* Enable the MCWDT_0 counters */
    Cy_MCWDT_Enable(MCWDT_0_HW, CY_MCWDT_CTR0|CY_MCWDT_CTR1|CY_MCWDT_CTR2, \
                    MCWDT_0_TWO_LF_CLK_CYCLES_DELAY);
    
    
    WS_Start();   
    Cy_SysTick_Init(CY_SYSTICK_CLOCK_SOURCE_CLK_IMO,8000);
    Cy_SysTick_Enable();    
    WS_DMATrigger();
    Cy_SysTick_SetCallback(0,WS_SysTickHandler);
    
    float matrix[6][6];
    int16_t countReference;
    
    /* Key press event count value */
	uint32_t event1_cnt, event2_cnt;
    uint32_t counter1_value, counter0_value;
    
    /* The time between two presses of switch */
    double timegap;
    cy_en_sar_status_t status;
    /* Initialize event count value */

    while(1)
    {
        
        printf("Mission Started!\r\n");
        /* Consider previous key press as 1st key press event */
        
        Cy_GPIO_Write(Pin_1_PORT,Pin_1_NUM, 0);  
        for(uint8 i = 0;i < 6;i++)
        {
                     
            AMux_2_FastSelect(i); 
            //CyDelayUs(3);  // 似乎这个Delay是不需要的
            
            
            
            for(uint8 j = 0; j < 6; j++)
            {
                countReference = 0;

                //CyDelay(5);
                //event1_cnt = event2_cnt;
                

                AMux_1_FastSelect(j); 
                

                //CyDelayUs(3);
//                counter0_value = Cy_MCWDT_GetCount(MCWDT_0_HW, CY_MCWDT_COUNTER0);
//                counter1_value = Cy_MCWDT_GetCount(MCWDT_0_HW, CY_MCWDT_COUNTER1);
//                event2_cnt = ((counter1_value<<16) | (counter0_value<<0));            
//                
//                /* Calculate the time between two presses of switch and print on the terminal */
//                /* MCWDT_0 Counter0 and Counter1 are clocked by LFClk = 32.768kHz +/- 0.015% */
//                /* Update code once CDT#282377 fix is available to read cascaded count value */
//                if(event2_cnt>event1_cnt)
//                {
//                    timegap = (event2_cnt - event1_cnt)/32768.0 * 1000;
//                }
//                else
//                {
//                    timegap = 0;
//                }
//                printf("\r\nThe time between one scan = %f ms\r\n", timegap);                    
                
                
                //Cy_SAR_StartConvert(SAR,CY_SAR_START_CONVERT_SINGLE_SHOT);
                 
                ADC_1_StartConvert();                 
                //status = ADC_1_IsEndConversion(CY_SAR_WAIT_FOR_RESULT);
                while(!ADC_1_IsEndConversion(CY_SAR_WAIT_FOR_RESULT));
                ADC_1_StopConvert();
                countReference = ADC_1_GetResult16(0);
                matrix[i][j] = ADC_1_CountsTo_Volts(0, countReference);
                
                
                
                
//                
//                printf(" %.2fv\t", matrix[i][j]);
//                
//
                base = (uint8)((matrix[i][j] + 3.30) / 3.30 * 256);
                B = base > 128 ? base - 50 : base ;
                G = base > 75 ? base - 30 : 0;
                R = base > 170 ? base : 0;
                WS_setRGB(i * 8 + j, R, G, B);

                
            }
            //printf("\r\n");        
        }
        Cy_GPIO_Write(Pin_1_PORT,Pin_1_NUM, 1);


        
    }

}

/* [] END OF FILE */
