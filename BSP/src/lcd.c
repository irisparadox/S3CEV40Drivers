#include <s3c44b0x.h>
#include <lcd.h>

extern uint8 font[];
uint8 lcd_buffer[LCD_BUFFER_SIZE];

static uint8 state;

void lcd_init( void )
{      
    DITHMODE = 0x12210;
    DP1_2    = 0xa5a5;
    DP4_7    = 0xba5da65;
    DP3_5    = 0xa5a5f;
    DP2_3    = 0xd6b;
    DP5_7    = 0xeb7b5ed;
    DP3_4    = 0x7dbe;
    DP4_5    = 0x7ebdf;
    DP6_7    = 0x7fdfbfe;
    
    REDLUT   = 0x0;
    GREENLUT = 0x0;
    BLUELUT  = 0x0;

    LCDCON1  = 0x1C020;
    LCDCON2  = 0x13CEF;
    LCDCON3  = 0x0;

    LCDSADDR1 = (2 << 27) | ((uint32)lcd_buffer >> 1);
    LCDSADDR2 = (1 << 29) | (((uint32)lcd_buffer + LCD_BUFFER_SIZE) & 0x3FFFFF) >> 1;
    LCDSADDR3 = 0x50;
    
    lcd_off();
}

void lcd_on( void )
{
    LCDCON1 |= 1;
    state = 1;
}

void lcd_off( void )
{
    LCDCON1 &= ~(1);
    state = 0;
}

uint8 lcd_status( void )
{
    return state;
}

void lcd_clear( void ) {
	uint16 i;
    for(i = 0; i < LCD_BUFFER_SIZE; ++i) {
    	lcd_buffer[i] = WHITE;
    }
}

void lcd_putpixel( uint16 x, uint16 y, uint8 c)
{
    uint8 byte, bit;
    uint16 i;

    i = x/2 + y*(LCD_WIDTH/2);
    bit = (1-x%2)*4;
    
    byte = lcd_buffer[i];
    byte &= ~(0xF << bit);
    byte |= c << bit;
    lcd_buffer[i] = byte;
}

uint8 lcd_getpixel( uint16 x, uint16 y ) {
    uint8 byte;
    uint16 i;

    i = x / 2 + y * (LCD_WIDTH / 2);

    byte = lcd_buffer[i];

    return byte;
}

void lcd_draw_hrow( uint16 xleft, uint16 xright, uint16 y, uint8 color, uint16 width ) {
	uint16 auxY = y;
	while(xleft <= xright){
		y = auxY;
		while(y < (auxY + width)){
			lcd_putpixel(xleft, y++, color);
		}
		++xleft;
	}
}

void lcd_draw_vrow( uint16 yup, uint16 ydown, uint16 x, uint8 color, uint16 width ) {
	uint16 auxX = x;
	while(yup <= ydown){
		x = auxX;
		while(x < (auxX + width)){
			lcd_putpixel(x++, yup,color);
		}
		++yup;
	}
}

void lcd_draw_box( uint16 xleft, uint16 yup, uint16 xright, uint16 ydown, uint8 color, uint16 width ) {
    lcd_draw_hrow(xleft, xright, yup, color, width);
    lcd_draw_hrow(xleft, xright, ydown, color, width);
    lcd_draw_vrow(yup, ydown, xleft, color, width);
    lcd_draw_vrow(yup, ydown, xright, color, width);
}

void lcd_putchar( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 row, col;
    uint8 *bitmap;

    bitmap = font + ch*16;
    for( row=0; row<16; row++ )
        for( col=0; col<8; col++ )                    
            if( bitmap[row] & (0x80 >> col) )
                lcd_putpixel( x+col, y+row, color );
            else
                lcd_putpixel( x+col, y+row, WHITE );
}

void lcd_puts( uint16 x, uint16 y, uint8 color, char *s ) {
    uint16 i = 0;
    while(s[i] != '\0') {
    	lcd_putchar(x, y, color, s[i]);
    	++i;
    	x += 8;
    }
}

void lcd_putint( uint16 x, uint16 y, uint8 color, int32 i )
{
	char buff[11 + 1];
	char* _Idx = buff + 11;
	boolean _Neg = 0;

	*_Idx = '\0';

	if(i == 0) {
		*--_Idx = '0';
		lcd_puts(x, y, color, _Idx);
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

	lcd_puts(x, y, color, _Idx);
}

void lcd_puthex( uint16 x, uint16 y, uint8 color, uint32 i )
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

	lcd_puts(x, y, color, p);
}

void lcd_putchar_x2( uint16 x, uint16 y, uint8 color, char ch )
{
    uint8 i, j, _Idx, iBit;
    _Idx = 0;
    uint8* bit;

    bit = font + ch * 16;
    for(j = 0; j < 32; ++j) {
    	iBit = 0;
    	for(i = 0; i < 16; ++i) {
    		if(bit[_Idx] & (0x80 >> iBit))
    			lcd_putpixel(x + i, y + j, color);
    		else
    			lcd_putpixel(x + i, y + j, WHITE);

    		if(i % 2 == 1)
    			++iBit;
    	}
    	if(j % 2 == 1)
    		++_Idx;
    }
}

void lcd_puts_x2( uint16 x, uint16 y, uint8 color, char *s )
{
    uint16 i = 0;
    while(s[i] != '\0') {
    	lcd_putchar_x2(x, y, color, s[i]);
    	++i;
    	x += 16;
    }
}

void lcd_putint_x2( uint16 x, uint16 y, uint8 color, int32 i )
{
	char buff[11 + 1];
	char* _Idx = buff + 11;
	boolean _Neg = 0;

	*_Idx = '\0';

	if(i == 0) {
		*--_Idx = '0';
		lcd_puts_x2(x, y, color, _Idx);
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

	lcd_puts_x2(x, y, color, _Idx);
}

void lcd_puthex_x2( uint16 x, uint16 y, uint8 color, uint32 i )
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

	lcd_puts_x2(x, y, color, p);
}

void lcd_putWallpaper( uint8 *bmp )
{
    uint32 headerSize;

    uint16 x, ySrc, yDst;
    uint16 offsetSrc, offsetDst;

    headerSize = bmp[10] + (bmp[11] << 8) + (bmp[12] << 16) + (bmp[13] << 24);

    bmp = bmp + headerSize;
    
    for( ySrc=0, yDst=LCD_HEIGHT-1; ySrc<LCD_HEIGHT; ySrc++, yDst-- )                                                                       
    {
        offsetDst = yDst*LCD_WIDTH/2;
        offsetSrc = ySrc*LCD_WIDTH/2;
        for( x=0; x<LCD_WIDTH/2; x++ )
            lcd_buffer[offsetDst+x] = ~bmp[offsetSrc+x];
    }
}
