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
#ifndef LINE_H
#define LINE_H
#include <QPointF>
#include <QVector>
#include <cmath>
#include <mathfun.h>
#include "dlogger.h"

#ifndef M_PI
    #define M_PI 3.1415926535897932384626433832795
#endif
#ifndef M_PI_2
    #define M_PI_2 2*3.1415926535897932384626433832795
#endif

class Line
{
public:
    enum
    {
        VERTICAL,
        HORIZONTAL,
        ROTATED
    };
    Line(){this->orientation = ROTATED;}
    Line(double slope,double intercept);
    Line(QPointF p1, double slope);
    Line(QPointF p1,QPointF p2);
    double distance2Point(QPoint point);
    double distance2Point(QPointF point);
    double getAngle();
    double getOrientation();
    double getSlope();
    double getX(double _y);
    QPointF getXaxisIntersectPoint();
    double getY(double _x);
    QPointF getYaxisIntersectPoint();
    bool isPointRightofLine(QPointF p);
    bool isPointLeftofLine(QPointF p);
    QPointF lineIntersectLine(Line lineInter);
    void setAngle(double angle); // in radians
    void setIntercept(double);
    void setOrientation(int);
    void setReferencePoint(QPointF p1);
    void setSlope(double);
    //eq: y = ax + b; a = slope (tag of angles with X-axis), b = const (intercept with Y-Axis)
    double a; // slope (tag of angles with X-axis)
    double b; // const (intercept with Y-Axis)
    double x;
    double orientation;
    QPointF p1,p2;
    double X; // the X value when the line is Vertical
    double Y; // the Y value when the line is Horizontal
};
#endif // LINE_H
