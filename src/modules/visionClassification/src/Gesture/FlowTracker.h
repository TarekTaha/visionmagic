/***************************************************************************
 *   Vision Classification Library                                         *
 *   Copyright (C) 2010 by:                                                *
 *      Tarek Taha, CAS-UTS  <tataha@cas.edu.au>                           *
 *      Dan Maynes-Aminzade  <monzy@cs.stanford.edu>                       *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/
#ifndef FLOWTRACKER_H
#define FLOWTRACKER_H

#include "constants.h"
#include "Graphics.h"

class FlowTracker
{
public:
    FlowTracker();
    FlowTracker(Graphics *);
    ~FlowTracker(void);
    void learnTrajectories(CvCapture* pCap);
    MotionTrack getTrajectoryInRange(long startFrame, long endFrame);
    MotionTrack getTrajectoryAtFrame(long frameNum);
    void processFrame();
    bool isTrained;

protected:
    IplImage *copyFrame;
    CvCapture *videoCapture;

private:
    MotionTrack trajectory;
    double currentX, currentY;
    Graphics *graphics;
    int numInactiveFrames;

    // Images to store current and previous frame, in color and in grayscale
    IplImage *currentFrame, *grcurrentFrame, *grlastFrame;

    // Arrays to store detected features
    CvPoint2D32f currframe_features[200];
    CvPoint2D32f lastframe_features[200];
    char found_features[200];
    float feature_error[200];

    // Some arrays needed as workspace for the optical flow algorithm
    IplImage *eigimage, *tempimage, *pyramid1, *pyramid2;
};

#endif
