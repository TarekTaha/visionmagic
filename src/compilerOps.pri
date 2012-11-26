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

# Common part for all G++ compilers :
*-g++* {
    !build_pass:message(----------- Compiling using GNU g++ ----------------)

    # Get more warnings on all G++ compilers.
    #QMAKE_CXXFLAGS_WARN_ON *= -Wall -W -Woverloaded-virtual -Wimplicit
    # Disable optimisations while in debug mode.
    QMAKE_CXXFLAGS_DEBUG = -O0 \ #   Reduce compilation time and make debugging produce the expected results. This is the default.
                           -fno-omit-frame-pointer \
                           -g
    QMAKE_LFLAGS_DEBUG += -g

    # Use most agressive AND standard 'optimizing for speed' option.
    # following the guidlines of GNU http://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html
    QMAKE_CXXFLAGS_RELEASE = -O3 \
                             -o -ffast-math\ # -march=i386 \
                             -msse2 \
                             -mfpmath=sse \
                             -ffunction-sections \  #opencv
                             -fomit-frame-pointer\  #opencv
                             -Wno-unused-function   #opencv
}
