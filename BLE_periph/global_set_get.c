/*
 * global_set_get.c
 *
 *  Created on: Feb 13, 2023
 *      Author: NPlano2
 */

#include "global_set_get.h"


static int data_len=0;
static int send_flag=0;
static uint8_t* rx_data;
void set_send_flag(int value){

	send_flag=value;
}
int get_send_flag(){
	return send_flag;
}



void set_data_len(int value){

	data_len=value;
}
int get_data_len(){
	return data_len;
}



void set_rx_data(uint8_t *data){
	rx_data =data;
}
uint8_t* get_rx_data(){
	return rx_data;
}
