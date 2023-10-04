# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})


project(reg_rw_cm7)

set(MCUX_BUILD_TYPES sdram_txt_debug sdram_txt_release flexspi_nor_sdram_debug flexspi_nor_sdram_release debug release sdram_debug sdram_release flexspi_nor_debug flexspi_nor_release)

set(MCUX_SDK_PROJECT_NAME reg_rw_cm7.elf)

if (NOT DEFINED SdkRootDirPath)
    if ((DEFINED ENV{MCUX_SDK_PATH}) AND (NOT ("$ENV{MCUX_SDK_PATH}" STREQUAL "")))
        SET(SdkRootDirPath $ENV{MCUX_SDK_PATH})
    else()
        SET(SdkRootDirPath ${ProjDirPath}/../../../../../..)
    endif()
endif()

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

add_executable(${MCUX_SDK_PROJECT_NAME}
"${ProjDirPath}/../phytec_pcm073_connect_cm4_cm7side.jlinkscript"
"${ProjDirPath}/../reg_rw.c"
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
)

target_include_directories(${MCUX_SDK_PROJECT_NAME} PUBLIC
    ${ProjDirPath}/..
)

set(CMAKE_MODULE_PATH
    ${SdkRootDirPath}/core/utilities
    ${SdkRootDirPath}/core/utilities/misc_utilities
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers
    ${SdkRootDirPath}/core/devices/MIMXRT1176
    ${SdkRootDirPath}/core/components/uart
    ${SdkRootDirPath}/core/components/lists
    ${SdkRootDirPath}/core/devices/MIMXRT1176/xip
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xip
    ${SdkRootDirPath}/core/devices/MIMXRT1176/drivers/cm7
    ${SdkRootDirPath}/core/boards/evkmimxrt1170/xmcd
    ${SdkRootDirPath}/core/CMSIS/Core/Include
    ${SdkRootDirPath}/core/drivers/common/
    ${SdkRootDirPath}/core/drivers/igpio/
    ${SdkRootDirPath}/core/drivers/lpuart/
    ${SdkRootDirPath}/core/utilities/assert/
)

# include modules
include(utility_debug_console_lite)

include(utility_assert_lite)

include(driver_clock)

include(driver_common)

include(device_CMSIS)

include(component_lpuart_adapter)

include(component_lists)

include(driver_lpuart)

include(device_startup_MIMXRT1176_cm7)

include(driver_iomuxc)

include(driver_igpio)

include(driver_xip_device)

include(driver_xip_board_evkmimxrt1170)

include(driver_pmu_1)

include(driver_dcdc_soc)

include(driver_cache_armv7_m7)

include(driver_xmcd_evkmimxrt1170)

include(driver_anatop_ai)

include(CMSIS_Include_core_cm)

include(utilities_misc_utilities_MIMXRT1176_cm7)

include(device_system_MIMXRT1176_cm7)


IF(NOT DEFINED TARGET_LINK_SYSTEM_LIBRARIES)
    SET(TARGET_LINK_SYSTEM_LIBRARIES "-lm -lc -lgcc -lnosys")
ENDIF()

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE ${TARGET_LINK_SYSTEM_LIBRARIES})

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)


