################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/utils_kernel/utils_kernel.c 

OBJS += \
./src/utils_kernel/utils_kernel.o 

C_DEPS += \
./src/utils_kernel/utils_kernel.d 


# Each subdirectory must supply rules for building sources it contributes
src/utils_kernel/%.o: ../src/utils_kernel/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


