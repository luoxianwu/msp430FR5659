/*
 * tmtc.c
 *
 *  Created on: Dec 4, 2024
 *      Author: x-luo
 */

#include "gpio.h"

int tmtc_exe_cmd( uint8_t function, uint16_t address ){
    if( function == 0 && address == 0 ) {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

    }else
    if( function == 0 && address == 1 ) {
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
    }else
    if( function == 1 && address == 0 ) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN6);

    }else
    if( function == 1 && address == 1 ) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
        GPIO_setOutputHighOnPin(GPIO_PORT_P4, GPIO_PIN6);
    }
    return 0;
}

int tmtc_get_data( uint8_t function, uint16_t address, uint8_t * buffer, uint16_t sizeof_buf ){
    return 1;
}

int tmtc_pack_data( uint8_t buf[], uint8_t size){
    return 0;
}


