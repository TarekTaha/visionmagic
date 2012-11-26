echo Setting up a MinGW/Qt only environment...
echo -- QTDIR set to C:\qt\2010.05\qt
echo -- PATH set to C:\qt\2010.05\qt\bin
echo -- Adding C:\qt\2010.05\bin to PATH
echo -- Adding %SystemRoot%\System32 to PATH
echo -- QMAKESPEC set to win32-g++
set QTDIR=C:\qt\2010.05\qt
set PATH=C:\qt\2010.05\qt\bin
set PATH=%PATH%;C:\qt\2010.05\bin;C:\qt\2010.05\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

set

g++ --version
echo Run qmake
qmake.exe ISG_ACD.pro -spec win32-g++ -recursive CONFIG+=%1
echo Run make
mingw32-make.exe -w %2

exit
