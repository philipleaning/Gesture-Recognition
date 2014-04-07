/*Parsing code for STM32*/
/* This searches an input string (use on those from the VCP) for commands 
stored in commands[] as pointers to strings, if found the following string is 
converted to an int and stored. Use this to send settings to the DISCOVERY 
from th PC*/


#include "parsing.h"

void parser( char* input_str, Host_comm* host, Host_comm1* host1 )
{
 /*some parameters must not be reinitialised by this function*/
int length = host->length;
int j = 0;
int scope = host->scope; 
int error = 0; 
int diode = host->diode; 
int pulse = host->pulse; 
int dutycycle = host->duty_cycle; 
int Temp = host->temperature;
int freq = host->freq;
int current = host->current;
int voltage = host->voltage;
int id_request = 0;
int sampling_f = host->sampling_freq;
int wave_req_ch1 = 0;
int wave_req_ch2 = 0;
int wave_req_ch3 = 0;
int aver_req_ch1 = 0;
int aver_req_ch2 = 0;
int aver_req_ch3 = 0;
int aver_req_all = 0;
int ampli_setting = host->ampli_setting;
int offset = host->offset;
int gain1 = host->gain1;
int gain2 = host->gain2;
int meas_request = 0;
int repeat = 0;
int aver_delay = host->aver_delay;       /*number of samples from triggering to be skipped in the average calculation*/
int aver_nsampl = host->aver_nsampl;      /*number of samples for the average*/
int reset = 0;
char* occ;
const char* commands[27] = {"OUTFREQ", "DUTYCYCLE", "SMPLFREQ", "PULSE_ON", "PULSE_OFF", 
"WAVE_CH1", "WAVE_CH2", "WAVE_CH3", "ID?", "TLOOP_ON", "TLOOP_OFF", "TEMPSET", "CRRSET", 
"AMPLSET_ON", "AMPLSET_OFF", "OFFSET", "GAIN1_", "GAIN2_", "REPEAT", "AVER_CH1", 
"AVER_CH2", "AVER_CH3", "AVER_D", "AVER_N", "VLTSET", "AVER_ALL", "RESET"};


j = 0;
//while ( j < 13 && input_length > 0 )
while ( j < 27 )
{
  occ = strstr (input_str, commands[j]);
  if (occ != NULL) 
  {
    switch (j)
    {
    case 0:
      /*Frequency setting*/
      freq = atoi(occ+strlen(commands[j]));
      if (freq > 1000) freq = 1000;
      break;
        
    case 1: 
      /*Duty cycle setting*/
      dutycycle = atoi(occ+strlen(commands[j]));
      if (dutycycle > 100) dutycycle = 100;
      break;
        
    case 2:  
      /*Sampling frequency setting*/
      sampling_f = atoi(occ+strlen(commands[j]));
      if (sampling_f > 500000) sampling_f = 500000;
      break;
      
    case 3:
      /*Pulse mode ON*/
      pulse = 1;
      break;
      
    case 4:
      /*Pulse mode OFF*/
      pulse = 0;
      break;
      
    case 5:
      /*Request for measurement from adc2ch1*/
      wave_req_ch1 = 1;
      meas_request = 1;
      break;
     
    case 6:
      /*Request for measurement from adc3ch2*/
      wave_req_ch2 = 1;
      meas_request = 1;
      break;
      
    case 7:
      /*Request for measurement from adc1ch3*/
      wave_req_ch3 = 1;
      meas_request = 1;
      break;
      
    case 8:
      /*ID string request*/
      id_request = 1;
      break;
    
    case 9:
      /*Temperature setting with feedback loop ON*/
      diode = 1;
      break;  
    
    case 10:
      /*Temperature setting with feedback loop OFF*/
      diode = 0;
      break;
    
    case 11:
      /*Temperature setting (with feedback loop)*/
      Temp = atoi(occ+strlen(commands[j]));
      if (Temp > 700) Temp = 700;
      break; 
      
    case 12:
      /*Current setting  [current is in mA]*/
      current = atoi(occ+strlen(commands[j]));
      if (current > 1000) current = 1000;
      break;  
      
    case 13:
      /*The system is configured for the amplificator's offset measurement*/
      ampli_setting=1;
      break;
      
    case 14:
      /*The system configuration is restored*/
      ampli_setting=0;
      break;
    
    case 15:
      /*LMP91050 offset setting*/
      offset = atoi(occ+strlen(commands[j]));
      if (offset > 255) offset = 255;
      break;
      
    case 16:
      /*LMP91050 gain1 setting*/
      //gain1 = 1;
      gain1 = atoi(occ+strlen(commands[j]));
      if (gain1 > 1) gain1 = 1;
      break;
      
    case 17:
      /*LMP91050 gain2 setting*/
      //gain2 = 3;
      gain2 = atoi(occ+strlen(commands[j]));
      if (gain2 > 3) gain2 = 3;
      break;
      
    case 18:
      repeat = 1;
      break;
      
    case 19:
      /*Request for average measurement from adc2ch1*/
      aver_req_ch1 = 1;
      meas_request = 1;
      break;
     
    case 20:
      /*Request for average measurement from adc3ch2*/
      aver_req_ch2 = 1;
      meas_request = 1;
      break;
      
    case 21:
      /*Request for average measurement from adc1ch3*/
      aver_req_ch3 = 1;
      meas_request = 1;
      break;
      
    case 22:
      /*average delay setting*/
      aver_delay = atoi(occ+strlen(commands[j]));
      if (aver_delay > 1000) aver_delay = 1000;
      break;
     
    case 23:
      /*average #samples setting*/
      aver_nsampl = atoi(occ+strlen(commands[j]));
      if (aver_nsampl > 1000) aver_nsampl = 1000;
      break; 
    
    case 24:
      /*Voltage setting  [voltage is in mV]*/
      voltage = atoi(occ+strlen(commands[j]));
      if (voltage > 3000) voltage = 3000; 
      break; 
    
    case 25:
      /*Request for average measurement from adc1ch3*/
      aver_req_all = 1;
      meas_request = 1;
      break;
      
    case 26:
      /*Reset of the device paramters to a known state*/
      reset = 1;
      break;
      
    default:
      break;
    }
    /*When a command string is recognised we have input_lenght-- left to decode*/
  }
  j++;
}


host->scope = scope;
host->diode = diode;
host->pulse = pulse;
host->duty_cycle = dutycycle;
host->temperature = Temp;
host->current = current;
host->voltage = voltage;
host->freq = freq;
host->id_request = id_request;
host->error = error;
host->sampling_freq = sampling_f;
host->meas_request = meas_request;
host->wave_req_adc3ch2=wave_req_ch2;
host->wave_req_adc2ch1=wave_req_ch1;
host->wave_req_adc1ch3=wave_req_ch3;
host->ampli_setting = ampli_setting;
host->offset = offset;
host->gain1 = gain1;
host->gain2 = gain2;
host->repeat = repeat;
host->aver_req_adc3ch2=aver_req_ch2;
host->aver_req_adc2ch1=aver_req_ch1;
host->aver_req_adc1ch3=aver_req_ch3;
host->aver_req_all=aver_req_all;
host->aver_delay = aver_delay;      
host->aver_nsampl = aver_nsampl;
host->reset = reset;
}