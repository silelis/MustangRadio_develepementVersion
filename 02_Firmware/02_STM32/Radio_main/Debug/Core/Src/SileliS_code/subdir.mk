################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Core/Src/SileliS_code/StepperOptoPowerOFF.cpp \
D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol/comunicationProtocol.cpp \
D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures/comunicationStructures.cpp \
D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum/comunication_calculate_checksum.cpp \
../Core/Src/SileliS_code/esp32i2cComunicationDriver.cpp \
../Core/Src/SileliS_code/i2cEngine.cpp \
D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass/i2c_slave_master_queueClass.cpp \
../Core/Src/SileliS_code/keyboardToFunction.cpp \
../Core/Src/SileliS_code/ledsController.cpp \
../Core/Src/SileliS_code/menuItem.cpp \
../Core/Src/SileliS_code/myList.cpp \
../Core/Src/SileliS_code/myPrintfTask.cpp \
../Core/Src/SileliS_code/radioMenu.cpp \
../Core/Src/SileliS_code/tasksFunctions.cpp \
../Core/Src/SileliS_code/tasksFunctionsCppWrapper.cpp 

C_SRCS += \
../Core/Src/SileliS_code/printfRedirect.c 

C_DEPS += \
./Core/Src/SileliS_code/printfRedirect.d 

OBJS += \
./Core/Src/SileliS_code/StepperOptoPowerOFF.o \
./Core/Src/SileliS_code/comunicationProtocol.o \
./Core/Src/SileliS_code/comunicationStructures.o \
./Core/Src/SileliS_code/comunication_calculate_checksum.o \
./Core/Src/SileliS_code/esp32i2cComunicationDriver.o \
./Core/Src/SileliS_code/i2cEngine.o \
./Core/Src/SileliS_code/i2c_slave_master_queueClass.o \
./Core/Src/SileliS_code/keyboardToFunction.o \
./Core/Src/SileliS_code/ledsController.o \
./Core/Src/SileliS_code/menuItem.o \
./Core/Src/SileliS_code/myList.o \
./Core/Src/SileliS_code/myPrintfTask.o \
./Core/Src/SileliS_code/printfRedirect.o \
./Core/Src/SileliS_code/radioMenu.o \
./Core/Src/SileliS_code/tasksFunctions.o \
./Core/Src/SileliS_code/tasksFunctionsCppWrapper.o 

CPP_DEPS += \
./Core/Src/SileliS_code/StepperOptoPowerOFF.d \
./Core/Src/SileliS_code/comunicationProtocol.d \
./Core/Src/SileliS_code/comunicationStructures.d \
./Core/Src/SileliS_code/comunication_calculate_checksum.d \
./Core/Src/SileliS_code/esp32i2cComunicationDriver.d \
./Core/Src/SileliS_code/i2cEngine.d \
./Core/Src/SileliS_code/i2c_slave_master_queueClass.d \
./Core/Src/SileliS_code/keyboardToFunction.d \
./Core/Src/SileliS_code/ledsController.d \
./Core/Src/SileliS_code/menuItem.d \
./Core/Src/SileliS_code/myList.d \
./Core/Src/SileliS_code/myPrintfTask.d \
./Core/Src/SileliS_code/radioMenu.d \
./Core/Src/SileliS_code/tasksFunctions.d \
./Core/Src/SileliS_code/tasksFunctionsCppWrapper.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/SileliS_code/%.o Core/Src/SileliS_code/%.su Core/Src/SileliS_code/%.cyclo: ../Core/Src/SileliS_code/%.cpp Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/SileliS_code/comunicationProtocol.o: D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol/comunicationProtocol.cpp Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/SileliS_code/comunicationStructures.o: D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures/comunicationStructures.cpp Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/SileliS_code/comunication_calculate_checksum.o: D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum/comunication_calculate_checksum.cpp Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/SileliS_code/i2c_slave_master_queueClass.o: D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass/i2c_slave_master_queueClass.cpp Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m7 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"
Core/Src/SileliS_code/%.o Core/Src/SileliS_code/%.su Core/Src/SileliS_code/%.cyclo: ../Core/Src/SileliS_code/%.c Core/Src/SileliS_code/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H7A3xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunication_calculate_checksum" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationProtocol" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/comunicationStructures" -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common/i2c_slave_master_queueClass" -ID:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_and_bluetooth/main/common -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-SileliS_code

clean-Core-2f-Src-2f-SileliS_code:
	-$(RM) ./Core/Src/SileliS_code/StepperOptoPowerOFF.cyclo ./Core/Src/SileliS_code/StepperOptoPowerOFF.d ./Core/Src/SileliS_code/StepperOptoPowerOFF.o ./Core/Src/SileliS_code/StepperOptoPowerOFF.su ./Core/Src/SileliS_code/comunicationProtocol.cyclo ./Core/Src/SileliS_code/comunicationProtocol.d ./Core/Src/SileliS_code/comunicationProtocol.o ./Core/Src/SileliS_code/comunicationProtocol.su ./Core/Src/SileliS_code/comunicationStructures.cyclo ./Core/Src/SileliS_code/comunicationStructures.d ./Core/Src/SileliS_code/comunicationStructures.o ./Core/Src/SileliS_code/comunicationStructures.su ./Core/Src/SileliS_code/comunication_calculate_checksum.cyclo ./Core/Src/SileliS_code/comunication_calculate_checksum.d ./Core/Src/SileliS_code/comunication_calculate_checksum.o ./Core/Src/SileliS_code/comunication_calculate_checksum.su ./Core/Src/SileliS_code/esp32i2cComunicationDriver.cyclo ./Core/Src/SileliS_code/esp32i2cComunicationDriver.d ./Core/Src/SileliS_code/esp32i2cComunicationDriver.o ./Core/Src/SileliS_code/esp32i2cComunicationDriver.su ./Core/Src/SileliS_code/i2cEngine.cyclo ./Core/Src/SileliS_code/i2cEngine.d ./Core/Src/SileliS_code/i2cEngine.o ./Core/Src/SileliS_code/i2cEngine.su ./Core/Src/SileliS_code/i2c_slave_master_queueClass.cyclo ./Core/Src/SileliS_code/i2c_slave_master_queueClass.d ./Core/Src/SileliS_code/i2c_slave_master_queueClass.o ./Core/Src/SileliS_code/i2c_slave_master_queueClass.su ./Core/Src/SileliS_code/keyboardToFunction.cyclo ./Core/Src/SileliS_code/keyboardToFunction.d ./Core/Src/SileliS_code/keyboardToFunction.o ./Core/Src/SileliS_code/keyboardToFunction.su ./Core/Src/SileliS_code/ledsController.cyclo ./Core/Src/SileliS_code/ledsController.d ./Core/Src/SileliS_code/ledsController.o ./Core/Src/SileliS_code/ledsController.su ./Core/Src/SileliS_code/menuItem.cyclo ./Core/Src/SileliS_code/menuItem.d ./Core/Src/SileliS_code/menuItem.o ./Core/Src/SileliS_code/menuItem.su ./Core/Src/SileliS_code/myList.cyclo ./Core/Src/SileliS_code/myList.d ./Core/Src/SileliS_code/myList.o ./Core/Src/SileliS_code/myList.su ./Core/Src/SileliS_code/myPrintfTask.cyclo ./Core/Src/SileliS_code/myPrintfTask.d ./Core/Src/SileliS_code/myPrintfTask.o ./Core/Src/SileliS_code/myPrintfTask.su ./Core/Src/SileliS_code/printfRedirect.cyclo ./Core/Src/SileliS_code/printfRedirect.d ./Core/Src/SileliS_code/printfRedirect.o ./Core/Src/SileliS_code/printfRedirect.su ./Core/Src/SileliS_code/radioMenu.cyclo ./Core/Src/SileliS_code/radioMenu.d ./Core/Src/SileliS_code/radioMenu.o ./Core/Src/SileliS_code/radioMenu.su ./Core/Src/SileliS_code/tasksFunctions.cyclo ./Core/Src/SileliS_code/tasksFunctions.d ./Core/Src/SileliS_code/tasksFunctions.o ./Core/Src/SileliS_code/tasksFunctions.su ./Core/Src/SileliS_code/tasksFunctionsCppWrapper.cyclo ./Core/Src/SileliS_code/tasksFunctionsCppWrapper.d ./Core/Src/SileliS_code/tasksFunctionsCppWrapper.o ./Core/Src/SileliS_code/tasksFunctionsCppWrapper.su

.PHONY: clean-Core-2f-Src-2f-SileliS_code

