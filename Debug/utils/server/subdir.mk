################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../utils/server/utils_server.c 

OBJS += \
./utils/server/utils_server.o 

C_DEPS += \
./utils/server/utils_server.d 


# Each subdirectory must supply rules for building sources it contributes
utils/server/%.o: ../utils/server/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


