/****************************************************************************************
**  Includes
****************************************************************************************/		
#include <stdio.h>
#include <string> 
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>
#include <linux/spi/spidev.h>


#include <termios.h>		//UART Support

#include "../../LINX_Common.h"
#include "../LINX_Device.h"
#include "LINX_Raspberry_Pi.h"


/****************************************************************************************
**  Variables
****************************************************************************************/		

/****************************************************************************************
**  Constructors / Destructors 
****************************************************************************************/
LINXRaspberryPi::LINXRaspberryPi( )
{
	//-----All-----
	deviceFamily = 0x04;
	
	//LINX API Version
	linxAPIMajor = 1;
	linxAPIMinor = 1;
	linxAPISubminor = 0;	
}


/****************************************************************************************
**  Functions
****************************************************************************************/
int LINXRaspberryPi::GPIOExport (const unsigned char*  GPIOChans, const unsigned char numGPIOChans)
{
	DEBUG("Exporting GPIO");
	
	//Export File
	FILE * exportFile;	
		
	//Export GPIO Pins
	for(int i=0; i<numGPIOChans; i++)
	{
		//Open Export File
		exportFile = fopen("/sys/class/gpio/export", "w");
		
		fprintf(exportFile, "%u", GPIOChans[i]);		
		
		//Close Export File
		fclose(exportFile);		
	}
	
	return 0;
}

int LINXRaspberryPi::GPIOUnexport(const unsigned char*  GPIOChans, const unsigned char numGPIOChans)
{
	DEBUG("Unexporting GPIO");
	
	//Unexport File
	FILE * unexportFile;

	//Export GPIO Pins
	for(int i=0; i<numGPIOChans; i++)
	{
		//Open Unexport File
		unexportFile = fopen("/sys/class/gpio/unexport", "w");
		
		fprintf(unexportFile, "%u", GPIOChans[i]);			
		
		//Close Unexport File
		fclose(unexportFile);
	}
	
	return 0;
}

int LINXRaspberryPi::GPIOSetDir(unsigned char pin, unsigned char mode)
{
	DEBUG("RPI GPIO Set Dir");
	
	//GPIO Direction File Handle
	FILE *dirFile;
	
	char filePath [40];
	snprintf(filePath, 40, "/sys/class/gpio/gpio%d/direction", pin );
	dirFile = fopen(filePath, "w");
	
	if(dirFile == NULL)
	{
		printf("Failed To Open %s\n", filePath);
	}
	else
	{	
		if(mode == INPUT)
		{
			fprintf(dirFile, "in");
			fclose(dirFile);
				
		}
		else if (mode == OUTPUT)
		{
			fprintf(dirFile, "out");
			fclose(dirFile);			
		}
		else
		{
			//TODO ERROR
		}			
	}
	
	return 0;
	
}

int LINXRaspberryPi::GPIOWrite(unsigned char pin, unsigned char val)
{
	DEBUG("RPI GPIO Write");
	
	//GPIO Value File Handle
	FILE *valFile;
	
	//Build GPIO Value Path And Open
	char filePath [40];
	snprintf(filePath, 40, "/sys/class/gpio/gpio%d/value", pin );
	valFile = fopen(filePath, "w");
	
	//Write Value
	fprintf(valFile, "%u", val);	
		
	//Close GPIO Value File
	fclose(valFile);
		
	return 0;
}

int LINXRaspberryPi::digitalWrite(unsigned char numPins, unsigned char* pins, unsigned char* values)
{
	DEBUG("Digital Write At Pi Level");		
	
	for(int i=0; i<numPins; i++)
	{		
		GPIOSetDir(pins[i], 1);
		GPIOWrite( pins[i], (values[i/8] >> i%8) & 0x01);
	}
	
	return 0;
}

//----------SPI

int LINXRaspberryPi::SPIOpenMaster(unsigned char channel)
{
	//Channel Checking Is Done On Farther Up The Stack, So Just Open It
	int handle = open(SPIPaths[channel], O_RDWR);
	if (handle < 0)
	{
		DEBUG("Failed To Open SPI Channel");
		return  L_UNKNOWN_ERROR;
	}
	else
	{
		//Default To Mode 0 With No CS (We'll Use GPIO When Performing Write)
		unsigned long spi_Mode = SPI_NO_CS | SPI_MODE_0;
		int retVal = ioctl(handle, SPI_IOC_WR_MODE, &spi_Mode);		
		SPIHandles[channel] = handle;
		return L_OK;
	}
}

int LINXRaspberryPi::SPISetBitOrder(unsigned char channel, unsigned char bitOrder)
{
	//Store Bit Order.  This Will Be Used During Write
	*(SPIBitOrders+channel) = bitOrder;
}

int LINXRaspberryPi::SPISetMode(unsigned char channel, unsigned char mode)	
{
	unsigned long spi_Mode = SPI_NO_CS | (unsigned long) mode;
	int retVal = ioctl(SPIHandles[channel], SPI_IOC_WR_MODE, &spi_Mode);
	if(retVal < 0)
	{
		DEBUG("Failed To Set SPI Mode");
		return  L_UNKNOWN_ERROR;
	}
	else
	{
		return L_OK;
	}	
}

int LINXRaspberryPi::SPISetSpeed(unsigned char channel, unsigned long speed, unsigned long* actualSpeed)
{

	int index = 0;
	//Loop Over All Supported SPI Speeds
	for(index=0; index < numSPISpeeds; index++)
	{
			
			if(speed < *(SPISupportedSpeeds+index))
			{
				index = index - 1; //Use Fastest Speed Below Target Speed
				break;
			}
			//If Target Speed Is Higher Than Max Speed Use Max Speed			
	}
	*(SPISetSpeeds+channel) = *(SPISupportedSpeeds+index);
	*actualSpeed = *(SPISupportedSpeeds+index);
	return L_OK;
}

int LINXRaspberryPi::SPIWriteRead(unsigned char channel, unsigned char frameSize, unsigned char numFrames, unsigned char csChan, unsigned char csLL, unsigned char* sendBuffer, unsigned char* recBuffer)
{
	unsigned char nextByte = 0;	//First Byte Of Next SPI Frame	
	
	//SPI Hardware Only Supports MSb First Transfer.  If  Configured for LSb First Reverse Bits In Software
	if( *(SPIBitOrders+channel) == LSBFIRST )
	{
		for(int i=0; i< frameSize*numFrames; i++)
		{			
			sendBuffer[i] = reverseBits(sendBuffer[i]);
		}
	}
	
	struct spi_ioc_transfer transfer;
	
	//Set CS As Output And Make Sure GPIO CS Starts Idle	
	GPIOSetDir(csChan, OUTPUT);	
	GPIOWrite(csChan, (~csLL & 0x01) );
	
	for(int i=0; i< numFrames; i++)
	{
		//Setup Transfer
		transfer.tx_buf = (unsigned long)(sendBuffer+nextByte);
		transfer.rx_buf = (unsigned long)(recBuffer+nextByte);
		transfer.len = frameSize;
		transfer.delay_usecs = 0;
		transfer.speed_hz = *(SPISetSpeeds+channel) * 2;
		//transfer.speed_hz = 3000000;
		transfer.bits_per_word = 8;
	
		//CS Active
		GPIOWrite(csChan, csLL);
		
		//Transfer Data
		int retVal = ioctl(SPIHandles[channel], SPI_IOC_MESSAGE(1), &transfer);
		
		//CS Idle
		GPIOWrite(csChan, (~csLL & 0x01) );
		
		if (retVal < 1)
		{
			DEBUG("Failed To Send SPI Data");
			return  L_UNKNOWN_ERROR;
		}
	}	
	
	return L_OK;
}

//----------I2C----------//
int LINXRaspberryPi::I2COpenMaster(unsigned char channel)
{
	int handle = open(I2CPaths[channel], O_RDWR);
	if (handle < 0)
	{
		DEBUG("Failed To Open I2C Channel");
		return  LI2C_OPEN_FAIL;
	}
	else
	{
		I2CHandles[channel] = handle;
	}
	return L_OK;
}

int LINXRaspberryPi::I2CSetSpeed(unsigned char channel, unsigned long speed, unsigned long* actualSpeed)
{
	return  L_FUNCTION_NOT_SUPPORTED;
}

int LINXRaspberryPi::I2CWrite(unsigned char channel, unsigned char slaveAddress, unsigned char eofConfig, unsigned char numBytes, unsigned char* sendBuffer)
{
	//Check EOF - Currently Only Support 0x00
	if(eofConfig != EOF_STOP)
	{
		return LI2C_EOF;	
	}
	
	//Set Slave Address
	if (ioctl(I2CHandles[channel], I2C_SLAVE, slaveAddress) < 0) 
	{
		//Failed To Set Slave Address
		return LI2C_SADDR;
	}
		
	//Write Data
	if(write(I2CHandles[channel], sendBuffer, numBytes) != numBytes)
	{
		return LI2C_WRITE_FAIL;
	}
	
	return L_OK;
}

int LINXRaspberryPi::I2CRead(unsigned char channel, unsigned char slaveAddress, unsigned char eofConfig, unsigned char numBytes, unsigned int timeout, unsigned char* recBuffer)
{
	//Check EOF - Currently Only Support 0x00
	if(eofConfig != EOF_STOP)
	{
		return LI2C_EOF;	
	}
	
	//Set Slave Address
	if (ioctl(I2CHandles[channel], I2C_SLAVE, slaveAddress) < 0) 
	{
		//Failed To Set Slave Address
		return LI2C_SADDR;
	}
	
	if(read(I2CHandles[channel], recBuffer, numBytes) < numBytes)
	{
		return LI2C_READ_FAIL;	
	}
	return L_OK;
}

int LINXRaspberryPi::I2CClose(unsigned char channel)
{
	//Close I2C Channel
	if(close(I2CHandles[channel]) < 0)
	{
		return LI2C_CLOSE_FAIL;
	}
	
	return L_OK;
}

//UART
int LINXRaspberryPi::UartOpen(unsigned char channel, unsigned long baudRate, unsigned long* actualBaud)
{
	//Open UART	
	int handle = open(UartPaths[channel],  O_RDWR | O_NDELAY);
	
	if (handle < 0)
	{
		DEBUG("Failed To Open UART Channel");
		return  LUART_OPEN_FAIL;
	}	
	else
	{
		UartHandles[channel] = handle;
	}
	
	UartSetBaudRate(channel, baudRate, actualBaud);
	
	return L_OK;
}

int LINXRaspberryPi::UartSetBaudRate(unsigned char channel, unsigned long baudRate , unsigned long* actualBaud)
{
	
	//Get Closest Support Baud Rate Without Going Over
	
	//Loop Over All Supported SPI Speeds
	int index = 0;
	for(index=0; index < NumUartSpeeds; index++)
	{
			
			if(baudRate < *(UartSupportedSpeeds+index))
			{
				index = index - 1; //Use Fastest Baud Less Or Equal To Target Baud
				break;
			}
			//If Target Baud Is Higher Than Max Baud Use Max Baud			
	}
	
	//Store Actual Baud Used
	*actualBaud = (unsigned long) *(UartSupportedSpeeds+index);
	printf("Baud = %d\n", *(UartSupportedSpeeds+index));
	
	//Set Baud Rate
	struct termios options;	
	tcgetattr(UartHandles[channel], &options);
	
	options.c_cflag = *(UartSupportedSpeedsCodes+index) | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	
	tcflush(UartHandles[channel], TCIFLUSH);	
	tcsetattr(UartHandles[channel], TCSANOW, &options);
	
	return  L_OK;
}

int LINXRaspberryPi::UartGetBytesAvailable(unsigned char channel, unsigned char *numBytes)
{
	int bytesAtPort = -1;
	ioctl(UartHandles[channel], FIONREAD, &bytesAtPort);
	
	printf("/n Bytes Avail = %d\n", bytesAtPort);
	
	if(bytesAtPort < 0)
	{
		return LUART_AVAILABLE_FAIL;
	}
	else
	{
		*numBytes = (unsigned char) bytesAtPort;
	}
	return  L_OK;
}

int LINXRaspberryPi::UartRead(unsigned char channel, unsigned char numBytes, unsigned char* recBuffer, unsigned char* numBytesRead)
{
	//Check If Enough Bytes Are Available
	unsigned char bytesAvailable = -1;
	UartGetBytesAvailable(channel, &bytesAvailable);
	
	if(bytesAvailable >= numBytes)
	{
		//Read Bytes From Input Buffer
		int bytesRead = read(UartHandles[channel], recBuffer, numBytes);
		*numBytesRead = (unsigned char) bytesRead;
		
		if(bytesRead != numBytes)
		{
			return LUART_READ_FAIL;
		}		
	}
	return  L_OK;
}

int LINXRaspberryPi::UartWrite(unsigned char channel, unsigned char numBytes, unsigned char* sendBuffer)
{
	
	//debug
	printf("\n");
	for(int i=0; i< numBytes; i++)
	{
		printf("%c", *(sendBuffer+i));
	}
		printf("\n");
	
	
	
	int bytesSent = write(UartHandles[channel], sendBuffer, numBytes);	
	if(bytesSent != numBytes)
	{
		return LUART_WRITE_FAIL;
	}
	return  L_OK;
}

int LINXRaspberryPi::UartClose(unsigned char channel)
{
	//Close UART Channel, Return OK or Error
	if (close(UartHandles[channel]) < 0)
	{
		return LUART_CLOSE_FAIL;
	}
	return  L_OK;
}