idf_component_register(SRCS "fang_song.c" "lcd.c"
                    INCLUDE_DIRS "."
                    REQUIRES lvgl)

lvgl_port_create_c_image("images/esp_logo.png" "images/" "ARGB8888" "NONE")
lvgl_port_add_images(${COMPONENT_LIB} "images/")