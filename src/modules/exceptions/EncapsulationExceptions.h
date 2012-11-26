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
#ifndef ENCAPSULATIONEXCEPTIONS_H
#define ENCAPSULATIONEXCEPTIONS_H

#include <QString>
#include <QCoreApplication>
#include <QtGlobal>

/*****************************************************************************
    Exception base class
******************************************************************************/
class EncapException
{
public:
    EncapException( const QString& msg ) : m_what( msg ) { }
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
    ReferenceLine Exceptions
******************************************************************************/
class ReferenceLineException : public EncapException
{
public:
    ReferenceLineException(const QString& msg) : EncapException(msg) { }
};
#endif //ENCAPSULATIONEXCEPTIONS_H
