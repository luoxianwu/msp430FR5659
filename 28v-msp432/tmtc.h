/*
 * tmtc.h
 *
 *  Created on: Dec 4, 2024
 *      Author: x-luo
 */

#ifndef TMTC_H_
#define TMTC_H_

int tmtc_exe_cmd( uint8_t func_code, uint16_t addr_code );



size_t tmtc_get_data( uint8_t func_code, uint16_t addr_code, void * p_data, size_t max_size);

#endif /* TMTC_H_ */
