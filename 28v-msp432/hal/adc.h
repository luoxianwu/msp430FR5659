#ifndef __ADC_H__
#define __ADC_H__

#define ADC_TOTAL_CH  8

void init_ADC(void);

void init_ADC2(void);

void adc_start(void);

bool adc_read_all( uint16_t array[], uint8_t cnt );

uint16_t adc_to_voltage( uint16_t adc_value );

uint16_t adc_board_temperature( uint16_t adc_value );


#endif //__ADC_H__
