//////////////////////////////////////////////////////////////////////////////////////////
//
//  Functions code for the MAX30003 WING board
//
//  Arduino UNO R3 to MAX30003 WING connections:
//
//  |MAX30003 pin label| Pin Function         |Arduino Connection|
//  |----------------- |:--------------------:|-----------------:|
//  | MISO             | Slave Out            |  D12             |
//  | MOSI             | Slave In             |  D11             |
//  | SCLK             | Serial Clock         |  D13             |
//  | CS               | Chip Select          |  D10             |
//  | VCC              | Digital VDD          |  3.3V            |
//  | GND              | Digital Gnd          |  GND             |
//  | INTB             | Interrupt            |  D3              |
//
//  Copyright © Md Abdur Rahman 
//
/////////////////////////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include "max30003.h"

#define MAX30003_SPI_SPEED 5000000 // I used 5 MHz

SPISettings SPI_SETTINGS(MAX30003_SPI_SPEED, MSBFIRST, SPI_MODE0); // I used SPI Mode 0, so that it can sample data on rising edge.

MAX30003::MAX30003(int cs_pin)
{
    _cs_pin=cs_pin;
    pinMode(_cs_pin, OUTPUT);
    digitalWrite(_cs_pin,HIGH); // At initial stage, chip select pin is High. 
}

void MAX30003::max30003RegWrite (unsigned char WRITE_ADDRESS, unsigned long data)
{
    // now combine the register address and the command into one byte:
    byte dataToSend = (WRITE_ADDRESS<<1) | WREG;

    SPI.beginTransaction(SPI_SETTINGS);
     // take the chip select low to select the device:
    digitalWrite(_cs_pin, LOW);

    delay(2);
    SPI.transfer(dataToSend);
    SPI.transfer(data>>16);
    SPI.transfer(data>>8);
    SPI.transfer(data);
    delay(2);

    // take the chip select high to de-select:
    digitalWrite(_cs_pin, HIGH);
    SPI.endTransaction();
}


void MAX30003::max30003SwReset(void)
{
    max30003RegWrite(SW_RST,0x000000);
    delay(100);
}

void MAX30003::max30003Synch(void)
{
    max30003RegWrite(SYNCH,0x000000);
}

void MAX30003::max30003RegRead(uint8_t Reg_address, uint8_t * buff)
{
    uint8_t spiTxBuff;

    SPI.beginTransaction(SPI_SETTINGS);
    digitalWrite(_cs_pin, LOW);

    spiTxBuff = (Reg_address<<1 ) | RREG;
    SPI.transfer(spiTxBuff); //Send register location

    for ( int i = 0; i < 3; i++)
    {
       buff[i] = SPI.transfer(0xff);
    }

    digitalWrite(_cs_pin, HIGH);
    SPI.endTransaction();
}


bool MAX30003::max30003ReadInfo(void)
{
    uint8_t spiTxBuff;
    uint8_t readBuff[4] ;

    SPI.beginTransaction(SPI_SETTINGS);
    digitalWrite(_cs_pin, LOW);

    spiTxBuff = (INFO << 1 ) | RREG;
    SPI.transfer(spiTxBuff); //Send register location

    for ( int i = 0; i < 3; i++)
    {
       readBuff[i] = SPI.transfer(0xff);
    }

    digitalWrite(_cs_pin, HIGH);
    SPI.endTransaction();

    if((readBuff[0]&0xf0) == 0x50 ){

      Serial.println("max30003 is ready");
      Serial.print("Rev ID:  ");
      Serial.println((readBuff[0]&0xf0));

      return true;
    }else{

      Serial.println("max30003 read info error\n");
      return false;
    }

    return false;
}

void MAX30003::max30003ReadData(int num_samples, uint8_t * readBuffer)
{
    uint8_t spiTxBuff;

    SPI.beginTransaction(SPI_SETTINGS);
    digitalWrite(_cs_pin, LOW);

    spiTxBuff = (ECG_FIFO<<1 ) | RREG;
    SPI.transfer(spiTxBuff); //Send register location

    for ( int i = 0; i < num_samples*3; ++i)
    {
      readBuffer[i] = SPI.transfer(0x00);
    }

    digitalWrite(_cs_pin, HIGH);
    SPI.endTransaction();
}

void MAX30003::max30003Begin()
{
    max30003SwReset();
    delay(100);
    max30003RegWrite(CNFG_GEN, 0x081007);
    delay(100);
    max30003RegWrite(CNFG_CAL, 0x720000);  // 0x700000
    delay(100);
    max30003RegWrite(CNFG_EMUX,0x0B0000);
    delay(100);
    max30003RegWrite(CNFG_ECG, 0x805000);  // d23 - d22 : 10 for 250sps , 00:500 sps
    delay(100);

    max30003RegWrite(CNFG_RTOR1,0x3fc600);
    max30003Synch();
    delay(100);
}

void MAX30003::max30003BeginRtorMode()
{
    max30003SwReset();
    delay(100);
    max30003RegWrite(CNFG_GEN, 0x081217); 
    delay(100);
    max30003RegWrite(CNFG_ECG, 0x835000); 
    delay(100);
    max30003RegWrite(CNFG_RTOR1,0x3FB300);
    delay(100);
    max30003RegWrite(MNGR_INT, 0x180014);
    delay(100);
    max30003RegWrite(EN_INT,0x800403);
    delay(100);
    max30003RegWrite(MNGR_DYN, 0x0F0000);
    delay(100);
    max30003RegWrite(CNFG_EMUX, 0x000000);
    delay(100);
    max30003Synch();
    delay(100);
}

void MAX30003::getEcgSamples(void) {
    uint8_t regReadBuff[4];
    uint16_t mV = 1000;
    max30003RegRead(ECG_FIFO, regReadBuff);

    unsigned long data0 = (unsigned long)(regReadBuff[0]) << 24;
    unsigned long data1 = (unsigned long)(regReadBuff[1]) << 16;
    unsigned long data2 = (unsigned long)(regReadBuff[2]) << 8;
    unsigned long data3 = (unsigned long)(regReadBuff[3]);

    unsigned long data = data0 | data1 | data2 | data3;
    data = data >> 6; // Removes the last 6 bits (3 bits of ETAG and 3 bits of PTAG)
    data = data & 0x3FFFF; // only getting the 18-bit value

    ecgdata = (signed long)(data);
	ecgdata = 262144-ecgdata;//ecgdata is in left justified 2's complement format, so for sample voltage will be in decimal = 2^18-ecgdata.

    // Calculate the sampling voltage
    //float ecgvalue = (ecgdata * 5.0) / (160.0 * (1 << 18 - 1)) * 1000.0;
}


void MAX30003::getHRandRR(void)
{
    uint8_t regReadBuff[4];
    max30003RegRead(RTOR, regReadBuff);

    unsigned long RTOR_msb = (unsigned long) (regReadBuff[0]);
    unsigned char RTOR_lsb = (unsigned char) (regReadBuff[1]);
    unsigned long rtor = (RTOR_msb<<8 | RTOR_lsb);
    rtor = ((rtor >>2) & 0x3fff) ; // only getting 14 bits

    float hr =  60 /((float)rtor*0.0078125);
    heartRate = (unsigned int)hr;

    unsigned int RR = (unsigned int)rtor* (7.8125) ;  //8ms
    RRinterval = RR;
}
