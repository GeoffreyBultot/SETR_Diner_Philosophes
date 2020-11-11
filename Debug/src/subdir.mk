################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/EvalDInerPhilosophes.cpp \
../src/SolutionEtudiant.cpp 

OBJS += \
./src/EvalDInerPhilosophes.o \
./src/SolutionEtudiant.o 

CPP_DEPS += \
./src/EvalDInerPhilosophes.d \
./src/SolutionEtudiant.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -pthread -I"/home/bultot/eclipse-workspace/EvalDInerPhilosophes/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


