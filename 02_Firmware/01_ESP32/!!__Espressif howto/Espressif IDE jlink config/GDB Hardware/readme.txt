1) Uruchom poenOCD: openocd.exe -f interface/jlink.cfg -c "transport select jtag" -f target/esp32.cfg -c "adapter speed 1000" -c "jlink usb 0" -c "reset_config none" -c "init"
2) Uruchom GDB hardware debugging w Espressif IDE






C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20240318\openocd-esp32\bin\openocd.exe -s "C:\Espressif\tools\openocd-esp32\v0.12.0-esp32-20240318\openocd-esp32\scripts" -f interface/jlink.cfg -f target/esp32.cfg -c "adapter speed 1000" -c "init" -c "reset halt" 





ta komenda działa
openocd.exe -f interface/jlink.cfg -f target/esp32.cfg -c "adapter speed 5000"   -c "transport select jtag" -c "jlink usb 0" -c "init" -c "reset halt"


Openocd to ten z 5.5.1
GNU gdb (esp-gdb) 16.2_20250324
xtensa-esp32-elf-gdb build\HMI_and_bluetooth.elf
target extended-remote :3333