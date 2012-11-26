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
#include "line.h"

Line::Line(double slope,double intercept):
        x(0),
        X(0),
        Y(0)
{
    if(slope == 0)
    {
        this->orientation = HORIZONTAL;
        Y  = intercept;
    }
    this->a = slope;
    this->b = intercept;
}

Line::Line(QPointF p1, double slope):
        x(0),
        X(0),
        Y(0)
{
    if(slope == 0)
    {
        this->orientation = HORIZONTAL;
        Y = p1.y();
    }
    this->p1 = p1;
    this->a = slope;
    b = p1.y() - a*p1.x();
}

Line::Line(QPointF p1,QPointF p2):
        X(0),
        Y(0)
{
    this->p1 = p1;
    this->p2 = p2;
    if((p1.x() - p2.x())==0)
    {
        this->orientation = VERTICAL;
        X = p1.x();
        // just a special identifier
        a = -1000;
        x = p1.x();
    }
    else
        a = (p1.y() - p2.y())/(p1.x() - p2.x());
    b = p1.y() - a*p1.x();
    if(p1.y() == p2.y())
    {
        this->orientation = HORIZONTAL;
        Y = p1.y();
    }
}

double Line::distance2Point(QPoint point)
{
    return distance2Point(QPointF(point.x(),point.y()));
}

double Line::distance2Point(QPointF point)
{
    double distance;
    if(orientation == HORIZONTAL) // slope a = 0
    {
        distance = fabs(point.y() - getY(point.x()));
    }
    else if(orientation == VERTICAL) // slope a is undefined
    {
        distance = fabs(point.x() - getY(point.y()));
    }
    else
    {
        double intersectSlope = -1/a; // Lines are perpendiculare -> slope product = -1;
        Line perpLine(point,intersectSlope);
        QPointF pointOfIntersection = lineIntersectLine(perpLine);
        if(pointOfIntersection.x()==point.x())
        {
            pointOfIntersection.setY(b);
        }
        distance = sqrt(pow((point.x() - pointOfIntersection.x()),2) + pow((point.y() - pointOfIntersection.y()),2));
    }
    return distance;
}

double Line::getAngle()
{
    return atan(a);
}

double Line::getSlope()
{
    return this->a;
}

QPointF Line::getXaxisIntersectPoint()
{
    // intersect X-axis => y = 0;
    // y = ax + b; y=0 => ax=-b; => x = -b/a;
    double x0 = getX(0);
    return QPointF(x0,0);
}

QPointF Line::getYaxisIntersectPoint()
{
    // intersect Y-axis => x = 0;
    // y = ax + b; x=0 => y=b;
    double y0 = getY(0);
    return QPointF(0,y0);
}

double Line::getX(double _y)
{
    // y=ax+b => x = (y-b)/a;
    //TODO:: throw a proper exception here
    if(orientation == HORIZONTAL) // a = 0
        return 0.0;
    else if(orientation == VERTICAL)
        return X;
    else
        return (_y-b)/a;
}

double Line::getY(double _x)
{
    //TODO:: throw a proper exception here
    if(orientation == VERTICAL)
        return 0.0;
    else
        return (a*_x + b);
}

bool Line::isPointRightofLine(QPointF p)
{
    // vertical Line special case
    if(orientation == VERTICAL)
        return (p.x() > X);
    if(orientation == HORIZONTAL)
        return (p.y() < Y);
    // if y = p.y then get then x:  y = ax + b; ax = p.y - b; => x =(p.y -b)/a;
    double x0 = (p.y() - b)/a; // y = ax + b; y=0 => ax=-b; => x = -b/a;
    // x0 < p.x => the point is right of the line
    return (x0<p.x());
}

bool Line::isPointLeftofLine(QPointF p)
{
    // vertical Line special case
    if(orientation == VERTICAL)
        return (p.x() < p1.x());
    if(orientation == HORIZONTAL)
        return (p.y() > Y);
    // if y = p.y then get then x:  y = ax + b; ax = p.y - b; => x =(p.y -b)/a;
    double x0 = (p.y() - b)/a; // y = ax + b; y=0 => ax=-b; => x = -b/a;
    // x0>p.x => the point is left of the line
    if(x0>p.x())
        return true;
    else
        return false;
}

QPointF Line::lineIntersectLine(Line lineInter)
{
    QPointF intersectionPoint;
    /*! y = a1x + b1; y = a2x + b2
          y=y => x(a1 - a2) = b2-b1; => x = (b2 - b1)/(a1 - a2);
          if(a1 = 1 or a2=1) =>
     */
    //TODO:: throw an exception here
    if((a - lineInter.a)==0) // They are parallel
    {
        intersectionPoint.setX(0);
        intersectionPoint.setY(0);
    }
    else if(orientation == VERTICAL)
    {
        intersectionPoint.setX(X);
        intersectionPoint.setY(lineInter.a*intersectionPoint.x() + lineInter.b);
    }
    else if(orientation == HORIZONTAL)
    {
        intersectionPoint.setY(Y);
        if(lineInter.orientation == VERTICAL)
            intersectionPoint.setX(lineInter.X);
        else
            intersectionPoint.setX((intersectionPoint.y() - lineInter.b)/lineInter.a);
    }
    else
    {
        intersectionPoint.setX((lineInter.b - b)/(a - lineInter.a));
        intersectionPoint.setY(a*intersectionPoint.x() + b);
    }
    return intersectionPoint;
}
// the angle here should be in radians and is only used to find the slope
void Line::setAngle(double angle)
{
    //slope here is zero, we don't really have to do this
    // a zero slope can be handled by the y = ax + b equation
    if(EQ(angle,0) || EQ(angle,M_PI))
    {
        this->a = 0;
        this->orientation = HORIZONTAL;
    }
    // this is the part where the slope will be undefined
    // the straight line equation can't handle this so we
    // add an orientation term to be used later on for calculations
    else if (EQ(fabs(angle),M_PI_2))
    {
        this->orientation = VERTICAL;
        // This slope should not be defined but here I am setting it to zero
        // but it should NEVER be used
        this->a = 0;
    }// slope a here is simply the tan of the rotation angle
    else
    {
        this->a = tan(angle);
        this->orientation = ROTATED;
    }
}

void Line::setReferencePoint(QPointF p1)
{
    this->p1 = p1;
    if(this->orientation == HORIZONTAL)
    {
        Y = p1.y();
    }
    else if (this->orientation == VERTICAL)
    {
        X = p1.x();
    }
    b = p1.y() - a*p1.x();
}

void Line::setSlope(double slope)
{
    this->a = slope;
    if(a == 0)
    {
        this->orientation = HORIZONTAL;
    }
}

void Line::setIntercept(double intercept)
{
    this->b = intercept;
}
