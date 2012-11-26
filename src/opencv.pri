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

INCLUDEPATH += "E:/pswlib/opencv/2.4.3_pack/build/include" \
               "E:/pswlib/opencv/2.4.3_pack/build/include/opencv"


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
