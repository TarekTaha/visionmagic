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
#ifndef LINEFIT_H
#define LINEFIT_H
#include "nozzle.h"
#include "line.h"
#include "math.h"
#include "mathfun.h"
#include "matchingnozzle.h"

class LineFit
{
public:
    LineFit();
    LineFit(QVector <QPointF> points);
    double getAngle();
    Line   getFittedLine();
    double getIntercept();
    double getRMSE();
    double getSlope();
    double getY(double x);
    double getX(double y);
    double distance2Point(QPointF point);
    double distance2Point(QPoint point);
    Line fitPoints(QVector <QPointF> points);
    Line fitPoints(QVector <Nozzle> row);
    Line fitPoints(QVector <MatchingNozzle> row);
    Line fitPointsIgnoringOutliners(QVector<QPointF> points,double acceptableRMSE);
    Line fitPointsIgnoringButtomPoint(QVector<QPointF> points,double acceptableRMSE);
private:
    /*! Slope tan of the angle with Y-axis*/
    double slope;
    /*! Intercept with the Y-Axis*/
    double intercept;
    double sx,sy,stt,sts;
    Line fittedLine;
    double correlation;
    /*! Root Mean Square Error*/
    double rmse;
    QPointF p1,p2;
};

#endif // LINEFIT_H
