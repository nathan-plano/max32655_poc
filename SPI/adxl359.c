/*
 * adxl359.c
 *
 *  Created on: Feb 20, 2023
 *      Author: NPlano2
 */


#include <stdlib.h>
#include <errno.h>
#include "adxl359.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "mxc_delay.h"
#include "gpio.h"

#define MXC_GPIO_PORT_IN MXC_GPIO1
#define MXC_GPIO_PIN_IN MXC_GPIO_PIN_8
mxc_gpio_cfg_t drdy;

void adxl355_init_drdy(){
	drdy.port = MXC_GPIO_PORT_IN;
	drdy.mask = MXC_GPIO_PIN_IN;
	drdy.pad = MXC_GPIO_PAD_NONE;
	drdy.func = MXC_GPIO_FUNC_IN;
	drdy.vssel = MXC_GPIO_VSSEL_VDDIO;
	MXC_GPIO_Config(&drdy);

}

uint16_t adxl355_drdy_get(){

	return (uint16_t)(MXC_GPIO_InGet(drdy.port, drdy.mask)>>8);

}

int adxl355_read_device_data(mxc_spi_regs_t *spi, uint8_t base_address,uint16_t size, uint8_t *read_data){
	mxc_spi_req_t req;
	int ret;
	uint8_t tx_data[size+1];
	for (int i=0 ; i <(size+1);i++){
		if (i==0){
			tx_data[i]=ADXL355_SPI_READ | (ADXL355_ADDR(base_address) << 1);
		}
		else{
			tx_data[i]=0;
		}
	}
	req.spi = spi;
	req.txData = tx_data;
	req.rxData = read_data-1;
	req.txLen = size+1;
	req.rxLen = size+1;
	req.ssIdx = 0;
	req.ssDeassert = 1;
	req.txCnt = 0;
	req.rxCnt = 0;
	ret =MXC_SPI_MasterTransaction(&req);
	return ret;
}
int adxl355_write_device_data(mxc_spi_regs_t *spi, uint8_t base_address,uint16_t size, uint8_t *write_data){
	mxc_spi_req_t req;
	int ret;
	uint8_t tx_data[size+1];
	for (int i=0 ; i <(size+1);i++){
		if (i==0){
			tx_data[i]=ADXL355_SPI_WRITE | (ADXL355_ADDR(base_address) << 1);
		}
		else{
			tx_data[i]=write_data[i-1];
		}
	}
	uint8_t rx_data[size+1];
	req.spi = spi;
	req.txData = tx_data;
	req.rxData = rx_data;
	req.txLen = size+1;
	req.rxLen = size+1;
	req.ssIdx = 0;
	req.ssDeassert = 1;
	req.txCnt = 0;
	req.rxCnt = 0;
	ret =MXC_SPI_MasterTransaction(&req);
	return ret;
}


int adxl355_get_raw_xyz(mxc_spi_regs_t *spi, uint16_t *raw_x,uint16_t *raw_y, uint16_t *raw_z){
	uint8_t array_raw_x[2] = {0};
	uint8_t array_raw_y[2] = {0};
	uint8_t array_raw_z[2] = {0};
	int ret;

	ret = adxl355_read_device_data(spi,ADXL355_ADDR(ADXL355_XDATA),2,array_raw_x);
	*raw_x =((uint16_t)array_raw_x[0]<<8)|((uint16_t)array_raw_x[1]);

	ret = adxl355_read_device_data(spi,ADXL355_ADDR(ADXL355_YDATA),2,array_raw_y);
	*raw_y =((uint16_t)array_raw_y[0]<<8)|((uint16_t)array_raw_y[1]);

	ret = adxl355_read_device_data(spi,ADXL355_ADDR(ADXL355_ZDATA),2,array_raw_z);
	*raw_z =((uint16_t)array_raw_z[0]<<8)|((uint16_t)array_raw_z[1]);



	return ret;
}



int adxl355_get_status(mxc_spi_regs_t *spi, uint8_t *status){
	int ret;
	uint8_t status_temp = {0};
	ret = adxl355_read_device_data(spi,ADXL355_ADDR(ADXL355_STATUS),1,&status_temp);
	*status =status_temp & 0x01;

	return ret;
}


int adxl355_soft_reset(mxc_spi_regs_t *spi){
	uint8_t tx_data[2];
	tx_data[0]=ADXL355_RESET_CODE;
	tx_data[1]=0;
	int ret =adxl355_write_device_data(spi, ADXL355_ADDR(ADXL355_RESET),1,tx_data);
	MXC_Delay(10000);
	return ret;
}



int adxl355_set_op_mode(mxc_spi_regs_t *spi, uint8_t mode){
	int ret;
	uint8_t tx_data[2];
	tx_data[0]=mode;
	tx_data[1]=0;
	ret = adxl355_write_device_data(spi, ADXL355_ADDR(ADXL355_POWER_CTL),1,tx_data);
	return ret;
}

int adxl355_set_filter(mxc_spi_regs_t *spi, uint8_t filter_op){
	int ret;
	uint8_t tx_data[2];
	tx_data[0]=filter_op;
	tx_data[1]=0;
	ret = adxl355_write_device_data(spi, ADXL355_ADDR(ADXL355_FILTER),1,tx_data);
	return ret;
}


int adxl355_set_self_test(mxc_spi_regs_t *spi, uint8_t self_test){
	int ret;
	uint8_t tx_data[2];
	tx_data[0]=self_test;
	tx_data[1]=0;
	ret = adxl355_write_device_data(spi, ADXL355_ADDR(ADXL355_SELF_TEST),1,tx_data);
	return ret;
}



int create_data_array(mxc_spi_regs_t *spi, uint32_t samplerate_us,uint16_t size,uint16_t *array){
	int iterate =((int)(size/3));
	int ret;
	uint16_t raw_x;
	uint16_t raw_y;
	uint16_t raw_z;
	uint8_t status;
	uint16_t drdy_status;
	int i =0;
	int sample=0;
	while( i <iterate )
	{
		drdy_status =adxl355_drdy_get();

		if((drdy_status==1) &(sample==0)){
			adxl355_get_raw_xyz(spi,  &raw_x,&raw_y,&raw_z);
			array[i]=raw_x;
			array[i+iterate]=raw_y;
			array[i+iterate*2]=raw_z;
			i++;
			sample=1;
		}

		if((drdy_status==0) &(sample==1)){
			sample=0;

		}

	}
	return 1;

}


print_registers(mxc_spi_regs_t *spi){
		uint8_t fifo_data[1];
		uint8_t OFFSET_X[2]= {0};
		uint8_t OFFSET_Y[2]= {0};
		uint8_t OFFSET_Z[2]= {0};
		uint8_t ACT_EN [1];
		uint8_t ACT_THRESH[2];
		uint8_t ACT_CNT[1];
		uint8_t FILTER[1];
		uint8_t FIFO_SAMPLES[1];
		uint8_t INT_MAP[1];
		uint8_t SYNC[1];
		uint8_t RANGE[1];
		uint8_t POWER_CTL[1];
		uint8_t SELF_TEST[1];
		uint8_t RESET[1];
		adxl355_read_device_data(spi,ADXL355_ADDR(ADXL355_FIFO_DATA),1, fifo_data);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_OFFSET_X),2, OFFSET_X);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_OFFSET_Y),2,OFFSET_Y);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_OFFSET_Z),2, OFFSET_Z);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_ACT_EN),1, ACT_EN);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_ACT_THRESH),2,ACT_THRESH);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_ACT_CNT),1, ACT_CNT);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_FILTER),1, FILTER);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_FIFO_SAMPLES),1, FIFO_SAMPLES);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_INT_MAP),1,INT_MAP);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_SYNC),1, SYNC);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_RANGE),1,RANGE);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_POWER_CTL),1, POWER_CTL);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_SELF_TEST),1,SELF_TEST);
		adxl355_read_device_data(spi, ADXL355_ADDR(ADXL355_RESET),1, RESET);


		printf("fifo_data  %d\n" ,fifo_data[0]);
		printf("OFFSET_X  %x, %x\n" ,OFFSET_X[0],OFFSET_X[1] );
		printf("OFFSET_Y  %x, %x\n" ,OFFSET_Y[0],OFFSET_Y[1] );
		printf("OFFSET_Z  %x, %x\n" ,OFFSET_Z[0],OFFSET_Z[1] );
		printf("ACT_EN  %x\n" ,ACT_EN[0]);
		printf("ACT_THRESH  %x, %x\n" ,ACT_THRESH[0],ACT_THRESH[1] );
		printf("ACT_CNT  %x\n" ,ACT_CNT[0]);
		printf("FILTER  %x\n" ,FILTER[0]);
		printf("FIFO_SAMPLES  %x\n" ,FIFO_SAMPLES[0]);
		printf("INT_MAP  %x\n" ,INT_MAP[0]);
		printf("SYNC  %x\n" ,SYNC[0]);
		printf("RANGE  %x\n" ,RANGE[0]);
		printf("POWER_CTL  %x\n" ,POWER_CTL[0]);
		printf("SELF_TEST  %x\n" ,SELF_TEST[0]);
		printf("RESET  %x\n" ,RESET[0]);

}

















