 /**
 * @file    main.c
 * @brief   SPI Master Demo
 * @details Shows Master loopback demo for QSPI0
 *          Read the printf() for instructions
 */

/******************************************************************************
 * Copyright (C) 2022 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 ******************************************************************************/

/***** Includes *****/
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


/***** Definitions *****/
#define SPI_SPEED 100000 // Bit Rate

// Board Selection
//#if defined(BOARD_FTHR_APPS_P1) // Defined in board.h
#define SPI MXC_SPI1
#define SPI_IRQ SPI1_IRQn
#define MOSI_PIN 21
#define MISO_PIN 22
#define FTHR_Defined 1

int main(void)
{
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
	adxl355_init_drdy();
    uint16_t data_out[3000];
    create_data_array(SPI,250, 3000,data_out);
    for(int k =2000; k<2500;k++){
    	printf("%hi \n",data_out[k] );
    }


    return E_NO_ERROR;
}
