#ifndef LINX_RASPBERRY_PI_B_H
#define LINX_RASPBERRY_PI_B_H


/****************************************************************************************
**  Defines
****************************************************************************************/	
#define DEVICE_NAME_LEN 20
#define NUM_DIO_CHANS 17

#define NUM_SPI_CHANS 1
#define NUM_SPI_SPEEDS 13

#define NUM_I2C_CHANS 2

#define NUM_UART_CHANS 1
#define NUM_UART_SPEEDS 19

/****************************************************************************************
**  Includes
****************************************************************************************/		
#include "LINX_Raspberry_Pi.h"
	
class LINXRaspberryPi_B : public LINXRaspberryPi
{
	public:	
		/****************************************************************************************
		**  Variables
		****************************************************************************************/		
		//System
		static const unsigned char m_deviceName[DEVICE_NAME_LEN];
		
		//DIO
		static const unsigned char m_DIOChans[NUM_DIO_CHANS];
		
		//SPI
		static const unsigned char m_SPIChans[NUM_SPI_CHANS];
		static int m_SPIHandles[NUM_SPI_CHANS];
		static const char m_SPIPaths[NUM_SPI_CHANS][SPI_PATH_LEN];
		static unsigned long m_SPISupportedSpeeds[NUM_SPI_SPEEDS];
		static unsigned long m_SPISetSpeeds[NUM_SPI_CHANS];
		static unsigned char m_SPIBitOrders[NUM_SPI_CHANS];
		
		//I2C
		static unsigned char m_I2CChans[NUM_I2C_CHANS];
		static int m_I2CHandles[NUM_I2C_CHANS];
		static const char m_I2CPaths[NUM_I2C_CHANS][I2C_PATH_LEN];
		
		//UART
		static unsigned char m_UartChans[NUM_UART_CHANS];
		static int m_UartHandles[NUM_UART_CHANS];
		static const char m_UartPaths[NUM_UART_CHANS][UART_PATH_LEN];
		static unsigned long m_UartSupportedSpeeds[NUM_UART_SPEEDS];
		static unsigned long m_UartSupportedSpeedsCodes[NUM_UART_SPEEDS];
		
		/****************************************************************************************
		**  Constructors /  Destructor
		****************************************************************************************/
		LINXRaspberryPi_B();
		
		~LINXRaspberryPi_B();
			
		/****************************************************************************************
		**  Functions
		****************************************************************************************/
		
	private:
		/****************************************************************************************
		**  Variables
		****************************************************************************************/		
				
		
		/****************************************************************************************
		**  Functions
		****************************************************************************************/
		
		
};


#endif //LINX_RASPBERRY_PI_B_H