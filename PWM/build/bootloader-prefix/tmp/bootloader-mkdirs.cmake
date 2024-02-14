# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/bdkirkendall/esp/esp-idf/components/bootloader/subproject"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/tmp"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/src"
  "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/bdkirkendall/Documents/ECE477/477Grp2/PWM/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
