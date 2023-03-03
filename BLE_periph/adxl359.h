/*
 * adxl359.h
 *
 *  Created on: Feb 20, 2023
 *      Author: NPlano2
 */

#ifndef ADXL359_H_
#define ADXL359_H_




#include <stdint.h>
#include <string.h>
#include "spi.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* SPI commands */
#define ADXL355_SPI_READ          0x01
#define ADXL355_SPI_WRITE         0x00
#define ADXL355_ADDR(x)			  ((x) & 0xFF)
#define GET_ADXL355_TRANSF_LEN(x) (((x) >>  8) & 0x0000FF)
#define SET_ADXL355_TRANSF_LEN(x) (((x) <<  8) & 0x00FF00)
#define GET_ADXL355_RESET_VAL(x)  (((x) >> 16) & 0x0000FF)
#define SET_ADXL355_RESET_VAL(x)  (((x) << 16) & 0xFF0000)

/* ADXL355 Register Map */
#define ADXL355_DEVID_AD     (ADXL355_ADDR(0x00) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0xAD))
#define ADXL355_DEVID_MST 	 (ADXL355_ADDR(0x01) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x1D))
#define ADXL355_PARTID       (ADXL355_ADDR(0x02) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0xED))
//#define ADXL359_PARTID       (ADXL355_ADDR(0x02) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0xE9))
#define ADXL359_PARTID       (ADXL355_ADDR(0x02) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0xED))
#define ADXL355_REVID 		 (ADXL355_ADDR(0x03) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x01))
#define ADXL355_STATUS       (ADXL355_ADDR(0x04) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_FIFO_ENTRIES (ADXL355_ADDR(0x05) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_TEMP         (ADXL355_ADDR(0x06) | SET_ADXL355_TRANSF_LEN(2))
#define ADXL355_XDATA        (ADXL355_ADDR(0x08) | SET_ADXL355_TRANSF_LEN(3))
#define ADXL355_YDATA        (ADXL355_ADDR(0x0B) | SET_ADXL355_TRANSF_LEN(3))
#define ADXL355_ZDATA        (ADXL355_ADDR(0x0E) | SET_ADXL355_TRANSF_LEN(3))
#define ADXL355_FIFO_DATA    (ADXL355_ADDR(0x11) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_OFFSET_X     (ADXL355_ADDR(0x1E) | SET_ADXL355_TRANSF_LEN(2))
#define ADXL355_OFFSET_Y     (ADXL355_ADDR(0x20) | SET_ADXL355_TRANSF_LEN(2))
#define ADXL355_OFFSET_Z     (ADXL355_ADDR(0x22) | SET_ADXL355_TRANSF_LEN(2))
#define ADXL355_ACT_EN       (ADXL355_ADDR(0x24) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_ACT_THRESH   (ADXL355_ADDR(0x25) | SET_ADXL355_TRANSF_LEN(2))
#define ADXL355_ACT_CNT      (ADXL355_ADDR(0x27) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x01))
#define ADXL355_FILTER       (ADXL355_ADDR(0x28) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_FIFO_SAMPLES (ADXL355_ADDR(0x29) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x60))
#define ADXL355_INT_MAP      (ADXL355_ADDR(0x2A) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_SYNC         (ADXL355_ADDR(0x2B) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_RANGE        (ADXL355_ADDR(0x2C) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x81))
#define ADXL355_POWER_CTL    (ADXL355_ADDR(0x2D) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x01))
#define ADXL355_SELF_TEST    (ADXL355_ADDR(0x2E) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))
#define ADXL355_RESET        (ADXL355_ADDR(0x2F) | SET_ADXL355_TRANSF_LEN(1) | SET_ADXL355_RESET_VAL(0x00))

#define ADXL355_SHADOW_REGISTER_BASE_ADDR (ADXL355_ADDR(0x50) | SET_ADXL355_TRANSF_LEN(5))
#define ADXL355_MAX_FIFO_SAMPLES_VAL  0x60
#define ADXL355_SELF_TEST_TRIGGER_VAL 0x03
#define ADXL355_RESET_CODE            0x52

#endif /* ADXL359_H_ */


int adxl355_read_device_data(mxc_spi_regs_t *spi, uint8_t base_address,uint16_t size, uint8_t *read_data);


int adxl355_write_device_data(mxc_spi_regs_t *spi, uint8_t base_address,uint16_t size, uint8_t *write_data);

int adxl355_get_raw_xyz(mxc_spi_regs_t *spi, uint16_t *raw_x,uint16_t *raw_y, uint16_t *raw_z);
int adxl355_soft_reset(mxc_spi_regs_t *spi);
int adxl355_set_op_mode(mxc_spi_regs_t *spi, uint8_t mode);
int create_data_array(mxc_spi_regs_t *spi, uint32_t samplerate_us,uint16_t size,uint16_t *array);
print_registers(mxc_spi_regs_t *spi);
int adxl355_set_filter(mxc_spi_regs_t *spi, uint8_t filter_op);
int adxl355_set_self_test(mxc_spi_regs_t *spi, uint8_t self_test);
