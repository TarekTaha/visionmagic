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
#ifndef MESSAGELOGGER_H
#define MESSAGELOGGER_H
#include "ui_messagelogger.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDialog>
#include <QtGlobal>
#include <QTime>
#include <QVariant>
#include <string>
#include <fstream>
#include <sstream>
#include <ios>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include "version.h"
#include "dlogger.h"

class MessageLogger: public QDialog
{
    Q_OBJECT
public:
    MessageLogger(QWidget *parent = 0);
    ~MessageLogger();
public slots:
    void clearMessages();
    void saveClicked();
    bool saved();
    void closed();
    void saveMessages(bool exitApp);
    void loadLogs();
private:
    Ui::MessageLoggerClass loggerUI;
    bool  textChanged, qdebug, saveBeforeDelete;
    QFile *file;
};

#endif // MESSAGELOGGER_H
