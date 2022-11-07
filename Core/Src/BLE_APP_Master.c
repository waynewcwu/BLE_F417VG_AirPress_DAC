/*
 * BLE_APP_Master.c V2
 *
 *For Chip: RYB080E
 *
 *  Created on: Jun 17, 2021
 *      Author: Wayne Wu
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
#include "stdio.h"
#include "ctype.h"
#include "stdlib.h"


/* Private typedef -----------------------------------------------------------*/
USART_BLE USARTBLE;
DAC_t DAConv;

/* Private variables ---------------------------------------------------------*/


void BLE_USART(UART_HandleTypeDef *huart)
{
    switch(USARTBLE.Status)
    {

    	case Connecting:

    		if(USARTBLE.Rstflag==1)
    		{
    			BLE_HD_RESET();//Reset BLE module
    			USARTBLE.Rstflag=0;
    		}
    		sprintf(USARTBLE.buffer, ConnectSlaveCMD);//Connect BLE module
    		USARTBLE.bufferSize = min(Uart_Biffer_Size, strlen(USARTBLE.buffer));
    		USARTBLE.sendTimeout = 10 ;
    		/**
    				* @param huart   UART handle.
    				* @param pData   Pointer to data buffer.
    				* @param Size    Amount of data to be received.
    				* @param Timeout Timeout duration.(ms)
    				*/
    		if(USARTBLE.sendflag==1)//when time out then Reconnect
    		{
    			while(HAL_UART_Transmit(huart, &USARTBLE.buffer, USARTBLE.bufferSize, USARTBLE.sendTimeout )!=HAL_OK);
    			memset( USARTBLE.buffer, 0, strlen(USARTBLE.buffer) ); //clear transmit data
    			USARTBLE.Status = WaitConRsp;	//when send succeed wait BLE device
    			USARTBLE.sendflag = 0;

    		}
    		break;

    	case WaitConRsp:
    		while(!USARTBLE.Revflag)
			{
    			if(Check_Con_Timeout()==ReConnect)
    				return;

    		}
    		Connect_Rep_Check();
    		HAL_Delay(1);
    		Send_ConSuccessed_ACK(huart);
    		USARTBLE.ConTimeOutflag = 0;
    		USARTBLE.Rev_Timeout_count = 0;
    		break;

    	case WaitRevData:
    	    while(!USARTBLE.Revflag)
    	    {
    	    	if(Check_Rev_Timeout()==ReConnect)
    	    		return;
    	    }
    	    Get_SlaveDevice_Data();
    	    Send_RevSuccessed_ACK(huart);
    	    break;
    }

}
void Get_SlaveDevice_Data(void)
{
	USARTBLE.Rev_Timeout_count=0; //reset receive timeout

	//copy receive data to remp array
	memset( USARTBLE.RevTemp, 0, strlen(USARTBLE.RevTemp) ); //clear Receive data temp array
	USARTBLE.RevTempSize = strlen(USARTBLE.RevData)-2;
	strncpy(USARTBLE.RevTemp, USARTBLE.RevData, USARTBLE.RevTempSize);
	memset( USARTBLE.RevData, 0, strlen(USARTBLE.RevData) ); //clear Receive data
	USARTBLE.Revflag=0;
	//Separate data
	unsigned char byteCount=0, ArrayNo=0, ArrayByte=0;
	char SeparateStr[]=", ";
	for(byteCount = 0; byteCount < USARTBLE.RevTempSize; byteCount++)
	{
		if(USARTBLE.RevTemp[byteCount] == SeparateStr[0] && USARTBLE.RevTemp[byteCount+1]==SeparateStr[1])
		{
			ArrayNo++;
			byteCount+=2;
			ArrayByte = 0;
		}
		DAConv.DACDataStr[ArrayNo][ArrayByte] = USARTBLE.RevTemp[byteCount];
		if(!isdigit(DAConv.DACDataStr[ArrayNo][ArrayByte]))
			return;
		ArrayByte++;
	}
	//convert string to integral
	for(ArrayNo = 0; ArrayNo <= 1 ;ArrayNo++) //2 DAC channel, arrayNo=0,1
	{
		 DAConv.DACDataInt[ArrayNo] = atoi(DAConv.DACDataStr[ArrayNo]);
		 memset(DAConv.DACDataStr[ArrayNo], 0, sizeof(DAConv.DACDataStr[ArrayNo])); //clear DAC data value
	}

}

void Connect_Rep_Check(void)
{
	memset(USARTBLE.ConnCheck, 0, strlen(USARTBLE.ConnCheck) ); //clear connect response string
	strncpy(USARTBLE.ConnCheck, USARTBLE.RevData, 5); //copy receive data at front 5 byte
	char ConSucceed[]="+MTU:";
	if(strcmp( USARTBLE.ConnCheck, ConSucceed ) == 0)
	{
		HAL_GPIO_WritePin(RLED_Port, RLED, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GLED_Port, GLED, GPIO_PIN_SET);
		USARTBLE.Status = WaitRevData;
	}
	else
	{
		USARTBLE.Status = Connecting;
	}

	memset( USARTBLE.RevData, 0, strlen(USARTBLE.RevData) ); //clear Receive data
	USARTBLE.Revflag=0;
}
          
Connect_StatusTypeDef Check_Rev_Timeout()
{
	if(USARTBLE.ConTimeOutflag)
	{
	   USARTBLE.Rev_Timeout_count++;
	   if(USARTBLE.Rev_Timeout_count==10)
	   {
		   USARTBLE.Rev_Timeout_count=0;
		   USARTBLE.Status = Connecting;	//When 5 second timeout that not receive any data, then turn to reconnct slave device.
		   HAL_GPIO_WritePin(RLED_Port, RLED, GPIO_PIN_SET);
		   HAL_GPIO_WritePin(GLED_Port, GLED, GPIO_PIN_RESET);
		   USARTBLE.Rstflag=1;//reset BLE module
		   return ReConnect;
	   }
	   USARTBLE.ConTimeOutflag=0;
	}
	return NoTimeOut;
}

Connect_StatusTypeDef Check_Con_Timeout()
{
	if(USARTBLE.ConTimeOutflag)//if no receive data when time out then reconnect
	{
		USARTBLE.Rev_Timeout_count++;
    	if(USARTBLE.Rev_Timeout_count==10)//count connect fail to 10 time, reset chip.
    	{
    		USARTBLE.Rstflag = 1;
    		USARTBLE.Rev_Timeout_count = 0;
    	}

    	USARTBLE.Status = Connecting;
    	USARTBLE.ConTimeOutflag = 0;
    	return ReConnect;
    }
    return NoTimeOut;
}


void BLE_HD_RESET(void)
{
	 HAL_GPIO_WritePin(Rst_Port, Rst, GPIO_PIN_RESET);
	 HAL_Delay(1);
	 HAL_GPIO_WritePin(Rst_Port, Rst, GPIO_PIN_SET);
	 //wait 20ms for chip reset
	 HAL_Delay(20);
	 //wait 10sec for slave connection ready.
	 HAL_Delay(10000);
}

void BLE_Status_Init(UART_HandleTypeDef *huart)
{

	//set LED3 for connecting
	HAL_GPIO_WritePin(RLED_Port, RLED, GPIO_PIN_SET);

	//set Reset pin high level
	HAL_GPIO_WritePin(Rst_Port, Rst, GPIO_PIN_SET);

	//init BLE Status
	USARTBLE.Status = Connecting;

  	//initial BLE receive interrupt flag
  	while(HAL_UART_Receive_IT(huart ,&USARTBLE.Rbuffer,1)!=HAL_OK);

  	//set reset flag to low level
	USARTBLE.Rstflag = 0;
}

void Send_ConSuccessed_ACK(UART_HandleTypeDef *huart)
{
	//send ACK for Connect succeed
   	sprintf(USARTBLE.buffer, "ConACK\r\n");
   	USARTBLE.bufferSize = min(Uart_Biffer_Size, strlen(USARTBLE.buffer));
    HAL_UART_Transmit(huart, &USARTBLE.buffer, USARTBLE.bufferSize, USARTBLE.sendTimeout );
    memset( USARTBLE.buffer, 0, strlen(USARTBLE.buffer) ); //clear transmit data
}
void Send_RevSuccessed_ACK(UART_HandleTypeDef *huart)
{
	//send ACK for receive succeed
	sprintf(USARTBLE.buffer, "RevACK\r\n");
	USARTBLE.bufferSize = min(Uart_Biffer_Size, strlen(USARTBLE.buffer));
	HAL_UART_Transmit(huart, &USARTBLE.buffer, USARTBLE.bufferSize, USARTBLE.sendTimeout );
	memset( USARTBLE.buffer, 0, strlen(USARTBLE.buffer) ); //clear transmit data
}
