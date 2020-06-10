idf_component_register(
	SRCS "BLINDS_Device.c" "BLINDS_Button.c" "BLINDS_Feedback.c" "BLINDS_Load.c" "main.c"
    INCLUDE_DIRS "."
	LDFRAGMENTS "iram.lf"
)
