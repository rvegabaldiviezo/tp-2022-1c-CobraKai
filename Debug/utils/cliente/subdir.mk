################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/cliente/utils_cliente.c 

OBJS += \
./utils/cliente/utils_cliente.o 

C_DEPS += \
./utils/cliente/utils_cliente.d 


# Each subdirectory must supply rules for building sources it contributes
utils/cliente/%.o: ../utils/cliente/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


