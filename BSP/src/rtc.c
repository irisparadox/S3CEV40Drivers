#include <s3c44b0x.h>
#include <s3cev40.h>
#include <rtc.h>

extern void isr_TICK_dummy( void );

void rtc_init( void )
{
    TICNT   = 0x0;
    RTCALM  = 0x0;
    RTCRST  = 0x0;
        
    RTCCON  = 0x1;
    
    BCDYEAR = 0x13;
    BCDMON  = 0x01;
    BCDDAY  = 0x01;
    BCDDATE = 0x2;
    BCDHOUR = 0x00;
    BCDMIN  = 0x00;
    BCDSEC  = 0x00;

    ALMYEAR = 0x0;
    ALMMON  = 0x0;
    ALMDAY  = 0x0;
    ALMHOUR = 0x0;
    ALMMIN  = 0x0;
    ALMSEC  = 0x0;

    RTCCON &= ~0x1;
}

void rtc_puttime( rtc_time_t *rtc_time )
{
    RTCCON |= 0x1;
    
    BCDYEAR = ((rtc_time->year / 10) << 4) | (rtc_time->year % 10);
    BCDMON  = ((rtc_time->mon / 10) << 4) | (rtc_time->mon % 10);
    BCDDAY  = ((rtc_time->mday / 10) << 4) | (rtc_time->mday % 10);
    BCDDATE = ((rtc_time->wday / 10) << 4) | (rtc_time->wday % 10);
    BCDHOUR = ((rtc_time->hour / 10) << 4) | (rtc_time->hour % 10);
    BCDMIN  = ((rtc_time->min / 10) << 4) | (rtc_time->min % 10);
    BCDSEC  = ((rtc_time->sec / 10) << 4) | (rtc_time->sec % 10);
        
    RTCCON &= ~0x1;
}

void rtc_gettime( rtc_time_t *rtc_time )
{
    RTCCON |= 0x1;
    
    rtc_time->year = ((BCDYEAR >> 4) * 10) + (BCDYEAR & 0xf);
    rtc_time->mon  = ((BCDMON >> 4) * 10) + (BCDMON & 0xf);
    rtc_time->mday = ((BCDDAY >> 4) * 10) + (BCDDAY & 0xf);
    rtc_time->wday = ((BCDDATE >> 4) * 10) + (BCDDATE & 0xf);
    rtc_time->hour = ((BCDHOUR >> 4) * 10) + (BCDHOUR & 0xf);
    rtc_time->min  = ((BCDMIN >> 4) * 10) + (BCDMIN & 0xf);
    rtc_time->sec  = ((BCDSEC >> 4) * 10) + (BCDSEC & 0xf);
    if( ! rtc_time->sec ){
    	rtc_time->year = ((BCDYEAR >> 4) * 10) + (BCDYEAR & 0xf);
		rtc_time->mon  = ((BCDMON >> 4) * 10) + (BCDMON & 0xf);
		rtc_time->mday = ((BCDDAY >> 4) * 10) + (BCDDAY & 0xf);
		rtc_time->wday = ((BCDDATE >> 4) * 10) + (BCDDATE & 0xf);
		rtc_time->hour = ((BCDHOUR >> 4) * 10) + (BCDHOUR & 0xf);
		rtc_time->min  = ((BCDMIN >> 4) * 10) + (BCDMIN & 0xf);
		rtc_time->sec  = ((BCDSEC >> 4) * 10) + (BCDSEC & 0xf);
    };

    RTCCON &= ~0x1;
}


void rtc_open( void (*isr)(void), uint8 tick_count )
{
    pISR_TICK = isr;
    I_ISPC    = BIT_TICK;
    INTMSK   &= ~(BIT_GLOBAL | BIT_TICK);
    TICNT     = (1 << 7) | tick_count;
}

void rtc_close( void )
{
    TICNT     = 0x0;
    INTMSK   |= (BIT_GLOBAL | BIT_TICK);
    pISR_TICK = isr_TICK_dummy;
}
