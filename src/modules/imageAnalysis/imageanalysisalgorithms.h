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
#ifndef IMAGEANALYSISALGORITHMS_H
#define IMAGEANALYSISALGORITHMS_H
#include <QVector>
#include <QTime>
#include <QWidget>
#include <QDir>
#include <QtAlgorithms>
#include <QTextStream>
#include <QPoint>
#include <QMutexLocker>
#include <QTimer>

#include "cv.h"
#include "highgui.h"

#include "dlogger.h"

#ifdef WIN32
    #include <windows.h>
    #define usleep(x) Sleep((x)/1000)
#else
    #include <unistd.h>
#endif

namespace ImageAnalysisAlgorithms
{
    double angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 );
    void histogramFiltering();
    void rotate(IplImage* img,float angle, float centreX, float centreY);
    IplImage* rotate(IplImage* img,float angle);
    void filterImage(IplImage *srcImage);
    void sumRGB( IplImage* src, IplImage* dst );
    IplImage * templateMatch(IplImage* analysisImage, IplImage* nozzelTemplate);
    IplImage * templateMatch(IplImage* analysisImage, IplImage* nozzelTemplate, int method);
    using std::vector;
    /*!
      FastMatchTemplate
      Performs a fast match template
      Returns: true on success, false on failure
      Parameters:
        source - source image (where we are searching)
        target - target image (what we are searching for)
        foundPointsList - contains a list of the points where the target was found
        confidencesList - contains a list of the confidence value (0-100) for each
                          found target
        matchPercentage - the minimum required match score to consider the target
                          found
        findMultipleTargets - if set to true, the function will attempt to find a
                              maximum of numMaxima targets
        numMaxima - the maximum number of search locations to try before exiting
                    (i.e. when image is down-sampled and searched, we collect the
                    best numMaxima locations - those with the highest confidence -
                    and search the original image at these locations)
        numDownPyrs - the number of times to down-sample the image (only increase
                      this number if your images are really large)
        searchExpansion - The original source image is searched at the top locations
                          with +/- searchExpansion pixels in both the x and y
                          directions
    */
    bool    FastMatchTemplate( const IplImage&  source,
                               const IplImage&  target,
                               vector<CvPoint>* foundPointsList,
                               vector<double>*  confidencesList,
                               int              matchPercentage = 70,
                               bool             findMultipleTargets = true,
                               int              numMaxima = 5,
                               int              numDownPyrs = 2,
                               int              searchExpansion = 15 );

    /*!
      MultipleMaxLoc
      Searches an image for multiple maxima
      Assumes a single channel, floating point image
      Parameters:
        image - the input image, generally the result from a cvMatchTemplate call
        locations - array of CvPoint (pass in a NULL point)
        numMaxima - the maximum number of best match maxima to locate
    */
    void    MultipleMaxLoc( const IplImage& image,
                            CvPoint**       locations,
                            int             numMaxima );

    /*!
      DrawFoundTargets
      Draws a rectangle of dimension size, at the given positions in the list,
      in the given RGB color space
      Parameters:
        image - a color image to draw on
        size - the size of the rectangle to draw
        pointsList - a list of points where a rectangle should be drawn
        confidencesList - a list of the confidences associated with the points
        red - the red value (0-255)
        green - the green value (0-255)
        blue - the blue value (0-255)
    */
    void    DrawFoundTargets( IplImage*              image,
                              const CvSize&          size,
                              const vector<CvPoint>& pointsList,
                              const vector<double>&  confidencesList,
                              int                    red   = 0,
                              int                    green = 255,
                              int                    blue  = 0 );
    void addNoise2Image(IplImage* image,
                        int numNoisyPixels,
                        int noiseIntensityAverage,
                        int intensityDeviation);
    void drawBox(IplImage *image, CvBox2D box, CvScalar color);
    /*!
      Calculate the area of a polygon represented by a sequence of points
      */
    double calculateContourArea( const CvSeq* contour);
    /*!
      Here we calculate the centroid and area from the gray scale image.
      The reason why they are combined is mainly due to speed optimization.
      The min intensity of the image patch is considered the normalizing factor and is used to
      get the area. The area of this polygon is treated as a "non-homogeneous" 2d-shape and the moment of mass
      formula is used to get the accurate centroid.
      */
    void getWeightedAreaCentroid(IplImage *image,CvRect rect,double&area,CvPoint2D64f &centroid,int buffer=0, int backgroundColorIntensity=255);
    /*!
      This function calculates a weighted area average of a bounding rectangle from a
      single channel image.
      */
    double calculateWeightedArea(IplImage *image,CvRect rect,int buffer=0,int backgroundColorIntensity=255);
    /*!
      */
    double calculateUpSampledContourArea(IplImage *image,CvRect rect,double &x, double &y, int buffer=0);
    /*!
      Takes an RGB image and returns a single channel image where I(x,y)= min[x,y](R,G,B)
      */
    IplImage* minRGB(IplImage* src);
}
#endif // IMAGEANALYSISALGORITHMS_H
