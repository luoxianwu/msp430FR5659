################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
hal/%.obj: ../hal/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="C:/ti/ccs1281/ccs/ccs_base/msp430/include" --include_path="C:/Users/x-luo/28-7/28-9/28v-msp432/driverlib/MSP430FR5xx_6xx" --include_path="C:/Users/x-luo/28-7/28-9/28v-msp432/hal" --include_path="C:/Users/x-luo/28-7/28-9/28v-msp432" --include_path="C:/ti/ccs1281/ccs/tools/compiler/ti-cgt-msp430_21.6.1.LTS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5969__ --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="hal/$(basename $(<F)).d_raw" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

