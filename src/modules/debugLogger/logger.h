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
#ifndef LOGGER_H
#define LOGGER_H

#include <QDebug>
#include <QMutex>
#include <QString>
#include <QThread>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <cstdlib>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <istream>
#include <ios>
#include <ctime>
#include "version.h"

using namespace std;
#ifdef WIN32
    #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define FUNCTION_NAME ( std::string( __FUNCTION__ ) )
#define LINE_NUMBER ( __LINE__ )

// Global LOG macro
#define LOG(level, msg)                                                      \
{                                                                            \
    std::ostringstream ss;                                                   \
    ss << msg;                                                               \
    Logger& lg = Logger::getLogger();                                        \
    lg.log( (Logger::Severity)level, ss.str(), FUNCTION_NAME, LINE_NUMBER ); \
}

// Like LOG but with added line break
#define LOGL(level, msg) LOG(level, msg << "\n")

/***************************************************************************
    Extra inserter to handle QStrings.
****************************************************************************/
inline std::ostream& operator<<(std::ostream& os, const QString& qs)
{
    os << qs.toAscii().data();
    return os;
}

#ifndef QT_NO_DEBUG
    #define qDebug() qDebug() <<"["<< QDateTime::currentDateTime().toUTC().toString( "dd/MM/yy-hh:mm:ss" ).toLatin1().data() \
                              << " - " <<QString("%1").arg((int)QThread::currentThreadId(),4).toLatin1().data() \
                              << " - " <<QString("%1(%2) - L4]: ").arg(__PRETTY_FUNCTION__,20).arg(__LINE__,4).toLatin1().data()
#else //DEBUG
    #define qDebug() qDebug() <<"["<< QDateTime::currentDateTime().toUTC().toString( "dd/MM/yy-hh:mm:ss" ).toLatin1().data() \
                              << " - " <<QString("%1").arg((int)QThread::currentThreadId(),4).toLatin1().data() \
                              << " - " <<QString("%1(%2) - L4]: ").arg(__PRETTY_FUNCTION__,20).arg(__LINE__,4).toLatin1().data()
#endif

/***************************************************************************
    Simple logging class
****************************************************************************/
class Logger
{
public:

    enum Severity
    {
        Critical = 1,
        Warning,
        Info,
        Debug
    };

    std::ofstream mFileOut;
    QMutex mMutex;
    QtMsgHandler mDefaultMsgHandler;

    /**********************************************************************
        Ctor
    ***********************************************************************/
    Logger() : mDefaultMsgHandler(NULL), mLevel(Warning) {}

    /**********************************************************************
        Dtor
    ***********************************************************************/
    virtual ~Logger()
    {
        mFileOut.close();
    }

    /**********************************************************************
        Initalises the logger.
        @param[in] sFilename The file to log to.
        @param[in] bOverwrite If true, the file is wiped before starting.
    ***********************************************************************/
    void initialize(QString sFilename, bool bOverwrite = true);

    /**********************************************************************
        Returns the static Logger object.
    ***********************************************************************/
    static Logger& getLogger();

    /**********************************************************************
        Called by LOG macro to do the outputting.
    ***********************************************************************/
    void log(Severity level, std::string message,std::string function,int line );

    /**********************************************************************
        Sets debug level.
    ***********************************************************************/
    void setLevel(Severity level) { mLevel = level; }

    /**********************************************************************
        Gets debug level.
    ***********************************************************************/
    int getLevel() { return mLevel; }

    /**********************************************************************
        Gets current logfile path.
    ***********************************************************************/
    QString getFilePath() const { return mFilePath; }

    /**********************************************************************
        Returns formatted time string.
    ***********************************************************************/
    static std::string getTime()
    {
        time_t now;
        time(&now);
        struct tm* tmnow;
        tmnow = gmtime(&now);
        //gmtime_s(tmnow,&now);
        char acTmp[128];
        strftime(acTmp, 127, "%d/%m/%Y %H:%M:%S", tmnow);
        std::string sTime(acTmp);
        return sTime;
    }
    /***********************************************************************
        Used to direct qDebug() output directly to the log, it formats itself
        correctly, and this is a temporary HACK until we redo this stuff
    ************************************************************************/
    void JustOutputThisHack( const char* msg );
private:
    Severity mLevel;
    QString mFilePath;
};


#endif // LOGGER_H
