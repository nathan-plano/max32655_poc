/*
 * global_set.get.h
 *
 *  Created on: Feb 13, 2023
 *      Author: NPlano2
 */
#include <stdio.h>
#include <string.h>

#ifndef GLOBAL_SET_GET_H_
#define GLOBAL_SET_GET_H_



void set_send_flag(int value);
int get_send_flag();

void set_rx_data(uint8_t *data);
uint8_t* get_rx_data();

void set_data_len(int value);
int get_data_len();

#endif /* GLOBAL_SET_GET_H_ */
