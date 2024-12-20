#include <s3c44b0x.h>
#include <uart.h>

void uart0_init(void)
{
    UFCON0 = 0x1;
    UMCON0 = 0x0;
    ULCON0 = 0x3;
    UBRDIV0 = 0x22;
    UCON0 = 0x5;
}

void uart0_putchar(char ch)
{
    while(UFSTAT0 & (1 << 9));
    UTXH0 = ch;
}        

char uart0_getchar(void)
{
    while(!(UFSTAT0 & 0xf));
    return URXH0;
}

void uart0_puts(char *s) {
    while(*s) {
    	uart0_putchar(*s);
    	++s;
    }
}

void uart0_putint(int32 i) {
    char buff[11 + 1];
    char* _Idx = buff + 11;
    boolean _Neg = 0;

    *_Idx = '\0';

    if(i == 0) {
    	*--_Idx = '0';
    	uart0_puts(_Idx);
    	return;
    }

    if(i < 0) {
    	_Neg = 1;
    	i = -i;
    }

    while(i) {
    	*--_Idx = (i % 10) + '0';
    	i /= 10;
    }

    if(_Neg) {
    	*--_Idx = '-';
    }

    uart0_puts(_Idx);

}

void uart0_puthex(uint32 i)
{
    char buf[8 + 1];
    char *p = buf + 8;
    uint8 c;

    *p = '\0';

    do {
        c = i & 0xf;
        if( c < 10 )
            *--p = '0' + c;
        else
            *--p = 'a' + c - 10;
        i = i >> 4;
    } while( i );

    uart0_puts( p );
}

void uart0_gets(char *s) {
    char _Ch;

    do {
    	_Ch = uart0_getchar();
    	*s++ = _Ch;
    } while(_Ch != '\n' && _Ch != '\r');

    *(s - 1) = '\0';
}

int32 uart0_getint(void) {
	char _Buff[11 + 1];
	uart0_gets(_Buff);

	int32 result = 0;
	boolean _Neg = 0;
	uint32 _Idx	 = 0;

	if(_Buff[0] == '-') {
		_Neg = 1;
		++_Idx;
	}

	while(_Buff[_Idx] >= '0' && _Buff[_Idx] <= '9') {
		result = result * 10 + (_Buff[_Idx] - '0');
		++_Idx;
	}

	if(_Neg) {
		result = -result;
	}

	return result;
}

uint32 uart0_gethex(void) {
    char _Buff[8 + 1];
    uart0_gets(_Buff);

    uint32 result = 0;
    char* _Idx 	  = _Buff;

    while(*_Idx != '\0') {
    	char _Ch = *_Idx;
    	if	   (_Ch >= '0' && _Ch <= '9')
    		result = result * 16 + (_Ch - '0');
    	else if(_Ch >= 'a' && _Ch <= 'f')
    		result = result * 16 + (_Ch - 'a' + 10);
    	else if(_Ch >= 'A' && _Ch <= 'F')
    		result = result * 16 + (_Ch - 'A' + 10);
    	else break;

    	++_Idx;
    }

    return result;
}
