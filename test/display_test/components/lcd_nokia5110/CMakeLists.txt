idf_component_register(SRCS "src/lcd_nokia5110.c" "src/font_5x7.c"
		INCLUDE_DIRS "include" "private_include"
		REQUIRES driver esp_driver_spi)
