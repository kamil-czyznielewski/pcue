del button_*.h

bmptolcd button_menu2.bmp
ren image_bw_32_32.h button_menu_32_32.h

bmptolcd button_mute2.bmp
ren image_bw_32_32.h button_mute_32_32.h

bmptolcd button_low2.bmp
ren image_bw_32_32.h button_low_32_32.h

bmptolcd button_high2.bmp
ren image_bw_32_32.h button_high_32_32.h

copy button*.h ..\OS\BMP
