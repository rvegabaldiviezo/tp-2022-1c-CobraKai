################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cliente/utils.c 

OBJS += \
./src/cliente/utils.o 

C_DEPS += \
./src/cliente/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/cliente/%.o: ../src/cliente/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


