################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (12.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32h7a3ritx.s 

S_DEPS += \
./Core/Startup/startup_stm32h7a3ritx.d 

OBJS += \
./Core/Startup/startup_stm32h7a3ritx.o 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m7 -c -I"D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/03_Common" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32h7a3ritx.d ./Core/Startup/startup_stm32h7a3ritx.o

.PHONY: clean-Core-2f-Startup

