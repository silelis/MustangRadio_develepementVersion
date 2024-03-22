# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/toolchains/Espressif/frameworks/esp-idf-v4.4.4/components/bootloader/subproject"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix/tmp"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix/src/bootloader-stamp"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix/src"
  "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/!!!__GitHUB_repositories/MustangRadio_develepementVersion/02_Firmware/01_ESP32/HMI_bt_a2dp_VScode_idf_4.4.4/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
