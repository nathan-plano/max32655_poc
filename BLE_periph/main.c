/*************************************************************************************************/
/*!
*  \file   main.c
*
*  \brief  Main file for periph application.
*
*  Copyright (c) 2013-2019 Arm Ltd. All Rights Reserved.
*
*  Copyright (c) 2019 Packetcraft, Inc.
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*/
/*************************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "wsf_types.h"
#include "wsf_trace.h"
#include "wsf_bufio.h"
#include "wsf_msg.h"
#include "wsf_assert.h"
#include "wsf_buf.h"
#include "wsf_heap.h"
#include "wsf_cs.h"
#include "wsf_timer.h"
#include "wsf_os.h"

#include "sec_api.h"
#include "hci_handler.h"
#include "dm_handler.h"
#include "l2c_handler.h"
#include "att_handler.h"
#include "smp_handler.h"
#include "l2c_api.h"
#include "att_api.h"
#include "smp_api.h"
#include "app_api.h"
#include "hci_core.h"
#include "app_terminal.h"


//#include "global_set_get.h"


#if defined(HCI_TR_EXACTLE) && (HCI_TR_EXACTLE == 1)
#include "ll_init_api.h"
#endif

#include "pal_bb.h"
#include "pal_cfg.h"

#include "periph_api.h"
#include "app_ui.h"


/**************************************************************************************************
  Macros
**************************************************************************************************/

/*! \brief UART TX buffer size */
#define PLATFORM_UART_TERMINAL_BUFFER_SIZE 2048U

/**************************************************************************************************
  Global Variables
**************************************************************************************************/

/*! \brief  Pool runtime configuration. */
static wsfBufPoolDesc_t mainPoolDesc[] = { { 16, 8 }, { 32, 4 }, { 192, 8 }, { 256, 8 } };

#if defined(HCI_TR_EXACTLE) && (HCI_TR_EXACTLE == 1)
static LlRtCfg_t mainLlRtCfg;
#endif

/**************************************************************************************************
  Functions
**************************************************************************************************/

/*! \brief  Stack initialization for app. */
extern void StackInitPeriph(void);

/*************************************************************************************************/
/*!
 *  \brief  RX data received callback.
 *
 *  \param  data     Pointer to received data.
 *  \param  len      Number of bytes received.
 */
/*************************************************************************************************/
static void appRxCallback(uint8_t *data, uint16_t len)
{
    unsigned i;

    /* Print the received data */
    printf("App received: ");

    for (i = 0; i < len; i++) {
        printf("0x%02X ", data[i]);
    }
    printf("\n");

    /* Echo the data back to the central */

    if (!PeriphTXData(data, len)) {
        printf("Error sending data to peer\n");
    }
}

/*************************************************************************************************/
/*!
 *  \brief  Initialize WSF.
 *
 *  \return None.
 */
/*************************************************************************************************/
static void mainWsfInit(void)
{
#if defined(HCI_TR_EXACTLE) && (HCI_TR_EXACTLE == 1)
    /* Configurations must be persistent. */
    static BbRtCfg_t mainBbRtCfg;

    PalBbLoadCfg((PalBbCfg_t *)&mainBbRtCfg);
    LlGetDefaultRunTimeCfg(&mainLlRtCfg);
    PalCfgLoadData(PAL_CFG_ID_LL_PARAM, &mainLlRtCfg.maxAdvSets, sizeof(LlRtCfg_t) - 9);
#endif

    uint32_t memUsed;
    memUsed = WsfBufIoUartInit(WsfHeapGetFreeStartAddress(), PLATFORM_UART_TERMINAL_BUFFER_SIZE);
    WsfHeapAlloc(memUsed);

#if defined(HCI_TR_EXACTLE) && (HCI_TR_EXACTLE == 1)
    /* +12 for message headroom, + 2 event header, +255 maximum parameter length. */
    const uint16_t maxRptBufSize = 12 + 2 + 255;

    /* +12 for message headroom, +4 for header. */
    const uint16_t aclBufSize = 12 + mainLlRtCfg.maxAclLen + 4 + BB_DATA_PDU_TAILROOM;

    /* Adjust buffer allocation based on platform configuration. */
    mainPoolDesc[2].len = maxRptBufSize;
    mainPoolDesc[2].num = mainLlRtCfg.maxAdvReports;
    mainPoolDesc[3].len = aclBufSize;
    mainPoolDesc[3].num = mainLlRtCfg.numTxBufs + mainLlRtCfg.numRxBufs;
#endif

    const uint8_t numPools = sizeof(mainPoolDesc) / sizeof(mainPoolDesc[0]);

    memUsed = WsfBufInit(numPools, mainPoolDesc);
    WsfHeapAlloc(memUsed);
    WsfOsInit();
    WsfTimerInit();
#if (WSF_TOKEN_ENABLED == TRUE) || (WSF_TRACE_ENABLED == TRUE)
    WsfTraceRegisterHandler(WsfBufIoWrite);
    WsfTraceEnable(TRUE);
#endif

    AppTerminalInit();

#if defined(HCI_TR_EXACTLE) && (HCI_TR_EXACTLE == 1)
    LlInitRtCfg_t llCfg = { .pBbRtCfg = &mainBbRtCfg,
                            .wlSizeCfg = 4,
                            .rlSizeCfg = 4,
                            .plSizeCfg = 4,
                            .pLlRtCfg = &mainLlRtCfg,
                            .pFreeMem = WsfHeapGetFreeStartAddress(),
                            .freeMemAvail = WsfHeapCountAvailable() };

    memUsed = LlInit(&llCfg);
    WsfHeapAlloc(memUsed);

    bdAddr_t bdAddr;
    PalCfgLoadData(PAL_CFG_ID_BD_ADDR, bdAddr, sizeof(bdAddr_t));
    LlSetBdAddr((uint8_t *)&bdAddr);
#endif

    StackInitPeriph();
    PeriphStart();

    PeriphRegisterRXCallback(appRxCallback);
}

/*************************************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_device.h"
#include "led.h"
#include "board.h"
#include "mxc_delay.h"
#include "uart.h"
//#include "dma.h"
#include "nvic_table.h"

/***** Definitions *****/
// #define DMA

#define UART_BAUD 115200
#define BUFF_SIZE 12000

/***** Globals *****/
volatile int READ_FLAG;

void UART2_Handler(void)
{


	MXC_UART_AsyncHandler(MXC_UART2);



}

void readCallback(mxc_uart_req_t *req, int error)
{
    READ_FLAG = error;
    printf("dpg\n");
}



#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "mxc_device.h"
#include "mxc_delay.h"
#include "mxc_pins.h"
#include "nvic_table.h"
#include "uart.h"
#include "spi.h"
#include "dma.h"
#include "adxl359.h"
#include "gpio.h"


/***** Definitions *****/
#define SPI_SPEED 5000000 // Bit Rate
#define SPI MXC_SPI1
#define SPI_IRQ SPI1_IRQn
#define MOSI_PIN 21
#define MISO_PIN 22
#define FTHR_Defined 1



/*************************************************************************************************/
/*!
*  \fn     main
*
*  \brief  Entry point for demo software.
*
*  \param  None.
*
*  \return None.
*/
/*************************************************************************************************/
int main(void)
{
    mainWsfInit();

    //uart init stuff
    int error, i, fail = 0;
	uint8_t RxData[BUFF_SIZE];
	uint8_t TxData[BUFF_SIZE];
	mxc_spi_pins_t spi_pins;
	spi_pins.clock = TRUE;
	spi_pins.miso = TRUE;
	spi_pins.mosi = TRUE;
	spi_pins.sdio2 = FALSE;
	spi_pins.sdio3 = FALSE;
	spi_pins.ss0 = TRUE;
	spi_pins.ss1 = FALSE;
	spi_pins.ss2 = FALSE;
	MXC_SPI_Init(SPI, 1, 0, 1, 0, SPI_SPEED, spi_pins);
	MXC_SPI_SetMode(SPI,SPI_MODE_0);
	MXC_SPI_SetWidth(SPI,SPI_WIDTH_STANDARD);
	MXC_SPI_SetDataSize(SPI, 8);
	adxl355_soft_reset(SPI);
	adxl355_soft_reset(SPI);
	adxl355_soft_reset(SPI);
	adxl355_set_filter(SPI,0x30);
	print_registers(SPI);


	adxl355_set_op_mode(SPI,0);
	uint16_t array[24000];
	int data_len=0;
	int set_data;
	set_rx_data(RxData);
	adxl355_init_drdy();




	int create_array = 0;


    while (1) {


    	//
    	while(get_connected_status()==0){
    		wsfOsDispatcher();
    		set_data_len(0);
    	}

    	while(get_data_len()==0) {
			MXC_Delay(100000);
			ask_for_data_len();
			wsfOsDispatcher();
    	}


    	if(set_data==0){
    		data_len= (int)get_data_len()*250;

    		memset(array, 0x0, data_len);

    	    create_data_array(SPI,3, data_len,array);

    	    int k =0;
			 for( int i=0; i<data_len; i++){
				 RxData[k] = (uint8_t) (array[i]&0xff);
				 k++;
				 RxData[k] = (uint8_t) ((array[i]>>8)&0xff);
				 k++;
			}
    		set_send_flag(0);
    		set_data=1;

    		}


    	//model checks here



    	while(get_send_flag()==0 &&get_connected_status()==1){
    				printf("notify\n");
    				MXC_Delay(100000);
    				notify_pc();
    	    		wsfOsDispatcher();
    	    	}

    	while(get_send_flag()==1 && get_connected_status()==1){
    		wsfOsDispatcher();
    	}

    	set_data=0;



        if (!WsfOsActive()) {
            /* No WSF tasks are active, optionally sleep */
        }
    }

    return 0;
}
