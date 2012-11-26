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
#ifndef MYEXCEPTION_H
#define MYEXCEPTION_H

#include <QString>
#include <QCoreApplication>
#include <QtGlobal>

/*****************************************************************************
    Exception base class
******************************************************************************/
class MyException
{
public:
    MyException( const QString& msg ) : m_what( msg ) { }
    QString what() const
    {
        return m_what;
    }

    QString tr_what() const
    {
        return qPrintable(m_what);
    }
private:
    QString m_what;
};
/*****************************************************************************
    Map Format Exceptions
******************************************************************************/
class MapFormatException : public MyException
{
public:
    MapFormatException(const QString& msg) : MyException(msg) { }
};
/*****************************************************************************
    Contour Blob Exceptions
******************************************************************************/
class ContourBlobException : public MyException
{
public:
    ContourBlobException(const QString& msg) : MyException(msg) { }
};
/*****************************************************************************
    Incorrect indexing Exceptions
******************************************************************************/
class IndexingException : public MyException
{
public:
    IndexingException(const QString& msg) : MyException(msg) { }
};
/*****************************************************************************
    Null Pointer Exceptions
******************************************************************************/
class NullPointerException : public MyException
{
public:
    NullPointerException(const QString& msg) : MyException(msg) { }
};
#endif // MyException_H
