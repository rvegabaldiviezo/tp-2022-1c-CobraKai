################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils_memoria/utils_memoria.c 

OBJS += \
./src/utils_memoria/utils_memoria.o 

C_DEPS += \
./src/utils_memoria/utils_memoria.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils_memoria/%.o: ../src/utils_memoria/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


