#include <avr/io.h>
#include <stdio.h>
#include "dcf77.h"


dcf77decoder::dcf77decoder()
{
    time.hour = 0;
    time.minute = 0;
    time.second = 0;
    date.year = 71;
    date.month = 1;
    date.day = 1;
    date.dayofweek = 0;
    pulselength = 0;
    pauselength = 0;
    signallength = 0;
    newpulsedetected = 0;
    newpausedetected = 0;
    bitstream[60] = {0};
    bitnumber = 0;
    time_is_dcftime = 0;
    millisec10 = 0;
    calls_without_result = 0;
    number_of_positive_results = 0;
    dcfsignalstatebefore = !DCFSIGNALACTIVELEVEL;
}

dcf77decoder::~dcf77decoder()
{
}

void dcf77decoder::cycledupdate_every_10ms(uint8_t dcfsignalstate)
{
    if(DCFSIGNALACTIVELEVEL == 0) dcfsignalstate = !dcfsignalstate;

    millisec10++;
    
    if(dcfsignalstate == dcfsignalstatebefore)
    {
        if(signallength<255) signallength++;
    }
    else if (dcfsignalstatebefore == 1)
    {
        pulselength = signallength;
        newpulsedetected = 1;
        signallength = 0;
    }
    else if (dcfsignalstatebefore == 0)
    {
        pauselength = signallength;
        newpausedetected = 1;
        signallength = 0;
    }

    if(newpausedetected)
    {
        if((pauselength>=160) && (pauselength<=230))
        {
            if(bitnumber>57)
            {
                if (parseData())
                {
                    if((++number_of_positive_results) >= MIN_AMOUNT_POSITIVE_RESULTS)
                    {
                        number_of_positive_results--;
                        time_is_dcftime = true;
                        time = timetemp;
                        date = datetemp;
                        millisec10 = 0;
                        time.second = 0;
                        calls_without_result = 0;
                    }
                }
                else
                {
                    number_of_positive_results = 0;
                }
            }

            bitnumber = 0;
        }

        newpausedetected = 0;
    }

    if(newpulsedetected)
    {
        if ((pulselength>=DCFSIGPULSE_0_MIN) && (pulselength<=DCFSIGPULSE_0_MAX))
        {
            bitstream[bitnumber] = 0;
            if(bitnumber<58) bitnumber++;
        }
        else if ((pulselength>=DCFSIGPULSE_1_MIN) && (pulselength<=DCFSIGPULSE_1_MAX))
        {
            bitstream[bitnumber] = 1;
            if(bitnumber<58) bitnumber++;
        }
         
        newpulsedetected = 0;
    }

    //Soft-Clock
    if(millisec10 >= 100)
    {
        millisec10 = 0;
        if(++time.second >= 60)
        {
            time.second = 0;
            if(++time.minute >= 60)
            {
                time.minute = 0;
                if(++time.hour >= 24)
                {
                    time.hour = 0;
                    if(++date.dayofweek > 6) date.dayofweek = 0;
                    date.day++;
                }
            }
        
            if( ((date.day==31) && ((date.month%2)==0)) ||
                ((date.day==29) && ((date.month==2))) ||
                (date.day==32) )
            {
                date.day=1;
                if(++date.month > 12)
                {
                    date.month = 1;
                    date.year++;
                }
            }
        }
    }

    //Reset variable after some time without dcf-signal
    if(calls_without_result == TIMETILLDCFLOST) time_is_dcftime = false;
    if(calls_without_result < (TIMETILLDCFLOST+1)) calls_without_result++;
    dcfsignalstatebefore=dcfsignalstate;

}


uint8_t dcf77decoder::parseData()
{
    timetemp.minute =   bitstream[21]+   
                        bitstream[22]*2+
                        bitstream[23]*4+
                        bitstream[24]*8+
                        bitstream[25]*10+
                        bitstream[26]*20+
                        bitstream[27]*40;

    timetemp.hour =     bitstream[29]+
                        bitstream[30]*2+
                        bitstream[31]*4+
                        bitstream[32]*8+
                        bitstream[33]*10+
                        bitstream[34]*20;

    datetemp.day =      bitstream[36]+
                        bitstream[37]*2+
                        bitstream[38]*4+
                        bitstream[39]*8+
                        bitstream[40]*10+
                        bitstream[41]*20;

    datetemp.dayofweek= bitstream[42]+
                        bitstream[43]*2+
                        bitstream[44]*4;

    datetemp.month =    bitstream[45]+
                        bitstream[46]*2+
                        bitstream[47]*4+
                        bitstream[48]*8+
                        bitstream[49]*10;

    datetemp.year =     bitstream[50]+
                        bitstream[51]*2+
                        bitstream[52]*4+
                        bitstream[53]*8+
                        bitstream[54]*10+
                        bitstream[55]*20+
                        bitstream[56]*40+
                        bitstream[57]*80;

    //perform plausibility-checks
    if((bitstream[0] != 0) || (bitstream[20] != 1)) return false;
    if(paritycheck(21, 27) != bitstream[28]) return false;
    if(paritycheck(29, 34) != bitstream[35]) return false;
    if(paritycheck(36, 57) != bitstream[58]) return false;
    if(timetemp.minute > 59) return false;
    if(timetemp.hour > 23) return false;
    if(datetemp.day > 31) return false;
    if(datetemp.month > 12) return false;
    if(datetemp.year > 99) return false;

    //everything okay
    return true;
}

uint8_t dcf77decoder::paritycheck(uint8_t begin, uint8_t end)
{
    uint8_t paritycounter = 0;

    for(uint8_t z=begin; z<=end; z++)
    {
        paritycounter += bitstream[z];
    }
    
    return (paritycounter % 2);
}

structtime dcf77decoder::getTime()
{
    return time;
}

structdate dcf77decoder::getDate()
{
    return date;
}

void dcf77decoder::setTime(structtime _time)
{
    time = _time;
}

void dcf77decoder::setDate(structdate _date)
{
    date = _date;
}
