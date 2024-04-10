#define     ss_pin_high         *outb |= 0x04
#define     ss_pin_low          *outb &= ~0x04
#define     complete_transfer   *spsr & 0x80
#define     command_en_high     0x20
#define     command_en_low      0x00
#define     data_en_high        0x30
#define     data_en_low         0x10




volatile char *dirb = 0x24;
volatile char *outb = 0x25;
volatile char *dirc = 0x27;
volatile char *outc = 0x28;
volatile char *dird = 0x2A;
volatile char *ind  = 0x29;
volatile char *spcr = 0x4C;
volatile char *spdr = 0x4E;
volatile char *spsr = 0x4D;




//-------------------------------------------------------------------------------------------------//
//                                          SPI Functions                                          //
//-------------------------------------------------------------------------------------------------//

void SPI_MasterInit(void){
 *dirb = 0x2c;                   // SCK, MOSI, SS set as Output & MISO as Input
 ss_pin_high;                    // Make SS pin High
 *spcr |= 0x51;                  // SPE = MSTR = SPR0 = 1
}

void SPI_Transmit(volatile uint8_t cData){
 *spdr = cData;
 ss_pin_low;                // Make SS pin Low
 while(!(complete_transfer));   // Wait until the Serial Transfer is completed
 ss_pin_high;               // Make SS pin High
}




//---------------------------------------------------------------------------------------------------//
//                                          LCD Functions                                            //
//---------------------------------------------------------------------------------------------------//

void send_lcd_command(volatile uint8_t cmd){
volatile uint8_t high_nibble = (cmd >> 4) & 0x0f;
volatile uint8_t low_nibble = cmd & 0x0f;
 SPI_Transmit(high_nibble + command_en_high);
 SPI_Transmit(high_nibble + command_en_low);
 delayMicroseconds(50);
 SPI_Transmit(low_nibble + command_en_high);
 SPI_Transmit(low_nibble + command_en_low);
 delayMicroseconds(500);
}

void send_lcd_data(volatile uint8_t data1){
volatile uint8_t high_nibble = (data1 >> 4) & 0x0f;
volatile uint8_t low_nibble = data1 & 0x0f;
 SPI_Transmit(high_nibble + data_en_high);
 SPI_Transmit(high_nibble + data_en_low);
 delayMicroseconds(50);
 SPI_Transmit(low_nibble + data_en_high);
 SPI_Transmit(low_nibble + data_en_low);
 delayMicroseconds(50);
}

void lcd_init(){
send_lcd_command (0x02); // 4bit mode
send_lcd_command (0x28); // Initialization of 16X2 LCD in 4bit mode
send_lcd_command (0x0C); // Display ON Cursor OFF
send_lcd_command (0x06); // Auto Increment cursor
send_lcd_command (0x01); // Clear display
send_lcd_command (0x80); // Cursor at home position
delayMicroseconds(1000);
}




//---------------------------------------------------------------------------------------------------//
//                                    KEYBOARD SCANNING FUNCTION                                     //
//---------------------------------------------------------------------------------------------------//

volatile char checkKey(){
 volatile char row, col, key;
 for(row=0; row<4; row++){
 *outc = 1 << row;
 if(*ind){
  col = ((*ind) >> 4) & 0x0F;   // Shifting the value to lower nibble
  break;
  }
}

 if(row==0){
 if(col==1) key='7';
 else if(col==2) key='8';
 else if(col==4) key='9';
 else if(col==8) key='/';
 }
 else if(row==1){
 if(col==1) key='4';
 else if(col==2) key='5';
 else if(col==4) key='6';
 else if(col==8) key='*';
 }
 else if(row==2){
 if(col==1) key='1';
 else if(col==2) key='2';
 else if(col==4) key='3';
 else if(col==8) key='-';
 }
else if(row==3){
 if(col==1) key='C';
 else if(col==2) key='0';
 else if(col==4) key='=';
 else if(col==8) key='+';
 }
 else key=0;
 return key;
}




//-------------------------------------------------------------------------------------------------//
//                                          Main Function                                          //
//-------------------------------------------------------------------------------------------------//

void setup(){
 *dird = 0;                     // PORTD sets as input
 *dirc = 0x0F;                  // PORTC lower nibble sets as output
 volatile char character;
 SPI_MasterInit();
 lcd_init();

 while(1){
 character = checkKey();
 if(character) send_lcd_data(character);
 for(volatile long i=0; i<100000; i++);      // keypad debouncing delay
 }
}

