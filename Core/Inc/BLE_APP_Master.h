/*
 * BLE_APP_Master.h V2
 *
 *For Chip: RYB080E
 *
 *  Created on: Jun 17, 2021
 *      Author: Wayne Wu
 *
 */


#ifndef INC_BLE_APP_MASTER_H_
#define INC_BLE_APP_MASTER_H_



#endif /* INC_BLE_APP_MASTER_H_ */

/* Exported types ------------------------------------------------------------*/
#define Uart_Biffer_Size 24
#define Uart_RBiffer_Size 1
typedef struct
{
	_Bool sendflag;
	_Bool ConTimeOutflag;
	_Bool Revflag;
	_Bool Rstflag;
	//usart transfermit
	unsigned char __attribute__ ((aligned (32))) buffer[Uart_Biffer_Size];
	//char __attribute__ ((aligned (32))) buffer[Uart_Biffer_Size];
	//unsigned char __attribute__ ((aligned (32))) Rbuffer[Uart_RBiffer_Size];
	uint8_t Rbuffer;
	char RevData[24];
	char RevTemp[10];
	uint8_t RevTempSize;
	uint8_t ConnCheck[5];
	uint8_t RxCount;
	uint32_t sendTimeout;
	uint32_t RevTimeout;
	uint32_t bufferSize;
	uint8_t Rev_Timeout_count;
	char Status;
} USART_BLE;

typedef struct
{
	_Bool digitlag;
	char DACDataStr[2][10];
	uint16_t DACDataInt[2];
} DAC_t;

/* Exported constants --------------------------------------------------------*/
enum
{
	Connecting,
	WaitConRsp,
	WaitRevData,
};

typedef enum
{
	NoTimeOut,
	ReConnect,
}Connect_StatusTypeDef;
/* Exported macro ------------------------------------------------------------*/
#define min(a, b) ((a) < (b)) ? (a) : (b)
#define max(a, b) ((a) > (b)) ? (a) : (b)
/* Exported functions prototypes ---------------------------------------------*/
void BLE_USART(UART_HandleTypeDef *huart);
void Connect_Rep_Check(void);
Connect_StatusTypeDef Check_Rev_Timeout();
Connect_StatusTypeDef Check_Con_Timeout();
void BLE_HD_RESET(void);
void Get_SlaveDevice_Data( void);
void BLE_Status_Init(UART_HandleTypeDef *huart);
void Send_ConSuccessed_ACK(UART_HandleTypeDef *huart);
void Send_RevSuccessed_ACK(UART_HandleTypeDef *huart);

/* Private defines ---ee--------------------------------------------------------*/
#define GLED_Port GPIOD
#define RLED_Port GPIOD
#define Rst_Port GPIOC
#define GLED GPIO_PIN_12
#define RLED GPIO_PIN_13
#define Rst GPIO_PIN_4

//command define
//#define ConnectSlaveCMD "AT+CON=E07DEA75DF95\r\n"
#define ConnectSlaveCMD "AT+CON=E07DEA8104F8\r\n"
//#define ConnectSlaveCMD "AT+CON=A434F1A5AE29\r\n" //test module
