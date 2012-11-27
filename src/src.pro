TEMPLATE = app
DESTDIR = ../bin
#DEFINES += QT_NO_DEBUG_OUTPUT
DEFINES += OPENCV
CONFIG += ordered \
    warn_on \
    qt \
    opengl \
    thread \
    exceptions \
    stl \
    resources

# console
TARGET = VisionMagic

#CONFIG(debug, debug|release) {
#    !build_pass:message(+++++++++++ SRC - Debug Build ++++++++++++)
#    MOC_DIR = ./debug/moc
#    OBJECTS_DIR = ./debug/obj
#    RCC_DIR = ./debug/rcc
#    UI_DIR = ./debug/ui
#}
#CONFIG(release, debug|release) {
#    !build_pass:message(+++++++++++ SRC - Release Build ++++++++++++)
#    MOC_DIR = ./release/moc
#    OBJECTS_DIR = ./release/obj
#    RCC_DIR = ./release/rcc
#    UI_DIR = ./release/ui
#}
#RESOURCES = ../resources/rcs.qrc
!include( opencv.pri ):error( Unable to find opencv.pri )
!include( qwt.pri ):error( Unable to find qwt.pri )
!include( boost.pri ):error( Unable to find boost.pri )
!include( compilerOps.pri ):error( Unable to find compilerOps.pri )
QT += core \
    gui \
    opengl \
    xml \
    webkit \
    script \
    svg \
    sql
INCLUDEPATH += ./gui \
    ./modules/autoFocus \
    ./modules/cameraGrabber \
    ./modules/exceptions \
    ./modules/imageAnalysis \
    ./modules/logger/ \
    ./modules/sceneRendering/ \
    ./modules/shared/ \
    ./modules/imageAcquisition
SOURCES += main.cpp \
    gui/mainwindow.cpp \
    gui/settingsdialog.cpp \
    gui/splashscreen.cpp \
    modules/sceneRendering/QOpenCVWidget.cpp \
    modules/sceneRendering/scenevisualization.cpp \
    modules/imageAnalysis/imageanalysisalgorithms.cpp \
    gui/templatemodel.cpp \
    modules/logger/messagelogger.cpp \
    gui/acdsettings.cpp \
    modules/cameraGrabber/cameragrabber.cpp \
    modules/cameraGrabber/avtcamera.cpp \
    modules/cameraGrabber/camera.cpp \
    modules/cameraGrabber/opencvcamera.cpp \
    modules/logger/dlogger.cpp \
    modules/imageAcquisition/imageviewer.cpp \
    gui/treeitem.cpp \
    gui/treemodel.cpp
HEADERS += gui/mainwindow.h \
    gui/settingsdialog.h \
    gui/splashscreen.h \
    modules/sceneRendering/QOpenCVWidget.h \
    modules/sceneRendering/scenevisualization.h \
    modules/imageAnalysis/imageanalysisalgorithms.h \
    modules/imageAnalysis/mathfun.h \
    gui/templatemodel.h \
    modules/logger/messagelogger.h \
    gui/version.h \
    gui/acdsettings.h \
    gui/settings.h \
    modules/exceptions/acdexceptions.h \
    modules/exceptions/EncapsulationExceptions.h \
    modules/cameraGrabber/cameragrabber.h \
    modules/exceptions/myexception.h \
    modules/cameraGrabber/avtcamera.h \
    modules/cameraGrabber/camera.h \
    modules/cameraGrabber/opencvcamera.h \
    modules/logger/dlogger.h \
    modules/imageAcquisition/imageviewer.h \
    gui/treeitem.h \
    gui/treemodel.h
FORMS += gui/mainwindow.ui \
    gui/settingsdialog_camera.ui \
    gui/settingsdialog_mes.ui \
    gui/settingsdialog_spa.ui \
    gui/settingsdialog_focus.ui \
    gui/settingsdialog_ledlighting.ui \
    gui/settingsdialog_motors.ui \
    gui/settingsdialog_acquisition.ui \
    gui/settingsdialog_analysis.ui \
    gui/settingsdialog.ui \
    modules/logger/messagelogger.ui
win32 {
    DEFINES += _TTY_WIN_
    #RC_FILE = ../resources/appIcon.rc
    INCLUDEPATH+= "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/include"
    HEADERS    += "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/include/UniControl.h"
    LIBS +=  \
        "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniControl.lib" \
        "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniTransform.lib" \
        -lsetupapi -lAdvapi32 \
        -lshell32
}
unix {
    DEFINES += _TTY_POSIX_
}






