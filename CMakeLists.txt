# ----------------------------------------------------------------------------
#  Root CMake file for ACD
#
#    From the off-tree build directory, invoke:
#      $ cmake <PATH_TO_ACD_ROOT>
#
# ----------------------------------------------------------------------------

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
# Add these standard paths to the search paths for FIND_LIBRARY
# to find libraries from these locations first
if(UNIX)
  set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /lib /usr/lib)
endif()
# it _must_ go before PROJECT(ACD) in order to work
if (NOT CMAKE_INSTALL_PREFIX)
    if (WIN32)
        set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE INTERNAL "" FORCE)
    elseif()
        set(CMAKE_INSTALL_PREFIX "/usr" CACHE INTERNAL "" FORCE)
    endif()
endif()

set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "Configs" FORCE)
set(CMAKE_C_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_C_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "" CACHE INTERNAL "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "" CACHE INTERNAL "" FORCE)
set(CMAKE_VERBOSE OFF CACHE BOOL "Verbose mode")

if (CMAKE_VERBOSE)
        set (CMAKE_VERBOSE_MAKEFILE 1)
endif()

project(ACD)

cmake_minimum_required(VERSION 2.4)

# --------------------------------------------------------------
# Indicate CMake 2.7 and above that we don't want to mix relative
#  and absolute paths in linker lib lists.
# Run "cmake --help-policy CMP0003" for more information.
# --------------------------------------------------------------
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

# ----------------------------------------------------------------------------
#  Current version number:
# ----------------------------------------------------------------------------
set(ACD_VERSION "3.9.3")

string(REGEX MATCHALL "[0-9]" ACD_VERSION_PARTS "${ACD_VERSION}")

list(GET ACD_VERSION_PARTS 0 ACD_VERSION_MAJOR)
list(GET ACD_VERSION_PARTS 1 ACD_VERSION_MINOR)
list(GET ACD_VERSION_PARTS 2 ACD_VERSION_PATCH)

set(ACD_SOVERSION "${ACD_VERSION_MAJOR}.${ACD_VERSION_MINOR}")

# ----------------------------------------------------------------------------
#  Variables for cvconfig.h.cmake
# ----------------------------------------------------------------------------
set(PACKAGE "ACD")
set(PACKAGE_BUGREPORT "ttaha@silverbrookresearch.com")
set(PACKAGE_NAME "opencv")
set(PACKAGE_STRING "${PACKAGE} ${ACD_VERSION}")
set(PACKAGE_TARNAME "${PACKAGE}")
set(PACKAGE_VERSION "${ACD_VERSION}")

# ----------------------------------------------------------------------------
#  Autodetect if we are in a SVN repository
# ----------------------------------------------------------------------------
find_program(SVNVERSION_PATH svnversion)
mark_as_advanced(force SVNVERSION_PATH)
if(SVNVERSION_PATH)
    message(STATUS "Extracting svn version, please wait...")
    execute_process(COMMAND ${SVNVERSION_PATH} -n ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE SVNVERSION_RESULT)

    if(SVNVERSION_RESULT MATCHES "exported")
       # This is NOT a svn repository:
       set(ACD_SVNVERSION "")
    else()
        set(ACD_SVNVERSION " svn:${SVNVERSION_RESULT}")
    endif()

    message(STATUS "SVNVERSION: ${ACD_SVNVERSION}")
else()
    # We don't have svnversion:
    set(ACD_SVNVERSION "")
endif()

# ----------------------------------------------------------------------------
# Detect GNU version:
# ----------------------------------------------------------------------------
if(CMAKE_COMPILER_IS_GNUCXX)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                  OUTPUT_VARIABLE CMAKE_ACD_GCC_VERSION_FULL
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

    # Typical output in CMAKE_ACD_GCC_VERSION_FULL: "c+//0 (whatever) 4.2.3 (...)"
    #  Look for the version number
    string(REGEX MATCH "[0-9].[0-9].[0-9]" CMAKE_GCC_REGEX_VERSION "${CMAKE_ACD_GCC_VERSION_FULL}")

    # Split the three parts:
    string(REGEX MATCHALL "[0-9]" CMAKE_ACD_GCC_VERSIONS "${CMAKE_GCC_REGEX_VERSION}")

    list(GET CMAKE_ACD_GCC_VERSIONS 0 CMAKE_ACD_GCC_VERSION_MAJOR)
    list(GET CMAKE_ACD_GCC_VERSIONS 1 CMAKE_ACD_GCC_VERSION_MINOR)

    set(CMAKE_ACD_GCC_VERSION ${CMAKE_ACD_GCC_VERSION_MAJOR}${CMAKE_ACD_GCC_VERSION_MINOR})
    math(EXPR CMAKE_ACD_GCC_VERSION_NUM "${CMAKE_ACD_GCC_VERSION_MAJOR}*100 + ${CMAKE_ACD_GCC_VERSION_MINOR}")
    message(STATUS "Detected version of GNU GCC: ${CMAKE_ACD_GCC_VERSION} (${CMAKE_ACD_GCC_VERSION_NUM})")

    if(WIN32)
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
                  OUTPUT_VARIABLE CMAKE_ACD_GCC_TARGET_MACHINE
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
        if(CMAKE_ACD_GCC_TARGET_MACHINE MATCHES "64")
            set(MINGW64 1)
        endif()
    endif()

endif()


if(CMAKE_COMPILER_IS_GNUCXX)
    set(ENABLE_PROFILING OFF CACHE BOOL "Enable profiling in the GCC compiler (Add flags: -g -pg)")
    set(USE_OMIT_FRAME_POINTER ON CACHE BOOL "Enable -fomit-frame-pointer for GCC")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES arm*)
        # We can use only -O2 because the -O3 causes gcc crash
        set(USE_O2 ON CACHE BOOL "Enable -O2 for GCC")
        set(USE_FAST_MATH ON CACHE BOOL "Enable -ffast-math for GCC")
    endif()

    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES amd64*)
        set(X86_64 1)
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64*)
        set(X86_64 1)
    endif()

    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES i686*)
        set(X86 1)
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES i386*)
        set(X86 1)
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86*)
        set(X86 1)
    endif()

    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES powerpc*)
        set(USE_O3 ON CACHE BOOL "Enable -O3 for GCC")
        set(ENABLE_POWERPC ON CACHE BOOL "Enable PowerPC for GCC")
    endif ()

    if(X86 OR X86_64)
        # enable everything, since the available set of instructions is checked at runtime
        set(USE_O3 ON CACHE BOOL "Enable -O3 for GCC")
        set(USE_FAST_MATH ON CACHE BOOL "Enable -ffast-math for GCC")
        set(ENABLE_SSE ON CACHE BOOL "Enable SSE for GCC")
        set(ENABLE_SSE2 ON CACHE BOOL "Enable SSE2 for GCC")
        set(ENABLE_SSE3 OFF CACHE BOOL "Enable SSE3 for GCC")
        set(ENABLE_SSSE3 OFF CACHE BOOL "Enable SSSE3 for GCC")
        #set(ENABLE_SSE4_1 OFF CACHE BOOL "Enable SSE4.1 for GCC")
    endif()
endif()

##################
#    QT STUFF    #
##################

FIND_PACKAGE( Qt4 REQUIRED)
FIND_PACKAGE (OpenGL QUIET)
SET(QT_USE_QTGUI TRUE )
SET(QT_USE_QTOPENGL TRUE )
SET(QT_USE_QTXML TRUE )
SET(QT_USE_QTSCRIPT TRUE )
SET(QT_USE_QTWEBKIT TRUE )
SET(QT_USE_QTXMLPATTERNS TRUE)
SET(QT_USE_QT3SUPPORT TRUE)
INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS( ${QT_DEFINITIONS} -DHAVE_QT_OPENGL -DQT_OPENGL_SUPPORT)

set(ACD_LINKER_LIBS ${ACD_LINKER_LIBS} ${OPENGL_LIBRARIES})

FIND_PACKAGE(Doxygen)

if(DOXYGEN_FOUND)
    set(BUILD_DOXYGEN_DOCS ON CACHE BOOL "Generate HTML docs using Doxygen")
endif()

# ----------------------------------------------------------------------------
#                   UPDATE CONFIG FILES & SCRIPTS:
#
#  CONFIGURE_FILE(InputFile OutputFile [COPYONLY] [ESCAPE_QUOTES] [@ONLY])
# If @ONLY is specified, only variables of the form @VAR@ will be
#  replaces and ${VAR} will be ignored.
#
#  A directory will be created for each platform so the "cvconfig.h" file is
#   not overwritten if cmake generates code in the same path.
# ----------------------------------------------------------------------------
add_definitions(-DHAVE_CONFIG_H)

set(ACD_CONFIG_FILE_INCLUDE_DIR "${CMAKE_BINARY_DIR}/" CACHE PATH "Where to create the platform-dependant cvconfig.h")

# ---------------------------------------------------------------------------
# The C+//0 include & link directories:
# ---------------------------------------------------------------------------
include_directories("."
    "${CMAKE_CURRENT_SOURCE_DIR}"
    )

# ----------------------------------------------------------------------------
#           Set the maximum level of warnings:
# ----------------------------------------------------------------------------
# Should be set to true for development
set(ACD_WARNINGS_ARE_ERRORS OFF CACHE BOOL "Treat warnings as errors")

set(EXTRA_C_FLAGS "")
set(EXTRA_C_FLAGS_RELEASE "")
set(EXTRA_C_FLAGS_DEBUG "")
set(EXTRA_EXE_LINKER_FLAGS "")
set(EXTRA_EXE_LINKER_FLAGS_RELEASE "")
set(EXTRA_EXE_LINKER_FLAGS_DEBUG "")

if(CMAKE_COMPILER_IS_GNUCXX)

    # High level of warnings.
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wall")

    # The -Wno-long-long is required in 64bit systems when including sytem headers.
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES x86_64*)
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-long-long")
    endif()
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES amd64*)
    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Wno-long-long")
    endif()

    # We need pthread's
    if(UNIX)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -pthread")
    endif()

    if(ACD_WARNINGS_ARE_ERRORS)
        set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -Werror")
    endif()

    if(X86)
        if(NOT MINGW64)
            if(NOT X86_64)
                if(NOT APPLE)
                    set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -march=i686")
                endif()
            endif()
        endif()
    endif()

    # Other optimizations
    if(USE_OMIT_FRAME_POINTER)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -fomit-frame-pointer")
    endif()
    if(USE_O2)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -O2")
    endif()
    if(USE_O3)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -O3")
    endif()
    if(USE_FAST_MATH)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -ffast-math")
    endif()
    if(ENABLE_POWERPC)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -mcpu=G3 -mtune=G5")
    endif()
    if(ENABLE_SSE)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -msse")
    endif()
    if(ENABLE_SSE2)
       set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -msse2")
    endif()
    # SSE3 and further should be disabled under MingW because it generates compiler errors
    if(NOT MINGW)
       if(ENABLE_SSE3)
          set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -msse3")
       endif()
       if(${CMAKE_ACD_GCC_VERSION_NUM} GREATER 402)
          set(HAVE_GCC43_OR_NEWER 1)
       endif()
       if(HAVE_GCC43_OR_NEWER OR APPLE)
          if(ENABLE_SSSE3)
             set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -mssse3")
          endif()
          #if(ENABLE_SSE4_1)
          #   set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -msse4.1")
          #endif()
       endif()
    endif()

    if(X86 OR X86_64)
        if(NOT APPLE)
            if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
                set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -mfpmath=387")
            endif()
        endif()
    endif()

    # Profiling?
    if(ENABLE_PROFILING)
        set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -pg -g")
    else()
        # Remove unreferenced functions: function level linking
        if(NOT APPLE)
            set(EXTRA_C_FLAGS "${EXTRA_C_FLAGS} -ffunction-sections")
        endif()
    endif()

    set(EXTRA_C_FLAGS_RELEASE "${EXTRA_C_FLAGS_RELEASE} -DNDEBUG")
    set(EXTRA_C_FLAGS_DEBUG "${EXTRA_C_FLAGS_DEBUG} -O0 -DDEBUG -D_DEBUG")
endif()

# Extra link libs if the user selects building static libs:
IF(NOT BUILD_SHARED_LIBS)
    if(CMAKE_COMPILER_IS_GNUCXX)
        set(ACD_LINKER_LIBS ${ACD_LINKER_LIBS} stdc++)
    endif()

    set(ACD_LINKER_LIBS ${ACD_LINKER_LIBS} flann zlib opencv_lapack)
endif()


# Add user supplied extra options (optimization, etc...)
# ==========================================================
set(ACD_EXTRA_C_FLAGS "" CACHE STRING "Extra compiler options")
set(ACD_EXTRA_C_FLAGS_RELEASE "" CACHE STRING "Extra compiler options for Release build")
set(ACD_EXTRA_C_FLAGS_DEBUG "" CACHE STRING "Extra compiler options for Debug build")
set(ACD_EXTRA_EXE_LINKER_FLAGS "" CACHE STRING "Extra linker flags" FORCE)
set(ACD_EXTRA_EXE_LINKER_FLAGS_RELEASE "" CACHE STRING "Extra linker flags for Release build" FORCE)
set(ACD_EXTRA_EXE_LINKER_FLAGS_DEBUG "" CACHE STRING "Extra linker flags for Debug build" FORCE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_C_FLAGS} ${ACD_EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_C_FLAGS} ${ACD_EXTRA_C_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE} ${ACD_EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${EXTRA_C_FLAGS_RELEASE} ${ACD_EXTRA_C_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG} ${ACD_EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${EXTRA_C_FLAGS_DEBUG} ${ACD_EXTRA_C_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EXTRA_EXE_LINKER_FLAGS} ${ACD_EXTRA_EXE_LINKER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${EXTRA_EXE_LINKER_FLAGS_RELEASE} ${ACD_EXTRA_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${EXTRA_EXE_LINKER_FLAGS_DEBUG} ${ACD_EXTRA_EXE_LINKER_FLAGS_DEBUG}")

# In case of Makefiles if the user does not setup CMAKE_BUILD_TYPE, assume it's Release:
if (${CMAKE_GENERATOR} MATCHES ".*Makefiles")
    if("${CMAKE_BUILD_TYPE}" STREQUAL "")
        set(CMAKE_BUILD_TYPE Release)
    endif()
endif()

if("${CMAKE_CONFIGURE_LDFLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_CONFIGURE_LDFLAGS}")
endif("${CMAKE_CONFIGURE_LDFLAGS}")

# ----------------------------------------------------------------------------
#                       PROCESS SUBDIRECTORIES:
# ----------------------------------------------------------------------------
# Save libs and executables in the same place
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib CACHE PATH "Output directory for libraries" )
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin CACHE PATH "Output directory for applications" )

if(WIN32)
set(ACD_DOC_INSTALL_PATH doc)
else()
set(ACD_DOC_INSTALL_PATH share/opencv/doc)
endif()


#-----------------------------------
# Subdirectories:
#-----------------------------------
#add_subdirectory(include)
#add_subdirectory(modules)

# ----------------------------------------------------------------------------
#   Sumary:
# ----------------------------------------------------------------------------
message(STATUS "")
message(STATUS "General configuration for opencv ${ACD_VERSION} =====================================")
message(STATUS "")
message(STATUS "    Built as dynamic libs?:    ${BUILD_SHARED_LIBS}")
message(STATUS "    Compiler:                  ${CMAKE_COMPILER}")
message(STATUS "    C++ flags (Release):       ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "    C++ flags (Debug):         ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
if(WIN32)
message(STATUS "    Linker flags (Release):    ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
message(STATUS "    Linker flags (Debug):      ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
else()
message(STATUS "    Linker flags (Release):    ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_RELEASE}")
message(STATUS "    Linker flags (Debug):      ${CMAKE_SHARED_LINKER_FLAGS} ${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
endif()

#YV
message(STATUS "")
message(STATUS "  GUI: ")

if (HAVE_QT)
    message(STATUS "    QT 4.x:                    ${HAVE_QT}")
    message(STATUS "    QT OpenGL support:         ${HAVE_QT_OPENGL}")
else()
    if(WIN32)
        message(STATUS "    Win32 UI:                  1")
    else()
        if(APPLE)
            if(WITH_CARBON)
                message(STATUS "    Carbon:                    1")
            else()
                message(STATUS "    Cocoa:                     1")
            endif()
        else()
            message(STATUS "    GTK+ 2.x:                  ${HAVE_GTK}")
            message(STATUS "    GThread:                   ${HAVE_GTHREAD}")
        endif()
    endif()
endif()

message(STATUS "")
message(STATUS "  Image I/O: ")
if(NOT WITH_JPEG OR JPEG_FOUND)
message(STATUS "    JPEG:                      ${JPEG_FOUND}")
else()
message(STATUS "    JPEG:                      build")
endif()

if(NOT WITH_PNG OR PNG_FOUND)
message(STATUS "    PNG:                       ${PNG_FOUND}")
else()
message(STATUS "    PNG:                       build")
endif()

if(NOT WITH_TIFF OR TIFF_FOUND)
message(STATUS "    TIFF:                      ${TIFF_FOUND}")
else()
message(STATUS "    TIFF:                      build")
endif()

if(NOT WITH_JASPER OR JASPER_FOUND)
message(STATUS "    JPEG 2000:                 ${JASPER_FOUND}")
else()
message(STATUS "    JPEG 2000:                 build")
endif()

if(WITH_OPENEXR AND OPENEXR_FOUND)
message(STATUS "    OpenEXR:                   YES")
else()
message(STATUS "    OpenEXR:                   NO")
endif()

if(UNIX AND NOT APPLE)
message(STATUS "")
message(STATUS "  Video I/O: ")
message(STATUS "    DC1394 1.x:                ${HAVE_DC1394}")
message(STATUS "    DC1394 2.x:                ${HAVE_DC1394_2}")
message(STATUS "    FFMPEG:                    ${HAVE_FFMPEG}")
message(STATUS "      codec:                   ${HAVE_FFMPEG_CODEC}")
message(STATUS "      format:                  ${HAVE_FFMPEG_FORMAT}")
message(STATUS "      util:                    ${HAVE_FFMPEG_UTIL}")
message(STATUS "      swscale:                 ${HAVE_FFMPEG_SWSCALE}")
message(STATUS "      gentoo-style:            ${HAVE_GENTOO_FFMPEG}")
message(STATUS "    GStreamer:                 ${HAVE_GSTREAMER}")
message(STATUS "    UniCap:                    ${HAVE_UNICAP}")
message(STATUS "    PvAPI:                     ${HAVE_PVAPI}")
if(HAVE_LIBV4L)
message(STATUS "    V4L/V4L2:                  Using libv4l")
else()
message(STATUS "    V4L/V4L2:                  ${HAVE_CAMV4L}/${HAVE_CAMV4L2}")
endif()
message(STATUS "    Xine:                      ${HAVE_XINE}")
endif()

if(APPLE)
message(STATUS "")
if(WITH_QUICKTIME)
message(STATUS "  Video I/O:                   QuickTime")
else()
message(STATUS "  Video I/O:                   QTKit")
endif()
endif()

if(WIN32)
message(STATUS "")
message(STATUS "  Video I/O: ")
if(HAVE_VIDEOINPUT)
message(STATUS "    VideoInput:                1")
else()
message(STATUS "    VideoInput:                0")
endif()
endif()

message(STATUS "")
message(STATUS "  Interfaces: ")
if(HAVE_TBB)
message(STATUS "    Use TBB:                   YES")
else()
message(STATUS "    Use TBB:                   NO")
endif()

message(STATUS "")
message(STATUS "  Documentation: ")

if(BUILD_LATEX_DOCS AND PDFLATEX_COMPILER)
message(STATUS "    Build PDF                  YES")
else()
message(STATUS "    Build PDF                  NO")
endif()

if(BUILD_DOXYGEN_DOCS AND DOXYGEN_FOUND)
message(STATUS "    Doxygen HTMLs              YES")
else()
message(STATUS "    Doxygen HTMLs              NO")
endif()

message(STATUS "")
message(STATUS "    Install path:              ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "    cvconfig.h is in:          ${ACD_CONFIG_FILE_INCLUDE_DIR}")
message(STATUS "-----------------------------------------------------------------")
message(STATUS "")

ADD_DEFINITIONS(-DQT_NO_DEBUG -DACD_ONLY -D_TTY_WIN_ -DLOCK_SPA_PARAMETERS -DCONTINUOUS_SWEEP)

SET( ACD_SRCS
    src/modules/imageAcquisition/imageacquisition.cpp
    src/main.cpp
    src/modules/autoFocus/autofocus.cpp
    src/modules/motorControl/serialLib/qextserialbase.cpp
    src/modules/motorControl/serialLib/qextserialport.cpp
    src/modules/motorControl/serialLib/qextserialenumerator.cpp
    src/modules/motorControl/serialLib/win_qextserialport.cpp
    src/modules/motorControl/motorcontrol.cpp
    src/gui/mainwindow.cpp
    src/gui/settingsdialog.cpp
    src/modules/dnmap/dnmapprobabilisticgenerator.cpp
    src/modules/visualRendering/QOpenCVWidget.cpp
    src/modules/visualRendering/scenevisualization.cpp
    src/modules/imageAcquisition/imageprovider.cpp
    src/modules/imageAnalysis/imageanalysis.cpp
    src/modules/imageAnalysis/imageanalysisalgorithms.cpp
    src/gui/templatemodel.cpp
    src/modules/logger/messagelogger.cpp
    src/modules/logger/logger.cpp
    src/gui/acdsettings.cpp
    src/gui/dnmapgeneratordialog.cpp
    src/modules/imageAnalysis/linefit.cpp
    src/modules/imageAnalysis/line.cpp
    src/modules/nozzleTools/nozzle.cpp
    src/modules/ledLightingController/ledlightingcontroller.cpp
    src/modules/motorControl/comportdetection.cpp
    src/modules/motorControl/comportdetectionthread.cpp
    src/modules/motorControl/configparser.cpp
    src/modules/visualRendering/nozzlehealthviewer.cpp
    src/modules/visualRendering/chipgraphicitem.cpp
    src/modules/scannerInterface/scannerinterface.cpp
    src/modules/nozzleTools/Chip.cpp
    src/modules/nozzleTools/nozzleoperations.cpp
    src/modules/nozzleTools/nozzleEjectionBehaviour.cpp
    src/modules/nozzleTools/matchingnozzle.cpp
    src/modules/encapsulationCoverage/encapsulationcoverage.cpp
    src/modules/particlesProfiler/particlesprofiler.cpp
    src/modules/encapsulationCoverage/calibrationdialog.cpp
    src/modules/cameraGrabber/cameragrabber.cpp
    src/modules/nozzleTools/nhmap.cpp
    src/modules/18-16-04-08-CPPLibs/src/ControlInfo.cpp
    src/modules/18-16-04-08-CPPLibs/src/Geometry2D.cpp
    src/modules/18-16-04-08-CPPLibs/src/LeastSquares.cpp
    src/modules/18-16-04-08-CPPLibs/src/log.cpp
    src/modules/shared/arthurstyle.cpp
    src/modules/shared/arthurwidgets.cpp
    src/modules/shared/hoverpoints.cpp
        )
SET( ACD_MOC_HDRS
    src/modules/imageAcquisition/imageacquisition.h
    src/modules/autoFocus/autofocus.h
    src/modules/motorControl/serialLib/qextserialbase.h
    src/modules/motorControl/serialLib/qextserialport.h
    src/modules/motorControl/serialLib/win_qextserialport.h
    src/modules/motorControl/motorcontrol.h
    src/gui/mainwindow.h
    src/gui/settingsdialog.h
    src/modules/dnmap/dnmapprobabilisticgenerator.h
    src/modules/visualRendering/QOpenCVWidget.h
    src/modules/visualRendering/scenevisualization.h
    src/modules/imageAnalysis/imageanalysis.h
    src/gui/templatemodel.h
    src/modules/logger/messagelogger.h
    src/gui/acdsettings.h
    src/gui/dnmapgeneratordialog.h
    src/modules/motorControl/comportdetection.h
    src/modules/motorControl/comportdetectionthread.h
    src/modules/visualRendering/nozzlehealthviewer.h
    src/modules/scannerInterface/scannerinterface.h
    src/modules/encapsulationCoverage/encapsulationcoverage.h
    src/modules/particlesProfiler/particlesprofiler.h
    src/modules/encapsulationCoverage/calibrationdialog.h
    src/modules/cameraGrabber/cameragrabber.h
    src/modules/shared/arthurwidgets.h
    src/modules/shared/hoverpoints.h
    )
# UI .ui files
SET(ACD_UIS
    src/gui/mainwindow.ui
    src/gui/settingsdialog_camera.ui
    src/gui/settingsdialog_mes.ui
    src/gui/settingsdialog_spa.ui
    src/gui/settingsdialog_focus.ui
    src/gui/settingsdialog_ledlighting.ui
    src/gui/settingsdialog_motors.ui
    src/gui/settingsdialog_acquisition.ui
    src/gui/settingsdialog_analysis.ui
    src/gui/settingsdialog.ui
    src/modules/logger/messagelogger.ui
    src/gui/dnmapgeneratordialog.ui
    src/modules/motorControl/comportdetection.ui
    src/modules/encapsulationCoverage/calibrationdialog.ui
    )
# Resources files
SET(ACD_RCS resources/rcs.qrc src/modules/shared/shared.qrc)
SET(LIB_TIFF_ROOT "C:/pswlib/libtiff/3.9.1/libtiff")
#SET(OPENCV_ROOT   "C:/pswlib/opencv/1.1pre/")
SET(BOOST_ROOT    "C:/pswlib/boost/1_42_0/gcc-mingw-4.4.0")

QT4_ADD_RESOURCES( ACD_RC_SRCS ${ACD_RCS} )
# this will run uic on .ui files:
QT4_WRAP_UI( ACD_UI_HDRS ${ACD_UIS} )
# and finally this will run moc:
QT4_WRAP_CPP( ACD_MOC_SRCS ${ACD_MOC_HDRS} )

FIND_PACKAGE(OpenCV REQUIRED )

# we need this to be able to include headers produced by uic in our code
INCLUDE_DIRECTORIES(
    ${QT_INCLUDES}
    ${CMAKE_BINARY_DIR}
    ${OpenCV_INCLUDE_DIRS}
    src/gui
    src/modules/autoFocus
    src/modules/cameraGrabber
    src/modules/dnmap
    src/modules/exceptions
    src/modules/imageAcquisition
    src/modules/imageAnalysis
    src/modules/imageAnalysis/cvBlobsLib
    src/modules/ledLightingController
    src/modules/logger/
    src/modules/motorControl/serialLib/
    src/modules/motorControl
    src/modules/visualRendering/
    src/modules/shared
    src/modules/scannerInterface
    src/modules/encapsulationCoverage
    src/modules/particlesProfiler
    src/modules/nozzleTools
    "C:/Program Files/EZTwain/VC"
    "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/include"
    src/modules/18-16-04-08-CPPLibs
    src/modules/18-16-04-08-CPPLibs/src
    src/modules/18-16-04-10-BasicTypes
    src/modules/18-12-04-07-NHALib
#    "${OPENCV_ROOT}/cxcore/include"
#    "${OPENCV_ROOT}/cv/include"
#    "${OPENCV_ROOT}/otherlibs/highgui"
    ${LIB_TIFF_ROOT}
    ${BOOST_ROOT}
)

SET(ICON_RC_FILE  "${CMAKE_CURRENT_SOURCE_DIR}/resources/appIcon.rc")
STRING(REPLACE "/" "\\\\" YES "${ICON_RC_FILE}")
SET(ICON_RC_FILE \"${YES}\")

SET(ICON_OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/ICON_OBJ.o")
STRING(REPLACE "/" "\\\\" YES "${ICON_OBJ_FILE}")
SET(ICON_OBJ_FILE \"${YES}\")

MESSAGE(STATUS "MAKE_WINDOWS_PATH end name: ${ICON_RC_FILE}")
MESSAGE(STATUS "MAKE_WINDOWS_PATH end name: ${ICON_OBJ_FILE}")

#This is the icon generation stuff, so stupid
#Doesn't work at the moment
IF( MINGW )
    # resource compilation for MinGW
    ADD_CUSTOM_COMMAND( OUTPUT ${ICON_OBJ_FILE}
                        COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR} -i${ICON_RC_FILE}
                             -o ${ICON_OBJ_FILE} )
#    SET(ACD_SRCS ${ACD_SRCS} ${ICON_OBJ_FILE})
ELSE( MINGW )
    SET(ACD_SRCS ${ACD_SRCS} "${CMAKE_CURRENT_SOURCE_DIR}/resources/appIcon.rc")
ENDIF( MINGW )

# here we instruct CMake to build "ACD" executable from all of the source files
# WIN32 gets rid of the console
ADD_EXECUTABLE( ACD WIN32 ${ACD_SRCS} ${ACD_MOC_SRCS} ${ACD_RC_SRCS} ${ACD_UI_HDRS} )
FIND_PACKAGE(OpenGL)
FIND_PACKAGE(threads)
MESSAGE(STATEUS " OPENCV PATH: ${OpenCV_LIBS} Lib DIR: ${OpenCV_LIB_DIR} Include DIR:${OpenCV_INCLUDE_DIRS}")
LINK_DIRECTORIES(
    "C:/pswlib/opencv/2.1.0/build/lib"
    )
TARGET_LINK_LIBRARIES( ACD
    ${CMAKE_THREAD_LIBS_INIT}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTOPENGL_LIBRARY}
    ${QT_QTXML_LIBRARY}
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARY}
    #libjpeg libtiff libjasper libpng zlib avcodec videoInput avformat avutil avdevice
    ${OpenCV_LIBS}
    "${BOOST_ROOT}/lib/libboost_system-mgw44-mt-s.lib"
    "${BOOST_ROOT}/lib/libboost_program_options-mgw44-mt-s.lib"
    "${BOOST_ROOT}/lib/libboost_filesystem-mgw44-mt-s.lib"
    "C:/Program Files/EZTwain/Release/Eztwain3.lib"
    "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniControl.lib"
    "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniTransform.lib"
    Setupapi.lib
)
