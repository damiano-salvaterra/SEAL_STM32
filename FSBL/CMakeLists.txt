cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Compiler options
set(STM32_MCU_FLAGS  "-mcpu=cortex-m55 -mfpu=fpv5-d16 -mfloat-abi=hard -mcmse " )

#Linker options
set(STM32_LINKER_SCRIPT STM32N657XX_AXISRAM2_fsbl.ld)
set(STM32_LINKER_OPTION  )

# Set the project name
set(CMAKE_PROJECT_NAME SEAL_FSBL)
project(${CMAKE_PROJECT_NAME})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Include mx-generated
include("mx-generated.cmake")


# In order to use CMake for cross-compiling
set(CMAKE_CXX_COMPILER_FORCED "true")

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    "Core/Inc/BNO085"
    "sh2/Inc"
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    "sh2/Src/sh2.c"
    "sh2/Src/shtp.c"
    "sh2/Src/sh2_SensorValue.c"
    "sh2/Src/sh2_util.c"
    "sh2/Src/euler.c"

    "Core/Src/callbacks.c"

    "Core/Src/BNO085/rvc_hal.c"
    "Core/Src/BNO085/rvc.c"
    "Core/Src/BNO085/bno08x_service.c"
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined libraries
)

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -u _printf_float # print float
)
# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Validate that STM32CubeMX code is compatible with C standard
if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()


set(ELF_FILE "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.elf")
set(HEX_FILE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex")
set(BIN_FILE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin")

set(TRUSTED_BIN_FILE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-trusted.bin")
set(TRUSTED_ELF_FILE "${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-trusted.elf")

# Command to generate .bin file
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
-O binary ${ELF_FILE} ${BIN_FILE}
    COMMENT "Generating .bin file"
)

# Command to generate .hex file
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
-O ihex ${ELF_FILE} ${HEX_FILE}
    COMMENT "Generating .hex file"
)


# Add a post-build step to sign the FSBL binary
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo y > input.txt
    COMMAND STM32_SigningTool_CLI -bin "${BIN_FILE}" -nk -of 0x80000000 -t fsbl -o "${TRUSTED_BIN_FILE}" -hv 2.3 -align -dump "${TRUSTED_BIN_FILE}" < input.txt
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Signing FSBL binary: ${BIN_FILE} -> ${TRUSTED_BIN_FILE}"
)

# Convert the signed FSBL .bin back to .elf
add_custom_command(
    TARGET ${CMAKE_PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -I binary -O elf32-littlearm -B arm --change-addresses=0x70000000 ${TRUSTED_BIN_FILE} ${TRUSTED_ELF_FILE}
    COMMENT "Converting signed FSBL .bin back to .elf in address [0x70000000]"
)



