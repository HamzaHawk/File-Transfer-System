################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/My\ UDP\ Server.cpp 

OBJS += \
./src/My\ UDP\ Server.o 

CPP_DEPS += \
./src/My\ UDP\ Server.d 


# Each subdirectory must supply rules for building sources it contributes
src/My\ UDP\ Server.o: ../src/My\ UDP\ Server.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/My UDP Server.d" -MT"src/My\ UDP\ Server.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


