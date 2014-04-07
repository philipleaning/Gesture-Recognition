#ifndef __PARSING_H
#define __PARSING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



/*struct to store host commands to device (mettiamolo in parsing.h)*/
typedef struct
{
  int length;       /*length (number of bytes) of the incoming host sequence*/
  int scope;        /*scope mode variable*/
  int diode;        /*temperature reading from diode enabling*/
  int pulse;
  int duty_cycle;
  int temperature;
  int current;
  int voltage;
  int id_request;
  int error;
  int freq;
  int sampling_freq;
  int meas_request;     /*SET if there is any measurement request*/
  int wave_req_adc3ch2; /*request of the 1000 samples from adc3ch2*/
  int aver_req_adc3ch2; /*request of the average mesaurement from adc3ch2*/
  int wave_req_adc2ch1; /*request of the 1000 samples from adc2ch1*/
  int aver_req_adc2ch1; /*request of the average mesaurement from adc2ch1*/
  int wave_req_adc1ch3; /*request of the 1000 samples from adc1ch3*/
  int aver_req_adc1ch3; /*request of the average mesaurement from adc1ch3*/
  int aver_req_all;     /*request of the average mesaurement from all channels*/
  int ampli_setting;    /*setting of the LMP91050 chip parameters via SPI*/
  int offset;           /*offset value for LMP91050 chip*/
  int gain1;            /*gain value for LMP amplificator (0~1)*/
  int gain2;            /*gain value for LMP amplificator (0~3)*/
  int repeat;           /*the host software asks for a repetition of the last data*/
  int aver_delay;       /*number of samples from triggering to be skipped in the average calculation*/
  int aver_nsampl;      /*number of samples for the average*/
  int reset;            /*reset variable*/
  
}Host_comm;

typedef struct
{
  int wave_req_adc3ch2; /*request of the 1000 samples from adc3ch2*/
  int aver_req_adc3ch2; /*request of the average mesaurement from adc3ch2*/
  int wave_req_adc2ch1; /*request of the 1000 samples from adc2ch1*/
  int aver_req_adc2ch1; /*request of the average mesaurement from adc2ch1*/
}Host_comm1;

typedef struct
{
  int scope;
  int diode;
  int pulse;
  int duty_cycle;
  int temperature;
  int current;
  int voltage;
  int freq;
  int sampl_freq;
  int ampli_setting;
  int lmp_change;       /*change in LMP parameters*/
}set_struct;

void parser( char* input_str, Host_comm* host, Host_comm1* host1 );

#endif /* __PARSING_H */
