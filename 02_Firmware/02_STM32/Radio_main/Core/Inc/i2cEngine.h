/*
 * i2cEngine.h
 *
 *  Created on: Jul 21, 2024
 *      Author: dbank
 */

#ifndef INC_I2CENGINE_H_
#define INC_I2CENGINE_H_

#include "i2c.h"
#include "printfRedirect.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "i2c_slave_master_queueClass.h"

class i2cMaster {
public:
	i2cMaster(I2C_HandleTypeDef *hi2c1);
	virtual ~i2cMaster();
	HAL_StatusTypeDef ping(uint16_t DevAddress_7bit);
	i2cReceiveQueue4DynamicData* pReceiveQueueObject;

	//void setEsp32InterruptCounterOverflow(void);
	//BaseType_t getEsp32InterruptCounterOverflow(void);
protected:
	BaseType_t i2cSemaphoreTake(void);
	BaseType_t i2cSemaphoreGive(void);

private:
	I2C_HandleTypeDef *p_hi2c1;						//adres interfejsu i2c po którym odbywa się komunikacja
	const char *TAG = "I2C master log:";
	SemaphoreHandle_t handle_i2cBinarySemaphore;	//semafor blokujący zasoby do interfejsu i2c SMT'a podczas transmisji/ odbioru danych przez inny wątek
	//BaseType_t esp32InterruptCounterOverflow;		//esp32 interrupt counter overflow indicator

};

#endif /* INC_I2CENGINE_H_ */
