#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <math.h>

#define VERTICAL_MODE                       01
#define HORIZONTAL_MODE                     02

#define SeeedGrayOLED_Address               0x3c
#define SeeedGrayOLED_Command_Mode          0x80
#define SeeedGrayOLED_Data_Mode             0x40

#define SeeedGrayOLED_Display_Off_Cmd       0xAE
#define SeeedGrayOLED_Display_On_Cmd        0xAF

#define SeeedGrayOLED_Normal_Display_Cmd    0xA4
#define SeeedGrayOLED_Inverse_Display_Cmd   0xA7
#define SeeedGrayOLED_Activate_Scroll_Cmd   0x2F
#define SeeedGrayOLED_Dectivate_Scroll_Cmd  0x2E
#define SeeedGrayOLED_Set_ContrastLevel_Cmd 0x81

#define Scroll_Left             0x00
#define Scroll_Right            0x01

#define Scroll_2Frames          0x7
#define Scroll_3Frames          0x4
#define Scroll_4Frames          0x5
#define Scroll_5Frames          0x0
#define Scroll_25Frames         0x6
#define Scroll_64Frames         0x1
#define Scroll_128Frames        0x2
#define Scroll_256Frames        0x3


int i2c_fd;


#if defined(__arm__) && !defined(PROGMEM)
  #define PROGMEM
  #define pgm_read_byte(STR) STR
#endif

// 8x8 Font ASCII 32 - 127 Implemented
// Users can modify this to support more characters(glyphs)
// BasicFont is placed in code memory.

// This font can be freely used without any restriction(It is placed in public domain)
const unsigned char BasicFont[][8] PROGMEM=
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x07,0x00,0x07,0x00,0x00,0x00},
  {0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00},
  {0x00,0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00},
  {0x00,0x23,0x13,0x08,0x64,0x62,0x00,0x00},
  {0x00,0x36,0x49,0x55,0x22,0x50,0x00,0x00},
  {0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x22,0x1C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x2A,0x1C,0x2A,0x08,0x00,0x00},
  {0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00},
  {0x00,0xA0,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
  {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
  {0x00,0x3E,0x51,0x49,0x45,0x3E,0x00,0x00},
  {0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00},
  {0x00,0x62,0x51,0x49,0x49,0x46,0x00,0x00},
  {0x00,0x22,0x41,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x00},
  {0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00},
  {0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x00},
  {0x00,0x01,0x71,0x09,0x05,0x03,0x00,0x00},
  {0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00},
  {0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00},
  {0x00,0x00,0xAC,0x6C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00},
  {0x00,0x14,0x14,0x14,0x14,0x14,0x00,0x00},
  {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x51,0x09,0x06,0x00,0x00},
  {0x00,0x32,0x49,0x79,0x41,0x3E,0x00,0x00},
  {0x00,0x7E,0x09,0x09,0x09,0x7E,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x22,0x1C,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x51,0x72,0x00,0x00},
  {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00},
  {0x00,0x41,0x7F,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x40,0x41,0x3F,0x01,0x00,0x00},
  {0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00},
  {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00},
  {0x00,0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00},
  {0x00,0x7F,0x04,0x08,0x10,0x7F,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00},
  {0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x00},
  {0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00},
  {0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x00},
  {0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00},
  {0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00},
  {0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x00},
  {0x00,0x3F,0x40,0x38,0x40,0x3F,0x00,0x00},
  {0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00},
  {0x00,0x03,0x04,0x78,0x04,0x03,0x00,0x00},
  {0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00},
  {0x00,0x41,0x41,0x7F,0x00,0x00,0x00,0x00},
  {0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x00},
  {0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00},
  {0x00,0x01,0x02,0x04,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00},
  {0x00,0x7F,0x48,0x44,0x44,0x38,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x28,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x48,0x7F,0x00,0x00},
  {0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00},
  {0x00,0x08,0x7E,0x09,0x02,0x00,0x00,0x00},
  {0x00,0x18,0xA4,0xA4,0xA4,0x7C,0x00,0x00},
  {0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00},
  {0x00,0x00,0x7D,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x80,0x84,0x7D,0x00,0x00,0x00,0x00},
  {0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x00},
  {0x00,0x41,0x7F,0x40,0x00,0x00,0x00,0x00},
  {0x00,0x7C,0x04,0x18,0x04,0x78,0x00,0x00},
  {0x00,0x7C,0x08,0x04,0x7C,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x38,0x00,0x00,0x00},
  {0x00,0xFC,0x24,0x24,0x18,0x00,0x00,0x00},
  {0x00,0x18,0x24,0x24,0xFC,0x00,0x00,0x00},
  {0x00,0x00,0x7C,0x08,0x04,0x00,0x00,0x00},
  {0x00,0x48,0x54,0x54,0x24,0x00,0x00,0x00},
  {0x00,0x04,0x7F,0x44,0x00,0x00,0x00,0x00},
  {0x00,0x3C,0x40,0x40,0x7C,0x00,0x00,0x00},
  {0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x00},
  {0x00,0x3C,0x40,0x30,0x40,0x3C,0x00,0x00},
  {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00},
  {0x00,0x1C,0xA0,0xA0,0x7C,0x00,0x00,0x00},
  {0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00},
  {0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x36,0x08,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x01,0x02,0x01,0x00,0x00},
  {0x00,0x02,0x05,0x05,0x02,0x00,0x00,0x00} 
};



void init() {
	
	sendCommand(0xFD); // Unlock OLED driver IC MCU interface from entering command. i.e: Accept commands
    sendCommand(0x12);
    sendCommand(0xAE); // Set display off
    sendCommand(0xA8); // set multiplex ratio
    sendCommand(0x5F); // 96
    sendCommand(0xA1); // set display start line
    sendCommand(0x00);
    sendCommand(0xA2); // set display offset
    sendCommand(0x60);
    sendCommand(0xA0); // set remap
    sendCommand(0x46);
    sendCommand(0xAB); // set vdd internal
    sendCommand(0x01); //
    sendCommand(0x81); // set contrasr
    sendCommand(0x53); // 100 nit
    sendCommand(0xB1); // Set Phase Length
    sendCommand(0X51); //
    sendCommand(0xB3); // Set Display Clock Divide Ratio/Oscillator Frequency
    sendCommand(0x01);
    sendCommand(0xB9); //
    sendCommand(0xBC); // set pre_charge voltage/VCOMH
    sendCommand(0x08); // (0x08);
    sendCommand(0xBE); // set VCOMH
    sendCommand(0X07); // (0x07);
    sendCommand(0xB6); // Set second pre-charge period
    sendCommand(0x01); //
    sendCommand(0xD5); // enable second precharge and enternal vsl
    sendCommand(0X62); // (0x62);
    sendCommand(0xA4); // Set Normal Display Mode
    sendCommand(0x2E); // Deactivate Scroll
    sendCommand(0xAF); // Switch on display
    delay(100);

    // Row Address
    sendCommand(0x75);    // Set Row Address 
    sendCommand(0x00);    // Start 0
    sendCommand(0x5f);    // End 95 


    // Column Address
    sendCommand(0x15);    // Set Column Address 
    sendCommand(0x08);    // Start from 8th Column of driver IC. This is 0th Column for OLED 
    sendCommand(0x37);    // End at  (8 + 47)th column. Each Column has 2 pixels(segments)

    // Init gray level for text. Default:Brightest White
    grayH= 0xF0;
    grayL= 0x0F;
}

	#define COMMAND_MODE 0x80
// cmd will be a hex code specifying a command defined in the
// display’s datasheet
void sendCommand(unsigned char cmd) {
	
	char buf[2];
	buf[0] = COMMAND_MODE;
	buf[1] = cmd;
	int result = write(i2c_fd, buf, 2);
	
	if(result != 2) {
		
		printf("Unable to write to i2c register\n");
		exit(-1);
	}
}


void setContrastLevel(unsigned char ContrastLevel) {
	
    sendCommand(SeeedGrayOLED_Set_ContrastLevel_Cmd);
    sendCommand(ContrastLevel);
}
	
void setHorizontalMode() {
	
	
	sendCommand(0xA0); // remap to
	sendCommand(0x42); // horizontal mode

	// Row Address
	sendCommand(0x75);    // Set Row Address 
	sendCommand(0x00);    // Start 0
	sendCommand(0x5f);    // End 95 

	// Column Address
	sendCommand(0x15);    // Set Column Address 
	sendCommand(0x08);    // Start from 8th Column of driver IC. This is 0th Column for OLED 
	sendCommand(0x37);    // End at  (8 + 47)th column. Each Column has 2 pixels(or segments)

}


void setVerticalMode() {
	
	sendCommand(0xA0); // remap to
    sendCommand(0x46); // Vertical mode
	
}



void setTextXY(unsigned char Row, unsigned char Column) {
	
	//Column Address
    sendCommand(0x15);             /* Set Column Address */
    sendCommand(0x08+(Column*4));  /* Start Column: Start from 8 */
    sendCommand(0x37);             /* End Column */
    // Row Address
    sendCommand(0x75);             /* Set Row Address */
    sendCommand(0x00+(Row*8));     /* Start Row*/
    sendCommand(0x07+(Row*8));     /* End Row*/
	
}


void clearDisplay() {
	
	unsigned char i,j;
	
	for(j=0;j<48;j++)
    {
        for(i=0;i<96;i++)  //clear all columns
        {
            sendData(0x00);
        }
    }
}

#define DATA_MODE 0x40
// c will contain the data you wish to display
void sendData(unsigned char c) {
	
	char buf[2];
	buf[0] = DATA_MODE;
	buf[1] = c;
	
	int result = write(i2c_fd, buf, 2);
	
	if(result != 2) {
		printf("Error sending data...\n");
		exit(-1);
	}
}


void setGrayLevel(unsigned char grayLevel) {
	
    grayH = (grayLevel << 4) & 0xF0;
    grayL =  grayLevel & 0x0F;
	
}


void putChar(unsigned char C) {
	
    if(C < 32 || C > 127) //Ignore non-printable ASCII characters. This can be modified for multilingual font.
    {
        C=' '; //Space
    }
	
	for(char i=0;i<8;i=i+2)
    {
        for(char j=0;j<8;j++)
        {
            // Character is constructed two pixel at a time using vertical mode from the default 8x8 font
            char c=0x00;
            char bit1=(pgm_read_byte(&BasicFont[C-32][i]) >> j)  & 0x01;  
            char bit2=(pgm_read_byte(&BasicFont[C-32][i+1]) >> j) & 0x01;
           // Each bit is changed to a nibble
            c|=(bit1)?grayH:0x00;
            c|=(bit2)?grayL:0x00;

            sendData(c);
        }
    }
}
	
void putString(const char *String)
{
    unsigned char i;
	i = 0;
    while(String[i])
    {
        putChar(String[i]);
        i++;
    }
}
	
	
	
	
// The following will display the string “hello world” 12 times,
// once per line
int main(int argc, char *argv[]) {
	
	// Function to initalize the I2C device for use
	// char *bus => the I2C device to use, “/dev/i2c-2”
	// int address => the address for the device, 0x3C
	
	char *bus = "/dev/i2c-2";
	
	i2c_fd = open(bus, O_RDWR);
	
	if(i2c_fd < 0) {
		
		printf("i2c: Unable to open bus for read/write (%s)\n", bus);
		exit(-1);
	}
	
	int result;
	result = ioctl(i2c_fd, I2C_SLAVE, 0x3c);
	
	if(result < 0) {
		
		printf("i2c: Unable to set i2c device to slave address\n");
		exit(-1);
	}
	
	init();
	clearDisplay();
	setVerticalMode();
	
	for (int i = 0; i < 12; i++) {
		
		setTextXY(i,0);
		setGrayLevel(i);
		putString("hello world");
	}
	
	return 0;
}

	

	
