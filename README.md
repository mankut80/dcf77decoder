# dcf77decoder
## Easy implementation for decoding the DCF77 Signal. Date and Time are represented as structs. 
the class is able to decode the signal of a connected DCF77-Receiver-Module. It also provides a softclock. Date and time is available via 'getDate' and 'getTime' which are structs (see below).

### usage example with DCF-Antenna connected to PD4 of an AVR µC and a variable 'tick10ms' which is set true every 10ms (you have to do this yourself in a TIMER-ISR or similar)
----------------------------------------------------------------
```cpp

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
```   
  ------------------------------------------------------------------

    
    changelog / history
    v1.00, 28.07.2022
    v1.01, 02.12.2022 -  fixed error in softclock for date.dayofweek
    v1.10  19.05.2023 -  Added option that the time is only valid if a certain number of successfully decoded
                         packets has been received (see #define MIN_AMOUNT_POSITIVE_RESULTS)
                      -  Starting-date after power-on is now 01/01/71
                      -  bugfixes in the pulse-detection-section
                      -  deleted unnecessary variables from headerfile
