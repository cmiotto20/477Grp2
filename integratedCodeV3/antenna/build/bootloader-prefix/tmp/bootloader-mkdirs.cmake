# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/mgher/esp/esp-idf/components/bootloader/subproject"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/tmp"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/src"
  "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/mgher/Documents/GitHub/477Grp2/integratedCodeV3/antenna/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
