################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/cli_message.c 

OBJS += \
./src/cli_message.o 

C_DEPS += \
./src/cli_message.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/media/ssd_data/Dev/projects/Project/LibMessage/libmessage/inc" -I/home/bruno/Dev/projects/Project/common/inc -I"/media/ssd_data/Dev/projects/Project/libapisyslog/apisyslog" -O0 -g3 -Wall -Wextra -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


