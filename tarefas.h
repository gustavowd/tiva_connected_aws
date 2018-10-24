/*
 * tarefas.h
 *
 *  Created on: 26/11/2014
 *      Author: Gustavo
 */

#ifndef TAREFAS_H_
#define TAREFAS_H_

extern xTaskHandle procId1;
extern xTaskHandle procId2;
extern xTaskHandle procId3;
extern xTaskHandle procId4;
extern xTaskHandle procId5;
extern xTaskHandle procId6;
extern xTaskHandle procId7;
extern xTaskHandle procId8;
extern xTaskHandle procId9;


void exec(void *param);
void exec2(void *param);
void exec3(void *param);
void Terminal(void *param);
void Keyboard_Handler(void *param);
void Keyb_Task(void *param);
void System_Time(void *param);
void UpLwIP(void *param);
void SD_Task(void *param);

typedef struct _leds_t_{
    unsigned long clock_port;
    unsigned long pin;
    unsigned long port_base;
}leds_t;

#endif /* TAREFAS_H_ */
