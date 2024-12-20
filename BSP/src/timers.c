#include <s3c44b0x.h>
#include <s3cev40.h>
#include <timers.h>

extern void isr_TIMER0_dummy( void );

static uint32 loop_ms = 0;
static uint32 loop_s = 0;

static void sw_delay_init( void );

void timers_init( void )
{
    TCFG0 = 0x0;
    TCFG1 = 0x0;

    TCNTB0 = 0x0;
    TCMPB0 = 0x0;
    TCNTB1 = 0x0;
    TCMPB1 = 0x0;
    TCNTB2 = 0x0;
    TCMPB2 = 0x0;
    TCNTB3 = 0x0;
    TCMPB3 = 0x0;
    TCNTB4 = 0x0;
    TCMPB4 = 0x0;
    TCNTB5 = 0x0;

    TCON = (1 << 1)  |	// timer 0
    	   (1 << 9)  |	// timer 1
    	   (1 << 13) |	// timer 2
    	   (1 << 17) |	// timer 3
    	   (1 << 21) |	// timer 4
    	   (1 << 25); 	// timer 5
    TCON = 0x0;

    sw_delay_init();
}

static void sw_delay_init( void )
{
    uint32 i;
    
    timer3_start();
    for( i=1000000; i; i--);
    loop_s = ((uint64)1000000*10000)/timer3_stop();
    loop_ms = loop_s / 1000;
};

void wait_for_1ms(void) {
	TCFG0  = (TCFG0 & ~(0xFF << 8)) | (0 << 8);  // Prescaler = 0
	TCFG1  = (TCFG1 & ~(0xF << 12)) | (0 << 12); // T3 divisor = 2
	TCNTB3 = 32000;								 // T3 count = 32000
	TCON   = (TCON  & ~(0xF << 16)) | (1 << 17); // one shot, load TCNT3, stop T3
	TCON   = (TCON  & ~(0xF << 16)) | (1 << 16); // one shot, unload TCNT3, start T3
	while(!TCNTO3);
	while(TCNTO3);
}

void wait_for_1s(void) {
	TCFG0  = (TCFG0 & ~(0xFF << 8)) | (63 << 8); // Prescaler = 63
	TCFG1  = (TCFG1 & ~(0xF << 12)) | (4 << 12); // T3 divisor = 32
	TCNTB3 = 31250;								 // T3 count = 31250
	TCON   = (TCON  & ~(0xF << 16)) | (1 << 17); // one shot, load TCNT3, stop T3
	TCON   = (TCON  & ~(0xF << 16)) | (1 << 16); // one shot, unload TCNT3, start T3
	while(!TCNTO3);
	while(TCNTO3);
}

void timer3_delay_ms( uint16 n ) {
    for( ; n; n-- ) {
    	wait_for_1ms();
    }
}

void sw_delay_ms( uint16 n )
{
    uint32 i;
    
    for( i=loop_ms*n; i; i-- );
}

void timer3_delay_s( uint16 n ) {
    for( ; n; n--){
    	wait_for_1s();
    }
}

void sw_delay_s( uint16 n )
{
    uint32 i;
    
    for( i=loop_s*n; i; i-- );
}

void timer3_start( void ) 
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (199 << 8);    
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (4 << 12);
    
    TCNTB3 = 0xffff; 
    TCON = (TCON & ~(0xf << 16)) | (1 << 17);
    TCON = (TCON & ~(0xf << 16)) | (1 << 16);
    while( !TCNTO3 );
}

uint16 timer3_stop( void )
{
    TCON &= ~(1 << 16);
    return 0xffff - TCNTO3;
}

void timer3_start_timeout( uint16 n ) 
{
    TCFG0 = (TCFG0 & ~(0xff << 8)) | (199 << 8);          
    TCFG1 = (TCFG1 & ~(0xf << 12)) | (4 << 12);
    
    TCNTB3 = n; 
    TCON = (TCON & ~(0xf << 16)) | (1 << 17);
    TCON = (TCON & ~(0xf << 16)) | (1 << 16);
    while( !TCNTO3 );
}

uint16 timer3_timeout( )
{
    return !TCNTO3;
}    

void timer0_open_tick( void (*isr)(void), uint16 tps )
{
    pISR_TIMER0 = isr;
    I_ISPC      = BIT_TIMER0;
    INTMSK     &= ~(BIT_GLOBAL | BIT_TIMER0);

    /*
	** We want a 25us, 2.5us, 0.25us and 31.25ns timer setup so we use the formula t = (N + 1) * D / MCLK
	** Since N in [0, 1, ..., 255] then we solve for N t(seconds) = ((N + 1) * D)/(64 * 10^6)
	** This is a bit of trial and error, since we want N to be in range. The divisor may vary.
	*/

    if( tps > 0 && tps <= 10 ) {
    	/*
    	** Solving the formula we get N = 199 and D = 8
    	*/
    	TCFG0  = (TCFG0 & ~(0xff << 0)) | (199 << 0); // Set prescaler to 199
		TCFG1  = (TCFG1 & ~(0xf << 0)) | (2 << 0);	  // Set divisor to 8
        TCNTB0 = (40000U / tps);
    } else if( tps > 10 && tps <= 100 ) {
    	/*
		** Solving the formula we get N = 79 and D = 2
		*/
    	TCFG0  = (TCFG0 & ~(0xff << 0)) | (79 << 0); // Set prescaler to 79
		TCFG1  = (TCFG1 & ~(0xf << 0)) | (0 << 0);	 // Set divisor to 2
        TCNTB0 = (400000U / (uint32) tps);
    } else if( tps > 100 && tps <= 1000 ) {
    	/*
		** Solving the formula we get N = 7 and D = 2
		*/
    	TCFG0  = (TCFG0 & ~(0xff << 0)) | (7 << 0); // Set prescaler to 7
		TCFG1  = (TCFG1 & ~(0xf << 0)) | (0 << 0);	// Set divisor to 2
        TCNTB0 = (4000000U / (uint32) tps);
    } else if ( tps > 1000 ) {
    	/*
		** Solving the formula we get N = 0 and D = 2
		*/
    	TCFG0  = (TCFG0 & ~(0xff << 0)) | (0 << 0); // Set prescaler to 0
		TCFG1  = (TCFG1 & ~(0xf << 0)) | (0 << 0);	// Set divisor to 2
        TCNTB0 = (32000000U / (uint32) tps);
    }

    TCON = (TCON  & ~(0xF << 0)) | (1 << 3) | (1 << 1); // one shot, load TCNT0, stop T0
	TCON = (TCON  & ~(0xF << 0)) | (1 << 3) | (1 << 0); // one shot, unload TCNT0, start T0
}

void timer0_open_ms( void (*isr)(void), uint16 ms, uint8 mode )
{
    pISR_TIMER0 = isr;
    I_ISPC      = BIT_TIMER0;
    INTMSK     &= ~(BIT_GLOBAL | BIT_TIMER0);

    /*
    ** We want a 100us timer setup so we use the formula t = (N + 1) * D / MCLK
    ** Since N in [0, 1, ..., 255] then we solve for N 10^(-4) = ((N + 1) * 32)/(64 * 10^6)
    ** Hence N is 199 and we need Divisor to be 32 in order to have N in range
    ** This is a bit of trial and error
	*/

    TCFG0  = (TCFG0 & ~(0xff << 0)) | (199 << 0); // Set prescaler to 199
    TCFG1  = (TCFG1 & ~(0xf << 0)) | (4 << 0);	  // Set divisor to 32
    TCNTB0 = 10*ms;

    TCON   = (TCON  & ~(0xF << 0)) | (mode << 3) | (1 << 1); // load TCNT0, stop T0
	TCON   = (TCON  & ~(0xF << 0)) | (mode << 3) | (1 << 0); // unload TCNT0, start T0
}

void timer0_close( void )
{
    TCNTB0 = 0x0;
    TCMPB0 = 0x0;

    TCON   = (TCON  & ~(0xF << 0)) | (1 << 3) | (1 << 1); // one shot, load TCNT0, stop T0
   	TCON   = (TCON  & ~(0xF << 0)) | (1 << 3) | (1 << 0); // one shot, unload TCNT0, start T0
    
    INTMSK     |= BIT_GLOBAL | BIT_TIMER0;
    pISR_TIMER0 = isr_TIMER0_dummy;
}
