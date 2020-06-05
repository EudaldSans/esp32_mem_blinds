idf_component_register(
	SRCS "BLINDS_Device.c" "main.c" "factory_testing.c" "factory_modules/test_button.c" "factory_modules/test_relay.c" "factory_modules/test_meter.c" "factory_modules/test_leds.c" "factory_modules/test_sync.c"
	INCLUDE_DIRS "." "factory_modules"
	LDFRAGMENTS "iram.lf"
)
