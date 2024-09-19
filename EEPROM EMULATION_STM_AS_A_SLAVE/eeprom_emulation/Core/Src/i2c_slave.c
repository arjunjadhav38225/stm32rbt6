/*
 * i2c_slave.c
 *
 *  Created on: Jun 9, 2023
 *      Author: controllerstech
 */


#include "main.h"
#include "i2c_slave.h"
#include "string.h"

uint8_t I2C_REGISTERS[100] = {0};
uint8_t rcvd_cmpltd=0;

extern I2C_HandleTypeDef hi2c1;

#define RxSIZE  100
uint8_t RxData[RxSIZE];
uint8_t rxcount=0;
uint8_t txcount=0;
uint8_t startPosition = 0;
uint8_t bytesRrecvd = 0;
uint8_t bytesTransd = 0;

int process_data (void)
{
	int startREG = RxData[0];  // get the register address
	int numREG = bytesRrecvd;  // Get the number of registers
	int endREG = startREG + numREG -1;  // calculate the end register
	if (endREG>99)  // There are a total of 100 registers (0-99)
	{
		// clear everything and return
		memset(RxData,'\0',RxSIZE);
		rxcount =0;
		return 0;
	}
	 for (int i = 0; i < 100; i++)
			        {
			            printf("0x%02X \n", RxData[i]);
			        }
	int indx = 1;  // set the indx to 1 in order to start reading from RxData[1]
	for (int i=0; i<numREG; i++)
	{
		I2C_REGISTERS[startREG++] = RxData[indx++];  // Read the data from RxData and save it in the I2C_REGISTERS
	}
	rcvd_cmpltd=1;
	return 1;  // success
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{


	if (TransferDirection == I2C_DIRECTION_TRANSMIT)  // if the master wants to transmit the data
	{
		RxData[0] = 0;  // reset the RxData[0] to clear any residue address from previous call
		rxcount =0;
		HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_FIRST_FRAME);
	}

	else
	{
		txcount = 0;
		startPosition = RxData[0];
		RxData[0] = 0;  // Reset the start register as we have already copied it
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C_REGISTERS+startPosition+txcount, 1, I2C_FIRST_FRAME);
	}
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	txcount++;
	HAL_I2C_Slave_Seq_Transmit_IT(hi2c, I2C_REGISTERS+startPosition+txcount, 1, I2C_NEXT_FRAME);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	rxcount++;
	if (rxcount < RxSIZE)
	{
		if (rxcount == RxSIZE-1)
		{
			HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_LAST_FRAME);
		}
		else
		{
			HAL_I2C_Slave_Seq_Receive_IT(hi2c, RxData+rxcount, 1, I2C_NEXT_FRAME);
		}
	}

	if (rxcount == RxSIZE)
	{
		process_data();
	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	uint32_t errorcode = HAL_I2C_GetError(hi2c);

	if (errorcode == 4)  // AF error
	{
		if (txcount == 0)  // error is while slave is receiving
		{
			bytesRrecvd = rxcount-1;  // the first byte is the register address
			rxcount = 0;  // Reset the rxcount for the next operation
			process_data();
		}
		else // error while slave is transmitting
		{
			bytesTransd = txcount-1;  // the txcount is 1 higher than the actual data transmitted
			txcount = 0;  // Reset the txcount for the next operation
		}
	}

	/* BERR Error commonly occurs during the Direction switch
	 * Here we the software reset bit is set by the HAL error handler
	 * Before resetting this bit, we make sure the I2C lines are released and the bus is free
	 * I am simply reinitializing the I2C to do so
	 */
	else if (errorcode == 1)  // BERR Error
	{
		HAL_I2C_DeInit(hi2c);
		HAL_I2C_Init(hi2c);
		memset(RxData,'\0',RxSIZE);  // reset the Rx buffer
		rxcount =0;  // reset the count
	}

	HAL_I2C_EnableListen_IT(hi2c);
}
