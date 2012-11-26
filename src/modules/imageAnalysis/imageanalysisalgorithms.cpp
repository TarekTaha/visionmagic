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
#include "imageanalysisalgorithms.h"

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
namespace ImageAnalysisAlgorithms
{

    double angle( CvPoint* pt1, CvPoint* pt2, CvPoint* pt0 )
    {
        double dx1 = pt1->x - pt0->x;
        double dy1 = pt1->y - pt0->y;
        double dx2 = pt2->x - pt0->x;
        double dy2 = pt2->y - pt0->y;
        return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
    }

    void filterImage(IplImage *srcImage)
    {
        CvSize sz = cvSize( srcImage->width & -2, srcImage->height & -2 );
        IplImage* pyr = cvCreateImage( cvSize(sz.width/2, sz.height/2), 8, 3 );

        // select the maximum ROI in the image
        // with the width and height divisible by 2
        cvSetImageROI( srcImage, cvRect( 0, 0, sz.width, sz.height ));

        qDebug() << "Image width"<<pyr->width <<" height"<<pyr->height;
        // down-scale and upscale the image to filter out the noise
        cvPyrDown(srcImage, pyr, 7 );
        cvPyrUp(pyr, srcImage, 7 );
        cvReleaseImage(&pyr);
    }

    void histogramFiltering()
    {
        int _brightness = 100;
        int _contrast = 100;
        int hist_size = 256;
        float range_0[]={0,256};
        float* ranges[] = { range_0 };
        IplImage *src_image = 0, *dst_image = 0, *hist_image = 0;
        CvHistogram *hist;
        uchar lut[256];
        CvMat* lut_mat;

        src_image = cvLoadImage("C:\\Documents and Settings\\ttaha\\Desktop\\Fast Dead Nozzle Mapping\\Image0028_grey.TIF", 0 );
        if( !src_image )
        {
            qDebug() << "Image was not loaded.\n";
            return;
        }
        cvNamedWindow("image", 0);
        cvNamedWindow("histogram", 0);

        dst_image = cvCloneImage(src_image);
        hist_image = cvCreateImage(cvSize(256,300), 8, 1);
        hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 3);
        lut_mat = cvCreateMatHeader( 1, 256, CV_8UC1 );
        cvSetData( lut_mat, lut, 0 );
        int brightness = _brightness - 100;
        int contrast = _contrast - 100;
        int i, bin_w;
        float max_value = 0;
        /*
         * The algorithm is by Werner D. Streidt
         * (http://visca.com/ffactory/archives/5-99/msg00021.html)
         */
        if( contrast > 0 )
        {
            double delta = 127.*contrast/100;
            double a = 255./(255. - delta*2);
            double b = a*(brightness - delta);
            for( i = 0; i < 256; i++ )
            {
                int v = cvRound(a*i + b);
                if( v < 0 )
                    v = 0;
                if( v > 255 )
                    v = 255;
                lut[i] = (uchar)v;
            }
        }
        else
        {
            double delta = -128.*contrast/100;
            double a = (256.-delta*2)/255.;
            double b = a*brightness + delta;
            for( i = 0; i < 256; i++ )
            {
                int v = cvRound(a*i + b);
                if( v < 0 )
                    v = 0;
                if( v > 255 )
                    v = 255;
                lut[i] = (uchar)v;
            }
        }

    //    cvLUT( src_image, dst_image, lut_mat );
        cvThreshold(src_image, dst_image, 225 ,255, CV_THRESH_BINARY_INV);
    //    cvAdaptiveThreshold(src_image,dst_image,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY,21,3);
        cvShowImage( "image", dst_image );
        cvSaveImage("histSrc.png",dst_image);

        cvCalcHist( &dst_image, hist, 0, NULL );
        cvZero( dst_image );
        cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0 );
        cvScale( hist->bins, hist->bins, ((double)hist_image->height)/max_value, 0 );
        /*cvNormalizeHist( hist, 1000 );*/

        cvSet( hist_image, cvScalarAll(255), 0 );
        bin_w = cvRound((double)hist_image->width/hist_size);

        for( i = 0; i < hist_size; i++ )
            cvRectangle( hist_image, cvPoint(i*bin_w, hist_image->height),
                         cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i))),
                         cvScalarAll(0), -1, 8, 0 );

        cvShowImage( "histogram", hist_image );
        cvSaveImage("hist.png",hist_image);

        cvReleaseImage(&src_image);
        cvReleaseImage(&dst_image);
        cvReleaseHist(&hist);
    }

    void rotate(IplImage* img,float angle, float centreX, float centreY)
    {
       qDebug() << "Rotating Image";
       CvPoint2D32f centre;
       IplImage* dst = cvCloneImage(img);
       CvMat *translate = cvCreateMat(2, 3, CV_32FC1);
       cvSetZero(translate);
       centre.x = centreX;
       centre.y = centreY;
       cv2DRotationMatrix(centre, angle, 1.0, translate);
       cvWarpAffine(img, dst, translate, CV_INTER_LINEAR + CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
       cvReleaseMat(&translate);
       cvCopy(dst,img);
       cvReleaseImage(&dst);
    }

    IplImage * rotate(IplImage *src,float degree)
    {
        qDebug() << "Rotating Image by:"<<degree;
        double angle = degree * CV_PI / 180.; // angle in radian
        double a = sin(angle), b = cos(angle); // sine and cosine of angle

        // dimensions of src, dst and actual needed size
        int w_src = src->width, h_src = src->height;
        int w_rot = 0, h_rot = 0; // actual needed size

        // scale factor for rotation
        double scale_w = 0., scale_h = 0., scale = 0.;

        // map matrix for WarpAffine, stored in array
        double map[6];
        CvMat map_matrix = cvMat(2, 3, CV_64FC1, map);

        // Rotation center needed for cv2DRotationMatrix
        CvPoint2D32f pt = cvPoint2D32f(w_src / 2, h_src / 2);

        // Make w_rot and h_rot according to phase
        w_rot = (int)(h_src * fabs(a) + w_src * fabs(b));
        h_rot = (int)(w_src * fabs(a) + h_src * fabs(b));

        // Create the destination Image
        IplImage*  dst = cvCreateImage(cvSize(w_rot,h_rot),8,src->nChannels);
        int w_dst = dst->width, h_dst = dst->height;

        scale_w = (double)w_dst / (double)w_rot;
        scale_h = (double)h_dst / (double)h_rot;
        scale = MAX(scale_w, scale_h);
        scale = 1.0;
        cv2DRotationMatrix(pt, degree, scale, &map_matrix);

        // Adjust rotation center to dst's center
        map[2] += (w_dst - w_src) / 2;
        map[5] += (h_dst - h_src) / 2;

        cvWarpAffine(
                src,
                dst,
                &map_matrix,
                CV_INTER_LINEAR | CV_WARP_FILL_OUTLIERS,
                cvScalarAll(0)
                );
        qDebug() <<"Image Rotated";
        return dst;
    }

    void sumRGB( IplImage* src, IplImage* dst )
    {
        // Allocate individual image planes.
        IplImage* r = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
        IplImage* g = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );
        IplImage* b = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );

        // Split image onto the color planes.
        cvSplit( src, r, g, b, NULL );

        // Temporary storage.
        IplImage* s = cvCreateImage( cvGetSize(src), IPL_DEPTH_8U, 1 );

        // Add equally weighted rgb values.
        cvAddWeighted( r, 1./3., g, 1./3., 0.0, s );
        cvAddWeighted( s, 2./3., b, 1./3., 0.0, s );

        cvCopy(s,dst);
        // Truncate values above 100.
    //    cvThreshold( s, dst, 100, 100, CV_THRESH_TRUNC );
    //    cvThreshold(s, dst,120,255, CV_THRESH_BINARY);
        cvReleaseImage( &r );
        cvReleaseImage( &g );
        cvReleaseImage( &b );
        cvReleaseImage( &s );
    }

    IplImage * templateMatch(IplImage* analysisImage, IplImage* nozzelTemplate, int method)
    {
        CvSize resultSize;
        resultSize.height= analysisImage->height - nozzelTemplate->height + 1;
        resultSize.width = analysisImage->width  - nozzelTemplate->width + 1;
        //This will hold the result of the Match
        IplImage* result = cvCreateImage( resultSize,IPL_DEPTH_32F,1);
        /*!METHODS can be :  CV_TM_SQDIFF
                                 CV_TM_SQDIFF_NORMED
                                 CV_TM_CCORR
                                 CV_TM_CCORR_NORMED
                                 CV_TM_CCOEFF
                                 CV_TM_CCOEFF_NORMED
                                 */
        cvMatchTemplate( analysisImage, nozzelTemplate, result, method);
        return result;
    }

    IplImage * templateMatch(IplImage* analysisImage, IplImage* nozzelTemplate)
    {
        return templateMatch(analysisImage,nozzelTemplate,CV_TM_CCOEFF_NORMED);
    }

    bool multipleTemplateMatch(const IplImage *  analysisImage,
                       const IplImage *  nozzelTemplate,
                       vector<CvPoint>* foundPointsList,
                       vector<double>*  confidencesList,
                       int              matchPercentage,
                       bool             findMultipleTargets,
                       int              numMaxima,
                       int              matchingMethod)
    {
        CvSize resultSize;
        resultSize.height= analysisImage->height - nozzelTemplate->height + 1;
        resultSize.width = analysisImage->width  - nozzelTemplate->width + 1;
        //This will hold the result of the Match
        IplImage* result = cvCreateImage( resultSize,IPL_DEPTH_32F,1);
        /*!METHODS can be :  CV_TM_SQDIFF
                                 CV_TM_SQDIFF_NORMED
                                 CV_TM_CCORR
                                 CV_TM_CCORR_NORMED
                                 CV_TM_CCOEFF
                                 CV_TM_CCOEFF_NORMED
                                 */
        cvMatchTemplate( analysisImage, nozzelTemplate, result, matchingMethod);
        foundPointsList->clear();
        confidencesList->clear();
        // find the top match locations
        CvPoint* locations = NULL;
        MultipleMaxLoc( *result, &locations, numMaxima );
        for( int currMax = 0; currMax < numMaxima; currMax++ )
        {
            locations[currMax].x += nozzelTemplate->width  / 2;
            locations[currMax].y += nozzelTemplate->height / 2;
            int i = locations[currMax].x;
            int j = locations[currMax].y;
            double value = (uchar)(result->imageData[j*result->widthStep + i]);
            value*=100;
            if( value >= matchPercentage)
            {
                foundPointsList->push_back( locations[currMax] );
                confidencesList->push_back( value );
                if( !findMultipleTargets )
                {
                    break;
                }
            }
        }
        cvReleaseImage( &result );
        delete [] locations;
        return (foundPointsList->size()>0);
    }

    template<class T> class Image
    {
    private:
        IplImage* imgp;
    public:
        Image(IplImage* img=0) {imgp=img;}
        ~Image(){imgp=0;}
        void operator =(IplImage* img) {imgp=img;}
        inline T* operator[](const int rowIndx)
        {
            return ((T *)(imgp->imageData + rowIndx*imgp->widthStep));
        }
    };

    typedef struct
    {
        unsigned char b,g,r;
    } RgbPixel;

    typedef struct
    {
        float b,g,r;
    } RgbPixelFloat;

    typedef Image<RgbPixel>       RgbImage;
    typedef Image<RgbPixelFloat>  RgbImageFloat;
    typedef Image<unsigned char>  BwImage;
    typedef Image<float>          BwImageFloat;

    //=============================================================================
    // Assumes that source image exists and numDownPyrs > 1, no ROIs for either
    //  image, and both images have the same depth and number of channels
    bool
            FastMatchTemplate( const IplImage&  source,
                               const IplImage&  target,
                               vector<CvPoint>* foundPointsList,
                               vector<double>*  confidencesList,
                               int              matchPercentage,
                               bool             findMultipleTargets,
                               int              numMaxima,
                               int              numDownPyrs,
                               int              searchExpansion )
    {
        // make sure that the template image is smaller than the source
        if( target.width > source.width || target.height > source.height )
        {
            printf( "\nSource image must be larger than target image.\n" );
            return false;
        }

        if( source.depth != target.depth )
        {
            printf( "\nSource image and target image must have same depth.\n" );
            return false;
        }

        if( source.nChannels != target.nChannels )
        {
            printf(
                    "\nSource image and target image must have same number of channels.\n" );
            return false;
        }

        CvSize sourceSize = cvGetSize( &source );
        CvSize targetSize = cvGetSize( &target );

        int depth = source.depth;
        int numChannels = source.nChannels;

        // create copies of the images to modify
        IplImage* copyOfSource = cvCloneImage( &source );
        IplImage* copyOfTarget = cvCloneImage( &target );

        // down pyramid the images
        for( int ii = 0; ii < numDownPyrs; ii++ )
        {
            // start with the source image
            sourceSize.width /= 2;
            sourceSize.height /= 2;

            IplImage* smallSource = NULL;
            smallSource = cvCreateImage( sourceSize, depth, numChannels );
            cvPyrDown( copyOfSource, smallSource, CV_GAUSSIAN_5x5 );

            // prepare for next loop, if any
            cvReleaseImage( &copyOfSource );
            copyOfSource = cvCloneImage( smallSource );
            cvReleaseImage( &smallSource );

            // next, do the target
            targetSize.width /= 2;
            targetSize.height /= 2;

            IplImage* smallTarget = NULL;
            smallTarget = cvCreateImage( targetSize, depth, numChannels );
            cvPyrDown( copyOfTarget, smallTarget, CV_GAUSSIAN_5x5 );

            // prepare for next loop, if any
            cvReleaseImage( &copyOfTarget );
            copyOfTarget = cvCloneImage( smallTarget );
            cvReleaseImage( &smallTarget );
        }

        // perform the match on the shrunken images
        CvSize smallTargetSize = cvGetSize( copyOfTarget );
        CvSize smallSourceSize = cvGetSize( copyOfSource );

        CvSize resultSize;
        resultSize.width = smallSourceSize.width - smallTargetSize.width + 1;
        resultSize.height = smallSourceSize.height - smallTargetSize.height + 1;

        IplImage* result = cvCreateImage( resultSize, IPL_DEPTH_32F, 1 );

        cvMatchTemplate( copyOfSource, copyOfTarget, result, CV_TM_CCOEFF_NORMED );

        // release memory we don't need anymore
        cvReleaseImage( &copyOfSource );
        cvReleaseImage( &copyOfTarget );

        // find the top match locations
        CvPoint* locations = NULL;
        MultipleMaxLoc( *result, &locations, numMaxima );

        cvReleaseImage( &result );

        // search the large images at the returned locations
        sourceSize = cvGetSize( &source );
        targetSize = cvGetSize( &target );

        // create a copy of the source in order to adjust its ROI for searching
        IplImage* searchImage = cvCloneImage( &source );
        for( int currMax = 0; currMax < numMaxima; currMax++ )
        {
            // transform the point to its corresponding point in the larger image
            locations[currMax].x *= ( int )pow( 2, double(numDownPyrs) );
            locations[currMax].y *= ( int )pow( 2, double(numDownPyrs) );
            locations[currMax].x += targetSize.width / 2;
            locations[currMax].y += targetSize.height / 2;

            const CvPoint& searchPoint = locations[currMax];

            // if we are searching for multiple targets and we have found a target or
            //  multiple targets, we don't want to search in the same location(s) again
            if( findMultipleTargets && !foundPointsList->empty() )
            {
                bool thisTargetFound = false;

                int numPoints = foundPointsList->size();
                for( int currPoint = 0; currPoint < numPoints; currPoint++ )
                {
                    const CvPoint& foundPoint = ( *foundPointsList )[currPoint];
                    if( abs( searchPoint.x - foundPoint.x ) <= searchExpansion * 2 &&
                        abs( searchPoint.y - foundPoint.y ) <= searchExpansion * 2 )
                    {
                        thisTargetFound = true;
                        break;
                    }
                }

                // if the current target has been found, continue onto the next point
                if( thisTargetFound )
                {
                    continue;
                }
            }

            // set the source image's ROI to slightly larger than the target image,
            //  centred at the current point
            CvRect searchRoi;
            searchRoi.x = searchPoint.x - ( target.width ) / 2 - searchExpansion;
            searchRoi.y = searchPoint.y - ( target.height ) / 2 - searchExpansion;
            searchRoi.width = target.width + searchExpansion * 2;
            searchRoi.height = target.height + searchExpansion * 2;

            // make sure ROI doesn't extend outside of image
            if( searchRoi.x < 0 )
            {
                searchRoi.x = 0;
            }
            if( searchRoi.y < 0 )
            {
                searchRoi.y = 0;
            }
            if( ( searchRoi.x + searchRoi.width ) > ( sourceSize.width - 1 ) )
            {
                int numPixelsOver
                        = ( searchRoi.x + searchRoi.width ) - ( sourceSize.width - 1 );

                searchRoi.width -= numPixelsOver;
            }
            if( ( searchRoi.y + searchRoi.height ) > ( sourceSize.height - 1 ) )
            {
                int numPixelsOver
                        = ( searchRoi.y + searchRoi.height ) - ( sourceSize.height - 1 );

                searchRoi.height -= numPixelsOver;
            }

            cvSetImageROI( searchImage, searchRoi );

            // perform the search on the large images
            resultSize.width = searchRoi.width - target.width + 1;
            resultSize.height = searchRoi.height - target.height + 1;

            result = cvCreateImage( resultSize, IPL_DEPTH_32F, 1 );

            cvMatchTemplate( searchImage, &target, result, CV_TM_CCOEFF_NORMED );
            cvResetImageROI( searchImage );

            // find the best match location
            double minValue, maxValue;
            CvPoint minLoc, maxLoc;
            cvMinMaxLoc( result, &minValue, &maxValue, &minLoc, &maxLoc );
            maxValue *= 100;

            // transform point back to original image
            maxLoc.x += searchRoi.x + target.width / 2;
            maxLoc.y += searchRoi.y + target.height / 2;

            cvReleaseImage( &result );

            if( maxValue >= matchPercentage )
            {
                // add the point to the list
                foundPointsList->push_back( maxLoc );
                confidencesList->push_back( maxValue );

                // if we are only looking for a single target, we have found it, so we
                //  can return
                if( !findMultipleTargets )
                {
                    break;
                }
            }
        }

        if( foundPointsList->empty() )
        {
            printf( "\nTarget was not found to required confidence of %d.\n",
                    matchPercentage );
        }

        delete [] locations;
        cvReleaseImage( &searchImage );

        return true;
    }

    void MultipleMaxLoc( const IplImage& image,
                         CvPoint**       locations,
                         int             numMaxima )
    {
        // initialize input variable locations
        *locations = new CvPoint[numMaxima];

        // create array for tracking maxima
        double * maxima = (double *)malloc(numMaxima*sizeof(double));
        for( int i = 0; i < numMaxima; i++ )
        {
            maxima[i] = 0.0;
        }

        // extract the raw data for analysis
        float* data;
        int step;
        CvSize size;

        cvGetRawData( &image, ( uchar** )&data, &step, &size );

        step /= sizeof( data[0] );

        for( int y = 0; y < size.height; y++, data += step )
        {
            for( int x = 0; x < size.width; x++ )
            {
                // insert the data value into the array if it is greater than any of the
                //  other array values, and bump the other values below it, down
                for( int j = 0; j < numMaxima; j++ )
                {
                    if( data[x] > maxima[j] )
                    {
                        // move the maxima down
                        for( int k = numMaxima - 1; k > j; k-- )
                        {
                            maxima[k] = maxima[k-1];
                            ( *locations )[k] = ( *locations )[k-1];
                        }

                        // insert the value
                        maxima[j] = ( double )data[x];
                        ( *locations )[j].x = x;
                        ( *locations )[j].y = y;
                        break;
                    }
                }
            }
        }
        free(maxima);
    }

    void    DrawFoundTargets( IplImage*              image,
                              const CvSize&          size,
                              const vector<CvPoint>& pointsList,
                              const vector<double>&  confidencesList,
                              CvScalar color)
    {
        int numPoints = pointsList.size();
        for( int currPoint = 0; currPoint < numPoints; currPoint++ )
        {
            const CvPoint& point = pointsList[currPoint];

            // write the confidences to stdout
            /*
            printf( "\nTarget found at (%d, %d), with confidence = %3.3f %%.\n",
                    point.x,
                    point.y,
                    confidencesList[currPoint] );
            */
            // draw a circle at the center
            cvCircle( image, point, 2, color );

            // draw a rectangle around the found target
            CvPoint topLeft;
            topLeft.x = point.x - size.width / 2;
            topLeft.y = point.y - size.height / 2;

            CvPoint bottomRight;
            bottomRight.x = point.x + size.width / 2;
            bottomRight.y = point.y + size.height / 2;

            cvRectangle( image, topLeft, bottomRight, color );
        }
    }
    void addNoise2Image(IplImage* image,
                        int numNoisyPixels,
                        int noiseIntensityAverage,
                        int intensityDeviation)
    {
        CvRNG randomNumGenState = cvRNG(0xffffffff);
        /* Coordinates of Noisy Points */
        CvMat* noiseLocations = cvCreateMat( numNoisyPixels, 1, CV_32SC2 );
        /* Arr of Noise intensity Values */
        CvMat* noiseValues = cvCreateMat( numNoisyPixels, 1, CV_32FC1 );
        CvSize size = cvGetSize( image );
//        cvRandInit( &randomNumGenState,
//                    0, 1,
//                    0xffffffff,// fixed seeding, adjust if needed
//                    CV_RAND_UNI); // randomness type (uniform in this case)
        /* Fill the noiseLocations array randomly using a uniform distribution */
        cvRandArr( &randomNumGenState, noiseLocations, CV_RAND_UNI, cvScalar(0,0,0,0), cvScalar(size.width,size.height,0,0) );
        /* Fill the noise array with random intensity noiseValues */
        cvRandArr( &randomNumGenState, noiseValues, CV_RAND_NORMAL,
                   cvRealScalar(noiseIntensityAverage), // noise average (intensity based)
                   cvRealScalar(intensityDeviation)     // noise deviation (intensity based)
                   );

        int numChannels = image->nChannels;
        int widthStep = image->widthStep;
        for( int i = 0; i < numNoisyPixels; i++ )
        {
            CvPoint pt = *(CvPoint*)cvPtr1D( noiseLocations, i, 0 );
            uchar value = *(cvPtr1D( noiseValues, i, 0 ));
            // add the noise to all the color channels
            for(int j =0;j<numChannels;j++)
                image->imageData[pt.y*widthStep + pt.x*numChannels + j] +=value;
        }
        cvReleaseMat( &noiseLocations );
        cvReleaseMat( &noiseValues );
    }

    void drawBox(IplImage *image, CvBox2D box, CvScalar color)
    {
        CvPoint2D32f boxPoints[4];
        box.angle = box.angle;
        cvBoxPoints(box, boxPoints);
        cvLine(image,
               cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y),
               cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y),
               color, 1, CV_AA, 0 );
        cvLine(image,
               cvPoint((int)boxPoints[1].x, (int)boxPoints[1].y),
               cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y),
               color, 1, CV_AA, 0 );
        cvLine(image,
               cvPoint((int)boxPoints[2].x, (int)boxPoints[2].y),
               cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y),
               color, 1, CV_AA, 0 );
        cvLine(image,
               cvPoint((int)boxPoints[3].x, (int)boxPoints[3].y),
               cvPoint((int)boxPoints[0].x, (int)boxPoints[0].y),
               color, 1, CV_AA, 0 );
    }

    double calculateContourArea( const CvSeq* contour)
    {
        double area;
        if( contour->total )
        {
            CvSeqReader reader;
            int lpt = contour->total;
            double a00 = 0, xi_1, yi_1;
            bool is_float = (CV_SEQ_ELTYPE(contour) == CV_32FC2);

            cvStartReadSeq( contour, &reader, 0 );

            if( !is_float )
            {
                 xi_1 = ((CvPoint*)(reader.ptr))->x;
                 yi_1 = ((CvPoint*)(reader.ptr))->y;
            }
            else
            {
                xi_1 = ((CvPoint2D32f*)(reader.ptr))->x;
                yi_1 = ((CvPoint2D32f*)(reader.ptr))->y;
            }

            CV_NEXT_SEQ_ELEM( contour->elem_size, reader );

            double dxy, xi, yi;
            while( lpt-- > 0 )
            {
                if( !is_float )
                {
                    xi = ((CvPoint*)(reader.ptr))->x;
                    yi = ((CvPoint*)(reader.ptr))->y;
                }
                else
                {
                    xi = ((CvPoint2D32f*)(reader.ptr))->x;
                    yi = ((CvPoint2D32f*)(reader.ptr))->y;
                }
                CV_NEXT_SEQ_ELEM( contour->elem_size, reader );
                dxy = xi_1 * yi - xi * yi_1;
//                qDebug()<<"Xi_1:"<<xi_1<<" Yi_1:"<<yi_1<<" Xi:"<<xi<<" Yi:"<<yi<<" lpt:"<<lpt<<" total:"<<contour->total<<" A00:"<<a00;
                a00 += dxy;
                xi_1 = xi;
                yi_1 = yi;
            }
            area = a00 * 0.5;
//            qDebug()<<"Total Area :"<<area<<" A00:"<<a00;
        }
        else
        {
            area = 0;
        }
        return area;
    }

    void getWeightedAreaCentroid(IplImage *image,CvRect rect,double&area,CvPoint2D64f &centroid,int buffer, int backgroundColorIntensity)
    {
        CV_FUNCNAME("getWeightedAreaCentroid");
        __BEGIN__;
        // we can only perform weighted Area on a 1 channel image
        if(image->nChannels!=1)
            return;
        int startX = rect.x - buffer, endX = rect.x + rect.width  + buffer;
        int startY = rect.y - buffer, endY = rect.y + rect.height + buffer;
        int widthStep = image->widthStep;
        if(startX<0) startX = 0; if(endX>=image->width)  endX = image->width; // no -1 because the look check is < not <=
        if(startY<0) startY = 0; if(endY>=image->height) endY = image->height;// no -1 because the look check is < not <=
        // Initialize the area variable
        area = 0;
        double minI =255,value=0,normalizedValue;
        //get Min intensity value in that region (used a reference for normalization)
        // TODO:: instead of using the min, a proper reference color should be extracetd either from local area
        // or from the surrounding blobs. A histogram can help but wont solve the issue with small color blobs
        for(int i=startX;i<endX;i++)
        {
            for(int j=startY;j<endY;j++)
            {
                value = (uchar)(image->imageData[j*widthStep + i]);
                if(value < minI )
                    minI  = value;
            }
        }
        // get Moments (00,01,10) ==> m_{p,q} = sum_{i=0}^{n}I(x,y)x^p*y^q
        double m00=0,m01=0,m10=0;
        for(int i=startX;i<endX;i++)
        {
            for(int j=startY;j<endY;j++)
            {
                value = (uchar)(image->imageData[j*widthStep + i]);
                // normalizedValue [0,1] and is max when the pixel is darkest, and 0 when pixel eq background color
                normalizedValue = (backgroundColorIntensity - value)/double(backgroundColorIntensity - minI);
                area+= normalizedValue;
                m00+=double(normalizedValue);
                m10+=double(normalizedValue*i);
                m01+=double(normalizedValue*j);
            }
        }
        centroid.x = m10/m00;
        centroid.y = m01/m00;
        __END__;
    }

    /*!
        This function calculates a weighted area from a gray scale (single channel)
        image. By default the assumption is that the background color is white (intensity 255)
        so the background value is subtracted from the average. A buffer area around the bounding
        rectangle can be used to get more information from the surrounding pixels.

    */
    double calculateWeightedArea(IplImage *image,CvRect rect,int buffer, int backgroundColorIntensity)
    {
        // we can only perform weighted Area on a 1 channel image
        if(image->nChannels!=1)
            return 0;
        double area=0;
        int startX = rect.x - buffer, endX = rect.x + rect.width  + buffer;
        int startY = rect.y - buffer, endY = rect.y + rect.height + buffer;
        int widthStep = image->widthStep;
        if(startX<0) startX = 0; if(endX>=image->width)  endX = image->width; // no -1 because the look check is < not <=
        if(startY<0) startY = 0; if(endY>=image->height) endY = image->height;// no -1 because the look check is < not <=
        //qDebug()<<"Rect Start x:"<<rect.x<<" y:"<<rect.y<<" rect w:"<<rect.width<<" h:"<<rect.height<<" startX:"<<startX<<" endX:"<<endX<<" startY:"<<startY<<" endY:"<<endY;
        int value;
        for(int i=startX;i<endX;i++)
        {
            for(int j=startY;j<endY;j++)
            {
                value = (uchar)(image->imageData[j*widthStep + i]);
                area+= (backgroundColorIntensity - value)/255.0;
            }
        }
        //qDebug()<<"Expected max area:"<<((rect.height+2*buffer)*(rect.width+2*buffer))<<" calculated Area is:"<<area;
        return area;
    }

    double calculateUpSampledContourArea(IplImage *image,CvRect rect,double &x, double &y,int buffer)
    {
        // we can only perform weighted Area on a 1 channel image
        if(image->nChannels!=1)
            return 0;
        CvMoments moments;
        double m00,m10,m01;
        int startX = rect.x - buffer, endX = rect.x + rect.width  + buffer;
        int startY = rect.y - buffer, endY = rect.y + rect.height + buffer;
        if(startX<0) startX = 0; if(endX>=image->width)  endX = image->width -1;
        if(startY<0) startY = 0; if(endY>=image->height) endY = image->height-1;
//        qDebug()<<"Here 1 startX:"<<startX<<" startY:"<<startY<<" endX:"<<endX<<" endY:"<<endY;
        cvSetImageROI(image,cvRect( startX, startY,endX-startX,endY-startY));
        IplImage * srcPatch = cvCreateImage(cvGetSize(image), 8, 1 );
//        cvSaveImage(QString("image%1.png").arg(i).toAscii(),image);
        cvCopy(image,srcPatch);
//        cvSaveImage(QString("src%1.png").arg(i).toAscii(),srcPatch);
        CvMemStorage *storage = 0;
        double area,maxArea=0;
        CvSize size;
        size= cvGetSize(image);
        size.height = srcPatch->height*2;
        size.width  = srcPatch->width*2;
        IplImage * doubleImage  = cvCreateImage(size, 8, 1 );
//        qDebug()<<"Image size h:"<<image->height<<" w:"<<image->width<<" srcPatch h:"<<srcPatch->height<<" w:"<<srcPatch->width<<" Doubled h:"<<doubleImage->height<<" w:"<<doubleImage->width;
        cvPyrUp(srcPatch,doubleImage,IPL_GAUSSIAN_5x5);
//        cvSaveImage(QString("double%1.png").arg(i).toAscii(),doubleImage);
        IplImage * temp_channel = cvCloneImage(doubleImage);
        double minValue, maxValue;
        CvPoint minLoc, maxLoc;
        cvMinMaxLoc(doubleImage, &minValue, &maxValue, &minLoc, &maxLoc );
        cvThreshold(doubleImage,doubleImage , maxValue*0.80 ,255, CV_THRESH_BINARY_INV);
//        cvAdaptiveThreshold(temp_channel,temp_channel,255,CV_ADAPTIVE_THRESH_GAUSSIAN_C,CV_THRESH_BINARY_INV,15,5);
//        cvOr(doubleImage,temp_channel,doubleImage);
//        cvSaveImage(QString("afterFilter%1.png").arg(i).toAscii(),doubleImage);

        CvSeq* contours = 0;
        storage = cvCreateMemStorage(0);
//        IplImage* dst = cvCreateImage( cvGetSize(doubleImage), 8, 3 );
//        cvCvtColor(doubleImage,dst,CV_GRAY2RGB);
        cvFindContours(doubleImage, storage, &contours, sizeof(CvContour),CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, cvPoint(0,0) );
        // Hopefully their should be only one contour, but in case we ended up breaking up
        // the contour, then just take the largest on
        for( ; contours != 0; contours = contours->h_next )
        {
//            cvDrawContours( dst, contours, CV_RGB(0,255,0), CV_RGB(0,255,0), -1, 1, CV_AA, cvPoint(0,0) );
            area = fabs(calculateContourArea(contours));
//            qDebug()<<"Area is:"<<area;
            if(area>maxArea)
            {
                maxArea = area;
                cvContourMoments(contours,&moments);
                m00 = cvGetSpatialMoment(&moments, 0, 0),
                m10 = cvGetSpatialMoment(&moments, 1, 0),
                m01 = cvGetSpatialMoment(&moments, 0, 1);
                // Centre of Area Calculation
                x = m10/m00;
                y = m01/m00;
            }
        }
//        cvSaveImage(QString("contour%1.png").arg(i).toAscii(),dst);
        x = startX + x/2.0;
        y = startY + y/2.0;
        if(storage)
            cvReleaseMemStorage(&storage);
//        cvReleaseImage(&dst);
        cvReleaseImage(&doubleImage);
        cvReleaseImage(&temp_channel);
        cvReleaseImage(&srcPatch);
        cvResetImageROI(image);
        return maxArea;
    }
/*
    double pghMatchShapes(CvSeq *shape1, CvSeq *shape2)
    {
        int dims[] = {8, 8};
        float range[] = {-180, 180, -100, 100};
        float *ranges[] = {&range[0], &range[2]};
        CvHistogram* hist1 = cvCreateHist(2, dims, CV_HIST_ARRAY, ranges, 1);
        CvHistogram* hist2 = cvCreateHist(2, dims, CV_HIST_ARRAY, ranges, 1);
        cvCalcPGH(shape1, hist1);
        cvCalcPGH(shape2, hist2);
        cvNormalizeHist(hist1, 100.0f);
        cvNormalizeHist(hist2, 100.0f);
        double corr = cvCompareHist(hist1, hist2, CV_COMP_BHATTACHARYYA);
        cvReleaseHist(&hist1);
        cvReleaseHist(&hist2);
        return corr;
    }
*/
    IplImage* minRGB(IplImage* src)
    {
        qDebug() <<"Min RGB";
        CV_FUNCNAME("MinRGB");
        IplImage* gray_image=0;
        IplImage* temp_channel=0;
        __BEGIN__;  // start processing. There may be some declarations just after this macro,
                    // but they couldn't be accessed from the epilogue.

        qDebug() << "MinRGB: SRC Image Channel Seq is:"<<src->channelSeq <<" and colorModel is:<<"<<src->colorModel<<" W:"<<src->width<<" H:"<<src->height;
        CV_CALL(gray_image  = cvCreateImage( cvGetSize(src), 8, 1 ));
        CV_CALL(temp_channel= cvCreateImage( cvGetSize(src), 8, 1 ));

        cvSetImageROI( gray_image,cvGetImageROI(src));
        cvSetImageROI( temp_channel,cvGetImageROI(src));

        //    qDebug() << "Splitting Channel 1";
        CV_CALL(cvSplit(src,temp_channel,NULL,NULL,NULL));
        CV_CALL(cvMin(temp_channel,temp_channel,gray_image));
        //    qDebug() << "Splitting Channel 2";
        CV_CALL(cvSplit(src,NULL,temp_channel,NULL,NULL));
        CV_CALL(cvMin(temp_channel,gray_image,gray_image));
        //    qDebug() << "Splitting Channel 3";
        CV_CALL(cvSplit(src,NULL,NULL,temp_channel,NULL));
        CV_CALL(cvMin(temp_channel,gray_image,gray_image));

        __END__; // finish processing. Epilogue follows after the macro.

        cvReleaseImage(&temp_channel);
        qDebug() <<"MinRGB END";
        return gray_image;
    }

}
