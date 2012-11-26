OPENCV_ROOT  = "E:/pswlib/opencv/2.4.3_pack"
OPENCV_VER = 243
build_pass:CONFIG(debug, debug|release) {
    message("Linking OPENCV $${OPENCV_VER} Debug Libraries")
    TARGET = $${TARGET}_d
    OPENCV_BUILD_ROOT  = "$${OPENCV_ROOT}/build/x86/vc10"
    OPENCV_COMPILER_SUFFIX = "d"
    QWTLIB       = qwtd
}
CONFIG(release, debug|release) {
    message("Linking OPENCV $${OPENCV_VER} Release Libraries")
    OPENCV_BUILD_ROOT  = "$${OPENCV_ROOT}/build/x86/vc10"
    OPENCV_COMPILER_SUFFIX = ""
    QWTLIB       = qwt
}
OPENCV_INCLUDE_PATH     = "$${OPENCV_ROOT}/build/include"
OPENCV_LIB_PATH         = "$${OPENCV_BUILD_ROOT}/staticlib"
OPENCV_3DPARTY_LIB_PATH = "$${OPENCV_BUILD_ROOT}/staticlib"
LIB_TIFF_ROOT           = "E:/pswlib/libtiff/3.9.4/libtiff"
#LIB_TIFF_ROOT           = "$${OPENCV_ROOT}/3rdparty/libtiff"

BOOST_ROOT     = "E:/pswlib/boost/1_42_0/gcc-mingw-4.4.0"
win32:ROOT_DIR = $$system( cd )
else:ROOT_DIR  = $$system( pwd )
QWT_ROOT       = "$${ROOT_DIR}/src/modules/qwt"

###
### The code parsing helper doesn't work Unless I explicity specify the path
###
#INCLUDEPATH += "$${OPENCV_BUILD_ROOT}" \
#            "C:/pswlib/opencv/2.4.1/build-static-release_2.4.1/install/include" \
#            "C:/pswlib/opencv/2.4.1/build-static-release_2.4.1/install/include/opencv" \
#            "C:/pswlib/opencv/2.4.1/build-static-release_2.4.1/install/share/opencv/3rdparty/lib"
INCLUDEPATH += "E:/pswlib/opencv/2.4.3_pack/build/include" \
               "E:/pswlib/opencv/2.4.3_pack/build/include/opencv"
###
###
###

# Copied from win32-msvc.net
# See http://blogs.msdn.com/branbray/archive/2005/07/08/437078.aspx for a nice description
# for the new switch rationals.

win32-msvc2010 {
    message( Compile using Visual Studio 2010 (10.0))

    # Make sure to have C++/C files (TP),
    # Share string literals (in a RO page) (GF),
    # add use exceptions (EHsc),
    # and compile using intrisic functions (Oi),
    # and enlarge compiler memory allocation (by 800%).
    # Now by default : ZR (RTTI) and  Zc:forScope
    QMAKE_CXXFLAGS = -TP \
        -EHsc \
        -Oi \
        -Zm800
    QMAKE_CFLAGS = -TC \
        -EHsc \
        -Oi \
        -Zm800

    # Full Optimise, Global optimize, Optimise for speed,
    # omit frame pointer, and expand any suitable inlines :
    # /Os = also throw Oy Ob2 GF
    # Note : -Os == -O2 == -Ox
    QMAKE_CXXFLAGS_RELEASE = -Os
    QMAKE_CFLAGS_RELEASE = -Os

    # Optimise for debug, and generate navigation file
    # Output debug info (Zi),
    # Don't optimize (Od)
    # /Od = also throw Oi- Oy-  Ob0 GF-
    QMAKE_CXXFLAGS_DEBUG = -Od \
        -Zi \
        /MTd
    QMAKE_CFLAGS_DEBUG = -Od \
        -Zi \
        /MTd

    # Qt Bug #114573 in vcproj generator. Waitting for 3.3.7 ...
    # QMAKE_CXXFLAGS += -arch:SSE2
    # .PDB and .SBR hidden in Obj dir.
    QMAKE_CXXFLAGS += -Fd$$OBJECTS_DIR
    QMAKE_CFLAGS += -Fd$$OBJECTS_DIR

    # avoid Microsoft crazyness about deprecating *NORMALIZED* functions...
    DEFINES += _CRT_SECURE_NO_DEPRECATE=1
    DEFINES += _CRT_NONSTDC_NO_DEPRECATE=1
    DEFINES += _CRT_SECURE_NO_WARNINGS=1
    DEFINES += __STDC_WANT_SECURE_LIB__=1
    DEFINES += _SECURE_ATL=0

    # Bugs in Qt's .h and mkspecs/qmake.conf :
    QMAKE_CXXFLAGS_WARN_ON = -W3

    # Links flags :
    # Add checksum, and machine code X86.
    QMAKE_LFLAGS += /machine:x86 /NODEFAULTLIB:libcmt
    QMAKE_LFLAGS_DEBUG += /NODEFAULTLIB:msvcrtd.lib /NODEFAULTLIB:msvcrt.lib
    release {
        QMAKE_LFLAGS *= /RELEASE
        QMAKE_LFLAGS -= /DEBUG \
            /NOLOGO
    }
    QMAKE_LFLAGS -= /NOLOGO
}

## Common part for all G++ compilers :
#*-g++* {
#    !build_pass:message(----------- Compiling using GNU g++ ----------------)

#    # Get more warnings on all G++ compilers.
#    #QMAKE_CXXFLAGS_WARN_ON *= -Wall -W -Woverloaded-virtual -Wimplicit
#    # Disable optimisations while in debug mode.
#    QMAKE_CXXFLAGS_DEBUG = -O0 \ #   Reduce compilation time and make debugging produce the expected results. This is the default.
#                           -fno-omit-frame-pointer \
#                           -g
#    QMAKE_LFLAGS_DEBUG += -g

#    # Use most agressive AND standard 'optimizing for speed' option.
#    # following the guidlines of GNU http://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
#    QMAKE_CXXFLAGS_RELEASE = -O3 \
#                             -o -ffast-math\ # -march=i386 \
#                             -msse2 \
#                             -mfpmath=sse \
#                             -ffunction-sections \  #opencv
#                             -fomit-frame-pointer\  #opencv
#                             -Wno-unused-function   #opencv
#}

SUFFIX_STR =
VVERSION = $$[QT_VERSION]
isEmpty(VVERSION) {
    # Qt 3
    debug {
        SUFFIX_STR = $${DEBUG_SUFFIX}
    }
    else {
        SUFFIX_STR = $${RELEASE_SUFFIX}
    }
}
else {
    CONFIG(debug, debug|release) {
        SUFFIX_STR = $${DEBUG_SUFFIX}
    }
    else {
        SUFFIX_STR = $${RELEASE_SUFFIX}
    }
}

INCLUDEPATH += $${QWT_ROOT}/src
DEPENDPATH  += $${QWT_ROOT}/src
#QWTLIB       = qwt$${SUFFIX_STR}

contains(QWT_CONFIG, QwtFramework) {

    LIBS      += -F$${QWT_ROOT}/lib
}
else {

    LIBS      += -L$${QWT_ROOT}/lib
}

contains(QWT_CONFIG, QwtSvg) {

    QT += svg
}
else {

    DEFINES += QWT_NO_SVG
}

CONFIG(debug, debug|release):LIBS   += -L"$${ROOT_DIR}/src/modules/tiffIOLib/debug" \
                                       -ltiffIO_d
CONFIG(release, debug|release):LIBS += -L"$${ROOT_DIR}/src/modules/tiffIOLib/release" \
                                       -ltiffIO
win32 {
    INCLUDEPATH +=  $${LIB_TIFF_ROOT} \
                    $${BOOST_ROOT}

#    LIBS += -L"$${BOOST_ROOT}/lib" \
#            "$${BOOST_ROOT}/lib/libboost_system-mgw44-mt-s.lib" \
#            "$${BOOST_ROOT}/lib/libboost_program_options-mgw44-mt-s.lib" \
#            "$${BOOST_ROOT}/lib/libboost_filesystem-mgw44-mt-s.lib"
    contains(CONFIG, QwtDll) {
        DEFINES    += QT_DLL QWT_DLL
        QWTLIB = $${QWTLIB}$${VER_MAJ}
    }
    contains(DEFINES, OPENCV){
        message("Building with OPENCV $${OPENCV_VER}")
        INCLUDEPATH += "$${OPENCV_INCLUDE_PATH}" \
            "$${OPENCV_INCLUDE_PATH}" \
            "$${OPENCV_INCLUDE_PATH}/opencv"
        LIBS+= -L"$${OPENCV_LIB_PATH}" \
               -L"$${OPENCV_3DPARTY_LIB_PATH}"
        Modules = opencv_imgproc opencv_core opencv_features2d opencv_gpu opencv_calib3d opencv_objdetect opencv_video opencv_highgui opencv_ml opencv_legacy opencv_contrib opencv_flann
        for(m, Modules){
           LIBS += -l$${m}$${OPENCV_VER}$${OPENCV_COMPILER_SUFFIX}
           MODULE_PATH  = "$${OPENCV_INCLUDE_PATH}/OPENCV/$$replace(m, opencv_, )"
           INCLUDEPATH += $${MODULE_PATH}
           message("$${MODULE_PATH}")
        }
        LIBS+= \
            -llibpng$${OPENCV_COMPILER_SUFFIX} \
            -lIlmImf$${OPENCV_COMPILER_SUFFIX} \
            -llibjasper$${OPENCV_COMPILER_SUFFIX} \
            -llibtiff$${OPENCV_COMPILER_SUFFIX} \
            -llibjpeg$${OPENCV_COMPILER_SUFFIX} \
            -lopengl32 \
            -lopencv_highgui$${OPENCV_VER}$${OPENCV_COMPILER_SUFFIX} \
            -lzlib$${OPENCV_COMPILER_SUFFIX} -llibpng$${OPENCV_COMPILER_SUFFIX} -llibjasper$${OPENCV_COMPILER_SUFFIX} -llibtiff$${OPENCV_COMPILER_SUFFIX} -llibjpeg$${OPENCV_COMPILER_SUFFIX}  \
            -lQtTest4 \
            -lglu32 -lopengl32 -lcomctl32 -lgdi32 -lole32 -lvfw32 -lwinmm -lstrmiids \
            -loleaut32 -luuid
    }
    win32-msvc:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc.net:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2002:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2003:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2005:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2008:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-msvc2010:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
    win32-g++:LIBS   += -L$${QWT_ROOT}/lib -l$${QWTLIB}
}
unix {
    LIBS += $$system(pkg-config --libs opencv)
    INCLUDEPATH += /usr/include/opencv
}
