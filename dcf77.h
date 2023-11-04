/*  
    DCF77 - Decoder-Class
    M. Schmid 2022
    
    the class is able to decode the signal of a connected
    DCF77-Receiver-Module. It also provides a softclock.
    Date and time is available via 'getDate' and 'getTime' which 
    are structs (see below).
    
    usage example with DCF-Antenna connected to PD4 and a
    variable 'tick10ms' which is set true every 10ms (you have
    to do this yourself in a TIMER-ISR)
    ----------------------------------------------------------------

    #include "dcf77.h"
    
    uint8_t tick10ms = 0;
    
    main()
    {
        
        dcf77decoder dcf77;
        structtime time;
        structdate date;
        
        while(1)
        {
            if(tick10ms) 
            {
                dcf77.cycledupdate_every_10ms(PIND & (1<<PIND4))
                tick10ms = 0;
            }

            time = dcf77.getTime();
            date = dcf77.getDate();

            uint8_t hour = time.hour;
            uint8_t minu = time.minute;
        }
    }
    ------------------------------------------------------------------
*/

// changelog / history
// v1.00, 28.07.2022
// v1.01, 02.12.2022 -  fixed error in softclock for date.dayofweek
// v1.10  19.05.2023 -  Added option that the time is only valid if a certain number of successfully decoded
//                      packets has been received (see #define MIN_AMOUNT_POSITIVE_RESULTS)
//                   -  Starting-date after power-on is now 01/01/71
//                   -  bugfixes in the pulse-detection-section
//                   -  deleted unnecessary variables from headerfile


#ifndef DCF77_H_
#define DCF77_H_

#include <avr/io.h>
#include <stdio.h>

// after this time (in times of 10ms) without syncing, the variable 'time_is_dcf_time' will be set false
// 360000 = 1 hour
#define TIMETILLDCFLOST 360000

// minimum amount of positive DCF77-results which will set the softclock (and the state 'time_is_dcf-time' to true)
#define MIN_AMOUNT_POSITIVE_RESULTS 2

// wether the signal of the DCF-Receiver is high-(1) or lowactive(0)
#define DCFSIGNALACTIVELEVEL 1

// allowed timings (in ms*10) for interpreting the DCF-signal as 'true' and 'false'
#define DCFSIGPULSE_0_MIN 7
#define DCFSIGPULSE_0_MAX 12
#define DCFSIGPULSE_1_MIN 17
#define DCFSIGPULSE_1_MAX 22

struct structtime
{
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

struct structdate
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t dayofweek;
};

class dcf77decoder
{
private:
    
    uint8_t pauselength;
    uint8_t signallength;
    uint8_t newpulsedetected;
    uint8_t newpausedetected;
    
    uint8_t bitstream[60];
    uint8_t dcfsignalstatebefore;
    uint32_t calls_without_result;
    uint8_t number_of_positive_results;
    uint8_t millisec10;
    structtime time;
    structdate date;
    structtime timetemp;
    structdate datetemp;

    uint8_t parseData();
    uint8_t paritycheck(uint8_t begin, uint8_t end);


public:
    dcf77decoder();
    ~dcf77decoder();

    // deliver the signalstate of the DCF77-Receiver
    void cycledupdate_every_10ms(uint8_t dcfsignalstate);

    // indicates the correct received time and date
    uint8_t time_is_dcftime;

    structdate getDate();
    structtime getTime();
    void setDate(structdate _date);
    void setTime(structtime _time);

    // position in the bitstream (second)
    uint8_t bitnumber;
    
    // length of the postive pulse in ms*10
    uint8_t pulselength;
};


#endif
