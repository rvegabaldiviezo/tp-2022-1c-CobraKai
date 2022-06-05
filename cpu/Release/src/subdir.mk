################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/client_utils.c \
../src/cpu.c \
../src/serv_utils.c 

OBJS += \
./src/client_utils.o \
./src/cpu.o \
./src/serv_utils.o 

C_DEPS += \
./src/client_utils.d \
./src/cpu.d \
./src/serv_utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


