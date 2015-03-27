################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/My\ UDP\ Client.cpp 

OBJS += \
./src/My\ UDP\ Client.o 

CPP_DEPS += \
./src/My\ UDP\ Client.d 


# Each subdirectory must supply rules for building sources it contributes
src/My\ UDP\ Client.o: ../src/My\ UDP\ Client.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"src/My UDP Client.d" -MT"src/My\ UDP\ Client.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


