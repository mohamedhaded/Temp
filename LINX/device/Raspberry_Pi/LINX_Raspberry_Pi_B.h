#ifndef LINX_RASPBERRY_PI_B_H
#define LINX_RASPBERRY_PI_B_H


/****************************************************************************************
**  Defines
****************************************************************************************/	
#define DEVICE_NAME_LEN 20
#define NUM_DIO_CHANS 17

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
		static const unsigned char m_deviceName[DEVICE_NAME_LEN];
		static const unsigned char m_DIOChans[NUM_DIO_CHANS];
		
		/****************************************************************************************
		**  Constructors
		****************************************************************************************/
		LINXRaspberryPi_B();
			
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