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
TARGET = PIT

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
RESOURCES = ../resources/rcs.qrc
CONFIG(debug, debug|release):LIBS += -Lmodules/shared/debug -larthurstyle_shared_d
                                     #-Lmodules/nhaWrapper/debug -lnhalib_d
CONFIG(release, debug|release):LIBS += -Lmodules/shared/release -larthurstyle_shared
                                       #-Lmodules/nhaWrapper/release -lnhalib
!include( opencv.pri ):error( Unable to find opencv.pri )
!include( qwt.pri ):error( Unable to find qwt.pri )
!include( tiff.pri ):error( Unable to find tiff.pri )
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
    ./modules/dnmap \
    ./modules/exceptions \
    ./modules/imageAcquisition \
    ./modules/imageAnalysis \
    ./modules/imageAnalysis/cvBlobsLib \
    ./modules/73-03-07-02-gardasoft_led_light_controller \
    ./modules/73-03-07-03-debug_logger/ \
    ./modules/motorControl/serialLib/ \
    ./modules/motorControl \
    ./modules/73-03-07-04-scene_rendering/ \
#    ./modules/nhaWrapper/ \
    ./modules/shared \
#    ./modules/scannerInterface \
    ./modules/encapsulationCoverage \
    ./modules/nozzleTools \
    "C:/Program Files/EZTwain/VC" \
    "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/include" \
    modules/18-16-04-08-CPPLibs \
    modules/18-16-04-08-CPPLibs/src \
    modules/18-16-04-10-BasicTypes
#    modules/18-12-04-07-NHALib
SOURCES += modules/imageAcquisition/imageacquisition.cpp \
    main.cpp \
    modules/autoFocus/autofocus.cpp \
    modules/motorControl/serialLib/qextserialbase.cpp \
    modules/motorControl/serialLib/qextserialport.cpp \
    modules/motorControl/serialLib/qextserialenumerator.cpp \
    modules/motorControl/motorcontrol.cpp \
    gui/mainwindow.cpp \
    gui/settingsdialog.cpp \
    gui/splashscreen.cpp \
    modules/dnmap/dnmapprobabilisticgenerator.cpp \
    modules/73-03-07-04-scene_rendering/QOpenCVWidget.cpp \
    modules/73-03-07-04-scene_rendering/scenevisualization.cpp \
    modules/imageAcquisition/imageprovider.cpp \
    modules/imageAnalysis/imageanalysis.cpp \
    modules/imageAnalysis/imageanalysisalgorithms.cpp \
    gui/templatemodel.cpp \
    modules/73-03-07-03-debug_logger/messagelogger.cpp \
    gui/acdsettings.cpp \
    gui/dnmapgeneratordialog.cpp \
    modules/imageAnalysis/linefit.cpp \
    modules/imageAnalysis/line.cpp \
    modules/imageAnalysis/encapdelaminspection.cpp \
    modules/nozzleTools/nozzle.cpp \
    modules/73-03-07-02-gardasoft_led_light_controller/ledlightingcontroller.cpp \
    modules/motorControl/comportdetection.cpp \
    modules/motorControl/comportdetectionthread.cpp \
    modules/motorControl/configparser.cpp \
    modules/73-03-07-04-scene_rendering/nozzlehealthviewer.cpp \
    modules/73-03-07-04-scene_rendering/chipgraphicitem.cpp \
#    modules/scannerInterface/scannerinterface.cpp \
    modules/nozzleTools/Chip.cpp \
    modules/nozzleTools/nozzleoperations.cpp \
    modules/nozzleTools/nozzleEjectionBehaviour.cpp \
    modules/nozzleTools/matchingnozzle.cpp \
    modules/encapsulationCoverage/encapsulationcoverage.cpp \
    modules/encapsulationCoverage/calibrationdialog.cpp \
    modules/73-03-07-01-camera_grabber/cameragrabber.cpp \
#    modules/nhaWrapper/nhaWrapper.cpp \
    modules/nozzleTools/nhmap.cpp \
    modules/18-16-04-08-CPPLibs/src/ControlInfo.cpp \
    modules/18-16-04-08-CPPLibs/src/Geometry2D.cpp \
    modules/18-16-04-08-CPPLibs/src/LeastSquares.cpp \
    modules/18-16-04-08-CPPLibs/src/log.cpp \
    modules/73-03-07-01-camera_grabber/avtcamera.cpp \
    modules/73-03-07-01-camera_grabber/camera.cpp \
    modules/73-03-07-01-camera_grabber/opencvcamera.cpp \
    modules/73-03-07-03-debug_logger/dlogger.cpp \
    modules/imageAcquisition/imageviewer.cpp \
    gui/treeitem.cpp \
    gui/treemodel.cpp
HEADERS += modules/imageAcquisition/imageacquisition.h \
    modules/autoFocus/autofocus.h \
    modules/motorControl/serialLib/qextserialbase.h \
    modules/motorControl/serialLib/qextserialport.h \
    modules/motorControl/serialLib/qextserialenumerator.h \
    modules/motorControl/motorcontrol.h \
    gui/mainwindow.h \
    gui/settingsdialog.h \
    gui/splashscreen.h \
    modules/dnmap/dnmapprobabilisticgenerator.h \
    modules/73-03-07-04-scene_rendering/QOpenCVWidget.h \
    modules/73-03-07-04-scene_rendering/scenevisualization.h \
    modules/imageAcquisition/imageprovider.h \
    modules/imageAnalysis/imageanalysis.h \
    modules/imageAnalysis/imageanalysisalgorithms.h \
    modules/imageAnalysis/mathfun.h \
    modules/imageAnalysis/encapdelaminspection.h \
    gui/templatemodel.h \
    modules/73-03-07-03-debug_logger/messagelogger.h \
    gui/version.h \
    gui/acdsettings.h \
    gui/settings.h \
    gui/dnmapgeneratordialog.h \
    modules/imageAnalysis/linefit.h \
    modules/imageAnalysis/line.h \
    modules/nozzleTools/nozzle.h \
    modules/73-03-07-02-gardasoft_led_light_controller/ledlightingcontroller.h \
    modules/exceptions/acdexceptions.h \
    modules/motorControl/comportdetection.h \
    modules/motorControl/comportdetectionthread.h \
    modules/motorControl/configparser.h \
    modules/73-03-07-04-scene_rendering/nozzlehealthviewer.h \
    modules/73-03-07-04-scene_rendering/chipgraphicitem.h \
    "C:/Program Files/EZTwain/VC/eztwain.h" \
    "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/include/UniControl.h" \
#    modules/scannerInterface/scannerinterface.h \
    modules/nozzleTools/Chip.h \
    modules/nozzleTools/nozzleoperations.h \
    modules/nozzleTools/nozzleEjectionBehaviour.h \
    modules/nozzleTools/matchingnozzle.h \
    modules/encapsulationCoverage/encapsulationcoverage.h \
    modules/exceptions/EncapsulationExceptions.h \
    modules/encapsulationCoverage/calibrationdialog.h \
    modules/73-03-07-01-camera_grabber/cameragrabber.h \
#    modules/nhaWrapper/nhaWrapper.h \
    modules/nozzleTools/nhmap.h \
    modules/exceptions/myexception.h \
    modules/18-16-04-08-CPPLibs/ControlInfo.h \
    modules/18-16-04-08-CPPLibs/Geometry2D.h \
    modules/18-16-04-08-CPPLibs/LeastSquares.h \
    modules/18-16-04-08-CPPLibs/log.h \
    modules/73-03-07-01-camera_grabber/avtcamera.h \
    modules/73-03-07-01-camera_grabber/camera.h \
    modules/73-03-07-01-camera_grabber/opencvcamera.h \
    modules/73-03-07-03-debug_logger/dlogger.h \
    modules/imageAcquisition/imageviewer.h \
    gui/treeitem.h \
    gui/treemodel.h
FORMS += gui/mainwindow.ui \
    gui/settingsdialog_camera.ui \
#    gui/settingsdialog_encap_mes.ui \
    gui/settingsdialog_mes.ui \
    gui/settingsdialog_spa.ui \
    gui/settingsdialog_focus.ui \
    gui/settingsdialog_ledlighting.ui \
    gui/settingsdialog_motors.ui \
    gui/settingsdialog_acquisition.ui \
    gui/settingsdialog_analysis.ui \
    gui/settingsdialog.ui \
    modules/73-03-07-03-debug_logger/messagelogger.ui \
    gui/dnmapgeneratordialog.ui \
    modules/motorControl/comportdetection.ui \
    modules/encapsulationCoverage/calibrationdialog.ui
win32 {
    DEFINES += _TTY_WIN_
    HEADERS += modules/motorControl/serialLib/win_qextserialport.h
    SOURCES += modules/motorControl/serialLib/win_qextserialport.cpp
    RC_FILE = ../resources/appIcon.rc
    LIBS +=  \
        "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniControl.lib" \
        "C:/Program Files/Allied Vision Technologies/UniversalPackage/UniAPI/Lib/UniTransform.lib" \
        -lsetupapi -lAdvapi32 \
#        -luser32 \
#        -lkernel32 \
#        -lodbc32 \
#        -lgdi32 \
#        -lcomdlg32 \
#        -lwinspool \
#        -lwinmm \
        -lshell32 \
#        -lcomctl32 \
#        -lole32 \
#        -loleaut32 \
#        -luuid \
#        -lrpcrt4
#        "C:/Program Files/EZTwain/Release/Eztwain3.lib"
}
unix {
    HEADERS += modules/motorControl/serialLib/posix_qextserialport.h
    SOURCES += modules/motorControl/serialLib/posix_qextserialport.cpp
    DEFINES += _TTY_POSIX_
}






