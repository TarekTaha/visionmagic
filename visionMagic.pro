TEMPLATE = subdirs

# build as ordered or build will fail
CONFIG -= build_all
CONFIG += ordered

!build_pass:message(+++++++++++ Qt Details ++++++++++++++)
!build_pass:message(Qt version: $$[QT_VERSION])
!build_pass:message(qmake version: $$[QMAKE_VERSION])
!build_pass:message(Qt is installed in $$[QT_INSTALL_PREFIX])
!build_pass:message(Qt resources can be found in the following locations:)
!build_pass:message(Documentation: $$[QT_INSTALL_DOCS])
!build_pass:message(Header files: $$[QT_INSTALL_HEADERS])
!build_pass:message(Libraries: $$[QT_INSTALL_LIBS])
!build_pass:message(Binary files (executables): $$[QT_INSTALL_BINS])
!build_pass:message(Plugins: $$[QT_INSTALL_PLUGINS])
!build_pass:message(Data files: $$[QT_INSTALL_DATA])
!build_pass:message(Translation files: $$[QT_INSTALL_TRANSLATIONS])
!build_pass:message(Settings: $$[QT_INSTALL_SETTINGS])
!build_pass:message(Examples: $$[QT_INSTALL_EXAMPLES])
!build_pass:message(Demonstrations: $$[QT_INSTALL_DEMOS])
!build_pass:message(++++++++++++++++++++++++++++++++++++)
# saves the location of Qt used to compile the project
# so that we use that location to link the DLLs
system("echo $$[QT_INSTALL_PREFIX]> qtLocation")

!build_pass {
    CONFIG(debug, debug|release) {
        message(|======================================|)
        message(|=============Debug Build==============|)
        message(|======================================|)
    }
    CONFIG(release, debug|release) {
        message(|======================================|)
        message(|===========Release Build==============|)
        message(|======================================|)
   }
}

SUBDIRS = src/modules/shared \
          src/modules/tiffIOLib \
          src/modules/qwt \
          src
