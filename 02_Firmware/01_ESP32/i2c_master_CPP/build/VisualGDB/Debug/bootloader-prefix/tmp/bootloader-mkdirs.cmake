# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/SysGCC/esp32/esp-idf/v5.2/components/bootloader/subproject"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/tmp"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/src/bootloader-stamp"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/src"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/i2c_master_CPP/build/VisualGDB/Debug/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
