build_pass:CONFIG(debug, debug|release) {
    QWTLIB = qwtd
}
CONFIG(release, debug|release){
    QWTLIB = qwt
}

win32:ROOT_DIR = $$system( cd )
else:ROOT_DIR  = $$system( pwd )
QWT_ROOT       = "$${ROOT_DIR}/modules/qwt"


message("QWT ROOT IS: $${QWT_ROOT}")

SUFFIX_STR =
VVERSION = $$[QT_VERSION]
isEmpty(VVERSION){
    # Qt 3
    debug{
        SUFFIX_STR = $${DEBUG_SUFFIX}
    }
    else{
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
message("QWT INCLUDEPATH IS: $${INCLUDEPATH}")
#QWTLIB       = qwt$${SUFFIX_STR}

contains(QWT_CONFIG, QwtFramework){
    LIBS+= -F$${QWT_ROOT}/lib
}
else {
    LIBS+= -L$${QWT_ROOT}/lib
}

contains(QWT_CONFIG, QwtSvg){
    QT+= svg
}
else{
    DEFINES+= QWT_NO_SVG
}

contains(CONFIG, QwtDll){
    DEFINES+= QT_DLL QWT_DLL
    QWTLIB = $${QWTLIB}$${VER_MAJ}
}

win32-msvc:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc.net:LIBS  += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc2002:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc2003:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc2005:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc2008:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-msvc2010:LIBS += $${QWT_ROOT}/lib/$${QWTLIB}.lib
win32-g++:LIBS      += -L$${QWT_ROOT}/lib -l$${QWTLIB}
