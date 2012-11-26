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
#include "linefit.h"

 LineFit::LineFit()
{
}

LineFit::LineFit(QVector <QPointF> points)
{
    fitPoints(points);
}

double LineFit::distance2Point(QPointF point)
{
    return this->fittedLine.distance2Point(point);
}

double LineFit::distance2Point(QPoint point)
{
    return this->fittedLine.distance2Point(point);
}

// In radians
double LineFit::getAngle()
{
    return atan(slope);
}

Line LineFit::getFittedLine()
{
    return this->fittedLine;
}

double LineFit::getIntercept()
{
    return intercept;
}

double LineFit::getRMSE()
{
    return rmse;
}

double LineFit::getSlope()
{
    return slope;
}

double LineFit::getY(double x)
{
    return (intercept + slope*x);
}

double LineFit::getX(double y)
{
    if(slope!=0)
        return (y - intercept)/slope;
    else
        return -1;
}

Line LineFit::fitPoints(QVector <QPointF> points)
{
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0, syy=0.0, t = 0.0, r = 0.0;
    int n = points.size();
    if(n>1)
    {
        p1.setX(points[0].x());
        p1.setY(points[0].y());
        p2.setX(points[n-1].x());
        p2.setY(points[n-1].y());
    }
    for (int i = 0; i < n; ++i)
    {
        sx += points[i].x();
        sy += points[i].y();
    }
    for (int i = 0; i < n; ++i)
    {
        t = points[i].x() - sx/double(n);
        r = points[i].y() - sy/double(n);
        sxx += t*t;
        syy += r*r;
        sxy += t*r;
    }
    slope = sxy/sxx;
    correlation = sxy*sxy/(sxx*syy);
    intercept = (sy - sx*slope)/double(n);
    fittedLine.setSlope(slope);
    fittedLine.setIntercept(intercept);
    rmse =0;
    double diff;
    for (int i = 0; i < n; ++i)
    {
        diff = (fittedLine.getY(points[i].x()) -(points[i].y()));
        rmse +=(diff*diff);
    }
    rmse = sqrt(rmse/double(n));
    return fittedLine;
}

Line LineFit::fitPoints(QVector <Nozzle> row)
{
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0, syy=0.0, t = 0.0, r = 0.0;
    int n = row.size();
    if(n>1)
    {
        p1.setX(row[0].location.x());
        p1.setY(row[0].location.y());
        p2.setX(row[n-1].location.x());
        p2.setY(row[n-1].location.y());
    }
    for (int i = 0; i < n; ++i)
    {
        sx += row[i].location.x();
        sy += row[i].location.y();
    }
    for (int i = 0; i < n; ++i)
    {
        t = row[i].location.x() - sx/double(n);
        r = row[i].location.y() - sy/double(n);
        sxx += t*t;
        syy += r*r;
        sxy += t*r;
    }
    correlation = sxy*sxy/(sxx*syy);
    slope = sxy/sxx;
    intercept = (sy - sx*slope)/double(n);
    rmse =0;
    double diff;
    for (int i = 0; i < n; ++i)
    {
        diff = (fittedLine.getY(row[i].location.x()) -(row[i].location.y()));
        rmse +=(diff*diff);
    }
    rmse = sqrt(rmse/double(n));
    return Line(slope,intercept);
}

Line LineFit::fitPoints(QVector <MatchingNozzle> row)
{
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0, syy=0.0, t = 0.0, r = 0.0;
    int n = row.size();
    if(n>1)
    {
        p1.setX(row[0].location.x());
        p1.setY(row[0].location.y());
        p2.setX(row[n-1].location.x());
        p2.setY(row[n-1].location.y());
    }
    for (int i = 0; i < n; ++i)
    {
        sx += row[i].location.x();
        sy += row[i].location.y();
    }
    for (int i = 0; i < n; ++i)
    {
        t = row[i].location.x() - sx/double(n);
        r = row[i].location.y() - sy/double(n);
        sxx += t*t;
        syy += r*r;
        sxy += t*r;
    }
    correlation = sxy*sxy/(sxx*syy);
    slope = sxy/sxx;
    intercept = (sy - sx*slope)/double(n);
    rmse =0;
    double diff;
    for (int i = 0; i < n; ++i)
    {
        diff = (fittedLine.getY(row[i].location.x()) -(row[i].location.y()));
        rmse +=(diff*diff);
    }
    rmse = sqrt(rmse/double(n));
    return Line(slope,intercept);
}

Line LineFit::fitPointsIgnoringOutliners(QVector<QPointF> points,double acceptableRMSE)
{
    Line fittedReferenceLine = fitPoints(points);
    while(getRMSE()>acceptableRMSE)
    {
        //LineFit fittedReferenceLine(centers);
        int maxIndex=0;
        double maxDist=0;
        for(int t=(points.size()-1);t>=0;t--)
        {
            double dist = fittedReferenceLine.distance2Point(points[t]);
            if(dist>maxDist)
            {
                maxIndex = t;
                maxDist = dist;
            }
        }
        points.remove(maxIndex);
        fitPoints(points);
    }
    return fittedReferenceLine;
}

Line LineFit::fitPointsIgnoringButtomPoint(QVector<QPointF> points,double acceptableRMSE)
{
    Line fittedReferenceLine = fitPoints(points);
    while(getRMSE()>acceptableRMSE)
    {
        //LineFit fittedReferenceLine(centers);
        int maxIndex=0;
        double maxDist=0;
        for(int t=(points.size()-1);t>=0;t--)
        {
            double dist = points[t].y();
            if(dist>maxDist)
            {
                maxIndex = t;
                maxDist = dist;
            }
        }
        points.remove(maxIndex);
        fitPoints(points);
    }
    return fittedReferenceLine;
}
