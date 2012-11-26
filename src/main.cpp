/**************************************************************************
*                         Vision Magic                                    *
*   Copyright (C) 2012 by:                                                *
*      Tarek Taha  <tarek@tarektaha.com>                                  *
*                                                                         *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not,If not, see                           *
*    <http://www.gnu.org/licenses/>.                                      *
***************************************************************************/

#include <qapplication.h>
#include "mainwindow.h"
#include <float.h>
#include "splashscreen.h"

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QString appName;

    QPixmap pixmap(":/splash_screen2");
    SplashScreen splashScreen(pixmap);
    splashScreen.setMessage();
    splashScreen.show();

    MainWindow main_win(&splashScreen);
    appName = QString("%1 v%2").arg(APPLICATION_NAME).arg(APP_VERSION_LONG);
    main_win.setWindowTitle(appName);
    a.setApplicationName("ASG-PIT");

    a.setWindowIcon(QIcon(":/appIcon.ico"));
    main_win.showMaximized();
    splashScreen.finish(&main_win);
    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    return a.exec();
}
