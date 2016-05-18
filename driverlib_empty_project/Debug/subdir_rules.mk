################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/inc" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project/Debug" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/CMSIS" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx/" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

printf.obj: ../printf.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/inc" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project/Debug" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/CMSIS" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx/" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="printf.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

printfOverride.obj: ../printfOverride.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/inc" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project/Debug" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/CMSIS" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx/" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="printfOverride.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

startup_msp432p401r_ccs.obj: ../startup_msp432p401r_ccs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/inc" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project/Debug" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/CMSIS" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx/" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="startup_msp432p401r_ccs.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

system_msp432p401r.obj: ../system_msp432p401r.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP432 Compiler'
	"Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 --abi=eabi -me --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/inc" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project/Debug" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/tools/compiler/ti-cgt-arm_5.2.5/include/" --include_path="Y:/Cody/Documents/TI/ccsv6/ccs_base/arm/include/CMSIS" --include_path="Y:/Cody/Documents/TI/workspace_v6_1/driverlib_empty_project" --include_path="Y:/Cody/Documents/TI/msp430/MSPWare_3_30_00_18/driverlib/driverlib/MSP432P4xx/" --advice:power=all -g --gcc --define=__MSP432P401R__ --define=TARGET_IS_MSP432P4XX --define=ccs --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="system_msp432p401r.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


