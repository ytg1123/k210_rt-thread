
# DO NOT MODIFY THIS FILE, IT WILL BE OVERRIDE!!!

# [section] Head
##### include(reset) #####
### reset basic var
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# set(CMAKE_SYSTEM "Generic")
set(CMAKE_SYSTEM_NAME "Generic")

# set(CMAKE_HOST_SYSTEM "Generic")
# set(CMAKE_HOST_SYSTEM_NAME "Generic")
#
set(CMAKE_CROSSCOMPILING TRUE)
# set(CMAKE_C_COMPILER_FORCED TRUE)
# set(CMAKE_CXX_COMPILER_FORCED TRUE)
# set(CMAKE_ASM_COMPILER_FORCED TRUE)
#

set(SOURCE_FILES "")
set(LDFLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_MODULE_LINKER_FLAGS "")


cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_NAME "lcd_image_LVGL")
# [/section] Head

# [section] Init
##### include(macros) #####
macro(header_directories parent)
    file(GLOB_RECURSE newList ${parent}/*.h)
    set(dir_list "")
    foreach (file_path ${newList})
        get_filename_component(dir_path ${file_path} DIRECTORY)
        set(dir_list ${dir_list} ${dir_path})
    endforeach ()
    list(REMOVE_DUPLICATES dir_list)

    include_directories(${dir_list})
endmacro()

string(SUBSTRING ${CMAKE_BINARY_DIR} 0 1 CMAKE_BINARY_DIR_L)
string(TOUPPER ${CMAKE_BINARY_DIR_L} CMAKE_BINARY_DIR_L)
string(SUBSTRING ${CMAKE_BINARY_DIR} 1 -1 CMAKE_BINARY_DIR_R)
set(CMAKE_BINARY_DIRX "${CMAKE_BINARY_DIR_L}${CMAKE_BINARY_DIR_R}")

macro(add_source_file)
    list(APPEND SOURCE_FILES ${ARGN})
    set("SOURCE_FILES_${PROJECT_NAME}" "${SOURCE_FILES}" CACHE STRING "Dump files to compile for '${PROJECT_NAME}'. Not used by program.")
endmacro()

macro(add_source_files)
    file(GLOB_RECURSE newlist ${ARGN})

    foreach (filepath ${newlist})
        string(FIND ${filepath} ${CMAKE_BINARY_DIRX} found1)
        string(FIND ${filepath} "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries" found2)
        if ((NOT found1 EQUAL 0) AND (NOT found2 EQUAL 0))
            list(APPEND SOURCE_FILES ${filepath})
        endif ()
    endforeach ()
    set("SOURCE_FILES_${PROJECT_NAME}" "${SOURCE_FILES}" CACHE STRING "Dump files to compile for '${PROJECT_NAME}'. Not used by program.")
endmacro()

function(JOIN VALUES GLUE OUTPUT)
    string(REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
    string(REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
    set(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

function(removeDuplicateSubstring stringIn stringOut)
    separate_arguments(stringIn)
    list(REMOVE_DUPLICATES stringIn)
    string(REPLACE ";" " " stringIn "${stringIn}")
    set(${stringOut} "${stringIn}" PARENT_SCOPE)
endfunction()

macro(add_compile_flags WHERE)
    JOIN("${ARGN}" " " STRING_ARGS)
    if (${WHERE} STREQUAL C)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL CXX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL LD)
        set(LDFLAGS "${LDFLAGS} ${STRING_ARGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${STRING_ARGS}")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${STRING_ARGS}")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${STRING_ARGS}")

    elseif (${WHERE} STREQUAL BOTH)
        add_compile_flags(C ${ARGN})
        add_compile_flags(CXX ${ARGN})

    elseif (${WHERE} STREQUAL ALL)
        add_compile_flags(C ${ARGN})
        add_compile_flags(CXX ${ARGN})
        add_compile_flags(LD ${ARGN})

    else ()
        message(FATAL_ERROR "add_compile_flags - only support: C, CXX, BOTH, LD, ALL")
    endif ()
endmacro()


##### include(coreFlags) #####
#add_compile_flags(LD -nostartfiles -Wl,--gc-sections)
add_compile_flags(ALL -mcmodel=medany)


##### include(ideSettings) #####
# set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
else()
    if((NOT CMAKE_BUILD_TYPE STREQUAL "Debug") AND (NOT CMAKE_BUILD_TYPE STREQUAL "Release"))
        message(FATAL_ERROR "CMAKE_BUILD_TYPE must either be Debug or Release instead of ${CMAKE_BUILD_TYPE}")
    endif()
endif()
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type options are: Debug Release RelWithDebInfo MinSizeRel")

## - Debug & Release
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DDEBUG=1)
endif()

## definitions in macros
add_definitions(-DCONFIG_LOG_LEVEL=LOG_INFO -DCONFIG_LOG_ENABLE -D__riscv64)
if(DEFINED KENDRYTE_IDE)
	set(CMAKE_COLOR_MAKEFILE FALSE)
else()
	add_definitions(-DCONFIG_LOG_COLORS)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE CACHE INTERNAL "must set to true, never edit or IDEs will not work")


##### include(toolchain) #####
if (WIN32)
    set(BINARY_EXTENSION ".exe")
else ()
    set(BINARY_EXTENSION "")
endif ()

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_LINKER "ld")
set(CMAKE_AR "ar")
set(CMAKE_OBJCOPY "objcopy")
set(CMAKE_SIZE "size")
set(CMAKE_OBJDUMP "objdump")
set(CMAKE_RANLIB "ranlib")
set(CMAKE_NM "nm")
set(CMAKE_STRIP "strip")
set(CMAKE_C_COMPILER_AR "gcc-ar")
set(CMAKE_C_COMPILER_RANLIB "gcc-ranlib")
set(CMAKE_CXX_COMPILER_AR "gcc-ar")
set(CMAKE_CXX_COMPILER_RANLIB "gcc-ranlib")

set(CMAKE_TOOLCHAIN_PREFIX "riscv64-unknown-elf-" CACHE INTERNAL "this const is defined by toolchain")
set(CMAKE_EXECUTABLE_FORMAT "ELF" CACHE INTERNAL "this const is defined by toolchain")

foreach(CMD CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_LINKER CMAKE_AR CMAKE_OBJCOPY CMAKE_SIZE CMAKE_OBJDUMP CMAKE_RANLIB CMAKE_NM CMAKE_STRIP CMAKE_C_COMPILER_AR CMAKE_C_COMPILER_RANLIB CMAKE_CXX_COMPILER_AR CMAKE_CXX_COMPILER_RANLIB)
	set(${CMD} "${CMAKE_TOOLCHAIN_PREFIX}${${CMD}}${BINARY_EXTENSION}" CACHE FILEPATH "the path of ${CMD}" FORCE)
endforeach()

execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTBEGIN_OBJ)
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTEND_OBJ)
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTI_OBJ)
execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE CRTN_OBJ)

set(CMAKE_C_LINK_EXECUTABLE
       "<CMAKE_C_COMPILER> -o <TARGET> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> \"${CRTI_OBJ}\" \"${CRTBEGIN_OBJ}\" <OBJECTS> \"${CRTEND_OBJ}\" \"${CRTN_OBJ}\" <LINK_LIBRARIES>"
       CACHE STRING "CMAKE_C_LINK_EXECUTABLE"
       )

set(CMAKE_CXX_LINK_EXECUTABLE
       "<CMAKE_CXX_COMPILER> -o <TARGET> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> \"${CRTI_OBJ}\" \"${CRTBEGIN_OBJ}\" <OBJECTS> \"${CRTEND_OBJ}\" \"${CRTN_OBJ}\" <LINK_LIBRARIES>"
       CACHE STRING "CMAKE_CXX_LINK_EXECUTABLE"
       )


# debug mode disabled
# [/section] Init

# [section] Project
##### include(fix9985) #####
### http://www.cmake.org/Bug/view.php?id=9985
string(REPLACE "-rdynamic" "" _C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")
string(REPLACE "-rdynamic" "" _CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${_C_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${_CXX_FLAGS}")


message("======== PROJECT:${PROJECT_NAME} ========")
project(${PROJECT_NAME})

## [section] Header
### from project local
include_directories(
  "${CMAKE_CURRENT_LIST_DIR}/src/lvgl"
)### from ide
include_directories("${CMAKE_CURRENT_LIST_DIR}/config")
## from project public
include_directories(
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/lib/bsp/include"
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/lib/drivers/include"
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/lib/utils/include"
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/third_party/xtl/include"
  "${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/lib/nncase/include"
)
## [/section] Header
## [section] Source
## add source from config json (123 files matched)
add_source_file(src/lvgl/examples/porting/lv_port_disp_template.c)
add_source_file(src/lvgl/examples/porting/lv_port_indev_template.c)
add_source_file(src/lvgl/src/lv_core/lv_disp.c)
add_source_file(src/lvgl/src/lv_core/lv_group.c)
add_source_file(src/lvgl/src/lv_core/lv_indev.c)
add_source_file(src/lvgl/src/lv_core/lv_obj.c)
add_source_file(src/lvgl/src/lv_core/lv_refr.c)
add_source_file(src/lvgl/src/lv_core/lv_style.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_arc.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_blend.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_img.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_label.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_line.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_mask.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_rect.c)
add_source_file(src/lvgl/src/lv_draw/lv_draw_triangle.c)
add_source_file(src/lvgl/src/lv_draw/lv_img_buf.c)
add_source_file(src/lvgl/src/lv_draw/lv_img_cache.c)
add_source_file(src/lvgl/src/lv_draw/lv_img_decoder.c)
add_source_file(src/lvgl/src/lv_font/lv_font.c)
add_source_file(src/lvgl/src/lv_font/lv_font_dejavu_16_persian_hebrew.c)
add_source_file(src/lvgl/src/lv_font/lv_font_fmt_txt.c)
add_source_file(src/lvgl/src/lv_font/lv_font_loader.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_10.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_12.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_12_subpx.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_14.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_16.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_20.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_18.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_22.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_24.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_26.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_28.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_28_compressed.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_30.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_32.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_36.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_34.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_38.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_40.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_42.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_44.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_46.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_48.c)
add_source_file(src/lvgl/src/lv_font/lv_font_montserrat_8.c)
add_source_file(src/lvgl/src/lv_font/lv_font_simsun_16_cjk.c)
add_source_file(src/lvgl/src/lv_font/lv_font_ipa_gothic_16.c)
add_source_file(src/lvgl/src/lv_font/lv_font_unscii_16.c)
add_source_file(src/lvgl/src/lv_font/lv_font_unscii_8.c)
add_source_file(src/lvgl/src/lv_gpu/lv_gpu_nxp_pxp.c)
add_source_file(src/lvgl/src/lv_gpu/lv_gpu_nxp_pxp_osa.c)
add_source_file(src/lvgl/src/lv_gpu/lv_gpu_nxp_vglite.c)
add_source_file(src/lvgl/src/lv_gpu/lv_gpu_stm32_dma2d.c)
add_source_file(src/lvgl/src/lv_hal/lv_hal_disp.c)
add_source_file(src/lvgl/src/lv_hal/lv_hal_indev.c)
add_source_file(src/lvgl/src/lv_hal/lv_hal_tick.c)
add_source_file(src/lvgl/src/lv_misc/lv_anim.c)
add_source_file(src/lvgl/src/lv_misc/lv_area.c)
add_source_file(src/lvgl/src/lv_misc/lv_async.c)
add_source_file(src/lvgl/src/lv_misc/lv_bidi.c)
add_source_file(src/lvgl/src/lv_misc/lv_color.c)
add_source_file(src/lvgl/src/lv_misc/lv_debug.c)
add_source_file(src/lvgl/src/lv_misc/lv_fs.c)
add_source_file(src/lvgl/src/lv_misc/lv_gc.c)
add_source_file(src/lvgl/src/lv_misc/lv_ll.c)
add_source_file(src/lvgl/src/lv_misc/lv_log.c)
add_source_file(src/lvgl/src/lv_misc/lv_math.c)
add_source_file(src/lvgl/src/lv_misc/lv_mem.c)
add_source_file(src/lvgl/src/lv_misc/lv_printf.c)
add_source_file(src/lvgl/src/lv_misc/lv_task.c)
add_source_file(src/lvgl/src/lv_misc/lv_templ.c)
add_source_file(src/lvgl/src/lv_misc/lv_txt.c)
add_source_file(src/lvgl/src/lv_misc/lv_txt_ap.c)
add_source_file(src/lvgl/src/lv_misc/lv_utils.c)
add_source_file(src/lvgl/src/lv_themes/lv_theme_empty.c)
add_source_file(src/lvgl/src/lv_themes/lv_theme.c)
add_source_file(src/lvgl/src/lv_themes/lv_theme_material.c)
add_source_file(src/lvgl/src/lv_themes/lv_theme_mono.c)
add_source_file(src/lvgl/src/lv_themes/lv_theme_template.c)
add_source_file(src/lvgl/src/lv_widgets/lv_arc.c)
add_source_file(src/lvgl/src/lv_widgets/lv_bar.c)
add_source_file(src/lvgl/src/lv_widgets/lv_btn.c)
add_source_file(src/lvgl/src/lv_widgets/lv_btnmatrix.c)
add_source_file(src/lvgl/src/lv_widgets/lv_calendar.c)
add_source_file(src/lvgl/src/lv_widgets/lv_canvas.c)
add_source_file(src/lvgl/src/lv_widgets/lv_chart.c)
add_source_file(src/lvgl/src/lv_widgets/lv_checkbox.c)
add_source_file(src/lvgl/src/lv_widgets/lv_cont.c)
add_source_file(src/lvgl/src/lv_widgets/lv_cpicker.c)
add_source_file(src/lvgl/src/lv_widgets/lv_dropdown.c)
add_source_file(src/lvgl/src/lv_widgets/lv_gauge.c)
add_source_file(src/lvgl/src/lv_widgets/lv_img.c)
add_source_file(src/lvgl/src/lv_widgets/lv_imgbtn.c)
add_source_file(src/lvgl/src/lv_widgets/lv_keyboard.c)
add_source_file(src/lvgl/src/lv_widgets/lv_label.c)
add_source_file(src/lvgl/src/lv_widgets/lv_led.c)
add_source_file(src/lvgl/src/lv_widgets/lv_line.c)
add_source_file(src/lvgl/src/lv_widgets/lv_linemeter.c)
add_source_file(src/lvgl/src/lv_widgets/lv_list.c)
add_source_file(src/lvgl/src/lv_widgets/lv_msgbox.c)
add_source_file(src/lvgl/src/lv_widgets/lv_objmask.c)
add_source_file(src/lvgl/src/lv_widgets/lv_objx_templ.c)
add_source_file(src/lvgl/src/lv_widgets/lv_page.c)
add_source_file(src/lvgl/src/lv_widgets/lv_roller.c)
add_source_file(src/lvgl/src/lv_widgets/lv_slider.c)
add_source_file(src/lvgl/src/lv_widgets/lv_spinbox.c)
add_source_file(src/lvgl/src/lv_widgets/lv_spinner.c)
add_source_file(src/lvgl/src/lv_widgets/lv_switch.c)
add_source_file(src/lvgl/src/lv_widgets/lv_table.c)
add_source_file(src/lvgl/src/lv_widgets/lv_tabview.c)
add_source_file(src/lvgl/src/lv_widgets/lv_textarea.c)
add_source_file(src/lvgl/src/lv_widgets/lv_tileview.c)
add_source_file(src/lvgl/src/lv_widgets/lv_win.c)
add_source_file(src/board_config.h)
add_source_file(src/daodao.c)
add_source_file(src/font.h)
add_source_file(src/image.h)
add_source_file(src/lcd.h)
add_source_file(src/lcd.c)
add_source_file(src/lv_conf.h)
add_source_file(src/main.c)
add_source_file(src/nt35310.c)
add_source_file(src/nt35310.h)
## [/section] Source
# [/section] Project

# [section] Custom

# [/section] Custom
# [section] Target
## final create lcd_image_LVGL executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE "PROJECT_PATH=${CMAKE_CURRENT_LIST_DIR}/")
### [section] Custom2

### [/section] Custom2
# debug mode disabled
## no properties
## set definitions
add_compile_definitions(TCB_SPAN_NO_EXCEPTIONS)
add_compile_definitions(TCB_SPAN_NO_CONTRACT_CHECKING)
add_compile_definitions(NNCASE_TARGET=k210)
add_compile_definitions(KENDRYTE_SDK_TYPE=1)
add_compile_definitions(KENDRYTE_SDK_TYPE_STANDALONE=1)
add_compile_definitions(KENDRYTE_SDK_TYPE_FREERTOS=2)
add_compile_definitions(KENDRYTE_SDK_RELEASE_DATE=20190906195432)
# [/section] Target

# [section] Dependency
cmake_policy(SET CMP0079 NEW)
## root project will include all dependency
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}//kendryte_libraries/kendryte_standalone-sdk" "kendryte_standalone-sdk")
## add simple folder dependency
# [/section] Dependency


# [section] C/C++ compiler flags

##### flags from config json #####
message("config flags for ${PROJECT_NAME}")
target_link_options(${PROJECT_NAME} PUBLIC
	"-nostartfiles"
	"-static"
	"-Wl,--gc-sections"
	"-Wl,-static"
	"-Wl,-EL"
	"-Wl,--no-relax"
	"-T"
	"${CMAKE_CURRENT_LIST_DIR}/kendryte_libraries/kendryte_standalone-sdk/lds/kendryte.ld"
	"-nostartfiles"
	"-Wl,--gc-sections"
)
target_link_libraries(${PROJECT_NAME} PUBLIC -Wl,--start-group
	## -> lcd_image_LVGL: lcd_image_LVGL
	##	not link component
	"gcc"
	"m"
	"c"
	## -> kendryte/standalone-sdk: kendryte_standalone-sdk
	"-Wl,--whole-archive"
	"kendryte_standalone-sdk"
	"-Wl,--no-whole-archive"
-Wl,--end-group)

# [/section] C/C++ compiler flags

# [section] Finish
##### include(afterProject) #####
removeDuplicateSubstring("${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)
removeDuplicateSubstring("${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)


##### include(fix9985) #####
### http://www.cmake.org/Bug/view.php?id=9985
string(REPLACE "-rdynamic" "" _C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}")
string(REPLACE "-rdynamic" "" _CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${_C_FLAGS}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${_CXX_FLAGS}")


##### include(flash) #####
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary ${CMAKE_BINARY_DIR}/${PROJECT_NAME}${SUFFIX} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating .bin file ...")


# [/section] Finish

# [section] Dump Setting
##### include(dumpConfig) #####
set(CMAKE_SYSTEM_NAME "Generic")

message("")
message("Project: ${PROJECT_NAME}")
message("  LIST_FILE=${CMAKE_PARENT_LIST_FILE}")
message("  TOOLCHAIN=${TOOLCHAIN}")
message("  KENDRYTE_IDE=${KENDRYTE_IDE}")
message("  BUILDING_SDK=${BUILDING_SDK}")
message("  _CMAKE_TOOLCHAIN_PREFIX=${_CMAKE_TOOLCHAIN_PREFIX}")
message("  _CMAKE_TOOLCHAIN_LOCATION=${_CMAKE_TOOLCHAIN_LOCATION}")


message("")
message("  CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
message("  CMAKE_C_COMPILER=${CMAKE_C_COMPILER}")
message("  CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}")
message("  CMAKE_LINKER=${CMAKE_LINKER}")
message("  CMAKE_OBJCOPY=${CMAKE_OBJCOPY}")
message("  CMAKE_OBJDUMP=${CMAKE_OBJDUMP}")
message("  CMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}")

message("")
message("  CMAKE_C_FLAGS=${CMAKE_C_FLAGS}")
message("  CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
message("  LDFLAGS=${LDFLAGS}")
message("  LINK_FLAGS=${LINK_FLAGS}")
message("  CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

message("Makefile created.")
message("")
message("")



# [/section] Dump Setting
