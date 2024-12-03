#ifndef __ADC_H__
#define __ADC_H__

void init_ADC(void);

void adc_start(void);

bool adc_read_all( uint16_t array[], uint8_t cnt );


#endif //__ADC_H__
