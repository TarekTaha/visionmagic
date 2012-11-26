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
#ifndef MATHFUN_H
#define MATHFUN_H
#include <cmath>
#include <QPointF>
#include <iostream>
#define THIRD 0.333333333333333
#define ROOTTHREE 1.73205080756888

#if  defined(_MSC_VER)
    inline long lround(double d)
    {
        return (long)(d>0 ? d+0.5: ceil(d-0.5));
    }
    #define LROUND(A) lround(A)
#else
    #define LROUND(A) lrint(A)
#endif

#define PRECISION 100000.0
#define EQ(A,B) ((LROUND(A*PRECISION))==(LROUND(B*PRECISION)))

#define LT(A,B) ((LROUND(A*PRECISION))<(LROUND(B*PRECISION)))
#define GT(A,B) ((LROUND(A*PRECISION))>(LROUND(B*PRECISION)))
#define GTE(A,B) ((LROUND(A*PRECISION))>=(LROUND(B*PRECISION)))

#define LTE(A,B) ((LROUND(A*PRECISION))<=(LROUND(B*PRECISION)))

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#define MILLION 1e6
#define BILLION 1e9

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#ifndef TWOPI
    #define TWOPI (2.0*M_PI)
#endif

#ifndef RTOD
    #define RTOD(r) ((r) * 180.0 / M_PI)
#endif

#ifndef DTOR
    #define DTOR(d) ((d) * M_PI / 180.0)
#endif

#ifndef NORMALIZE
    #define NORMALIZE(z) atan2(sin(z), cos(z))
#endif

using namespace std;

template <class T>
inline bool inRange(T number, T range1,T range2)
{
    T upperRange,lowerRange;
    if(range1>range2)
    {
        upperRange = range1;
        lowerRange = range2;
    }
    else
    {
        upperRange = range2;
        lowerRange = range1;
    }
    if (number>=lowerRange && number<=upperRange)
        return true;
    else
        return false;
}

template <class T>
inline T limit2Boundaries(T number, T range1,T range2)
{
    T upperRange,lowerRange;
    if(range1>range2)
    {
        upperRange = range1;
        lowerRange = range2;
    }
    else
    {
        upperRange = range2;
        lowerRange = range1;
    }
    if (number<lowerRange)
        return lowerRange;
    else if(number>upperRange)
        return upperRange;
    else
        return number;
}

inline double getAngle(const QPoint &p1,const QPoint &p2)
{
    double angle = atan((p2.y()-p1.y())/float((p2.x()-p1.x())));
    return angle;
}

inline double getAngle(QPointF p1, QPointF p2, QPointF s1,QPointF s2)
{
    double a = p2.x() - p1.x();
    double b = p2.y() - p1.y();
    double c = s2.x() - s1.x();
    double d = s2.y() - s1.y();
    return acos(((a*c) + (b*d))/(sqrt(a*a+b*b)*sqrt(c*c+d*d)));
}

inline double eucDistance(const QPoint &p1,const QPoint &p2)
{
    return sqrt(pow(p1.x() - p2.x(),2.0)+pow(p1.y() -p2.y(),2.0));
}

inline double eucDistance(const QPointF &p1,const QPointF &p2)
{
    return sqrt(pow(p1.x() - p2.x(),2)+pow(p1.y() -p2.y(),2));
}

inline double ellipseEccentricity(int axis1,int axis2)
{
    double semiMinor, semiMajor;
    if(axis1>axis2)
    {
        semiMajor = axis1;
        semiMinor = axis2;
    }
    else
    {
        semiMajor = axis2;
        semiMinor = axis1;
    }

    return sqrt(semiMajor*semiMajor - semiMinor*semiMinor)/semiMajor;
}

inline double roundness(double perimeter,double area)
{
    return (4*M_PI*area)/(perimeter*perimeter);
}

// this function returns the cube root if x were a negative number aswell
inline long double cubeRoot(long double x)
{
    if (x < 0)
        return -pow(-x, long double(1.0/3.0));
    else
        return  pow( x, long double(1.0/3.0));
}

#endif // MATHFUN_H
