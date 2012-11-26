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
#include "chipgraphicitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include "nozzlehealthviewer.h"

SingleNozzle::SingleNozzle(QGraphicsItem *parent,int _chipID, int _row,int _column,double _status)
    : QGraphicsItem(parent), color(Qt::lightGray), dragOver(false),
    chipID(_chipID),
    row(_row),
    column(_column),
    status(_status)
{
    setFlags(ItemIsSelectable);
    setAcceptsHoverEvents(true);
    NozzleOperations nozOp;
    QPoint p = nozOp.localLocation2Abs(_chipID,_row,_column);
    absPosition  = p.x();
    colorChannel = p.y();
    if(p.x()==-1 && p.y()==-1)
    {
        color = Qt::gray;
    }
    else
    {
        // This will give a red color for a nozzle that is always dead
        // green for a nozzle that is always firing
        // and a gradient between for anything else
        color = QColor(qRound(255*(1-status)),qRound(255*status),0);
    }
}

void SingleNozzle::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    if (event->mimeData()->hasColor())
    {
        event->setAccepted(true);
        dragOver = true;
        update();
    }
    else
    {
        event->setAccepted(false);
    }
}

void SingleNozzle::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    dragOver = false;
    update();
}

void SingleNozzle::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    dragOver = false;
    if (event->mimeData()->hasColor())
        color = qVariantValue<QColor>(event->mimeData()->colorData());
    else if (event->mimeData()->hasImage())
        pixmap = qVariantValue<QPixmap>(event->mimeData()->imageData());
    update();
}

QPainterPath SingleNozzle::shape() const
{
    QPainterPath path;
    path.addRect(-10,-10 , 20, 20);
    return path;
}

QRectF SingleNozzle::boundingRect() const
{
    return QRectF(-15, -15, 30, 30);
}

void SingleNozzle::paint(QPainter *painter,const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    QColor fillColor = (option->state & QStyle::State_Selected) ? Qt::yellow : color;
    if (option->state & QStyle::State_MouseOver)
    {
//        QFont font("Times", 10);
//        font.setStyleStrategy(QFont::ForceOutline);
//        painter->setFont(font);
//        painter->save();
//        painter->scale(1, 1);
//        painter->drawText(10, 180, QString("Nozzle: at row:%1 column:%2").arg(row).arg(column));
//        painter->restore();
        fillColor = Qt::yellow;// fillColor.darker(150);
    }
    if(row == 4 && column==0)
    {
        QFont font("Times", 16);
        font.setBold(true);
        font.setStyleStrategy(QFont::ForceOutline);
        painter->setFont(font);
        painter->save();
        painter->drawText(-200, -15, QString("Chip %1").arg(chipID));
        painter->restore();
    }
    painter->setBrush(fillColor);
    painter->drawEllipse(-10, -10, 20, 20);
}

int SingleNozzle::type() const
{
    return Type;
}

void SingleNozzle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    QGraphicsItem::hoverEnterEvent(event);
    //emit showInfo(QString("Chip:%1 Row:%2 Nozzle:%3").arg(chipID).arg(row).arg(column));
    //setSelected(true);
    update();
}
void SingleNozzle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
    update();
}

void SingleNozzle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->modifiers() & Qt::ShiftModifier)
    {
        update();
        return;
    }
    QGraphicsItem::mouseMoveEvent(event);
}

void SingleNozzle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
    update();
}

ChipGraphicItem::ChipGraphicItem(NozzleHealthViewer *graphWidget,Chip chip)
    : graph(graphWidget),
    chipID(chip.getChipID())
{
    setFlag(ItemIsMovable);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);
    crossRowsReferences[0] = 3;
    crossRowsReferences[1] = 5;
    crossRowsReferences[2] = 11;
    crossRowsReferences[3] = 13;
    crossRowsReferences[4] = 19;
    crossRowsReferences[5] = 21;
    crossRowsReferences[6] = 27;
    crossRowsReferences[7] = 29;
    crossRowsReferences[8] = 35;
    crossRowsReferences[9] = 37;

    rowXPositionOffset[0] = 22;
    rowXPositionOffset[1] = 20;
    rowXPositionOffset[2] = 17;
    rowXPositionOffset[3] = 15;
    rowXPositionOffset[4] = 12;
    rowXPositionOffset[5] = 10;
    rowXPositionOffset[6] = 7;
    rowXPositionOffset[7] = 5;
    rowXPositionOffset[8] = 2;
    rowXPositionOffset[9] = 0;
    int gridSize = 30;
    //int innerRowGap  = gridSize;
    int colorPlanGap = 2*gridSize;
    int startX,startY,yOffset;
    int topGap = 200;
    yOffset = topGap + chipID*750;
    for(int i=0;i<Chip::numOfRows;i++)
    {
        for(int j=0;j<Chip::numOfNozzlesPerRow;j++)
        {
            startX = rowXPositionOffset[i]*gridSize + j*gridSize;
            if(j<=crossRowsReferences[i])
            {
                startY = yOffset + i*gridSize  + int(i/2.0f)*(colorPlanGap);
            }
            else
            {
                startY = yOffset + i*gridSize  + int(i/2.0f)*(colorPlanGap) - (2*gridSize + colorPlanGap);
            }
            if((i%2)!=0)
            {
                startX += int(gridSize/2.0f);
            }
            SingleNozzle *item = new SingleNozzle(this,chip.getChipID(),i,j,chip.getNozzleHealth(i,j));
            item->setPos(QPointF(startX, startY));
            //connect(item,SIGNAL(showInfo(QString)),graph,SLOT(showInfo(QString)));
        }
    }
    update();
}


QRectF ChipGraphicItem::boundingRect() const
{
    return QRectF(0,0,30*750,200+750*11);
}

void ChipGraphicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int gridSize = 30;
    int colorPlanGap = 2*gridSize;
    int yOffset, topGap = 200;
    yOffset = topGap + chipID*750;
    int evenRowStartIndex,evenRowEndIndex,oddRowStartIndex,oddRowEndIndex,evenRowIndex,oddRowIndex;
    QPointF evenRowStartRef,evenRowEndRef,oddRowStartRef,oddRowEndRef;
    for(int i=0;i<Chip::numColorChannels;i++)
    {
        for(uint j=0;j<REMORA_PLANE_FEEDHOLE_COUNTS[i];j++)
        {
            evenRowStartIndex = FEEDHOLES_REMORA[i][j].even.start;
            evenRowEndIndex   = FEEDHOLES_REMORA[i][j].even.end;
            oddRowStartIndex  = FEEDHOLES_REMORA[i][j].odd.start;
            oddRowEndIndex    = FEEDHOLES_REMORA[i][j].odd.end ;

            evenRowIndex = i*2;
            oddRowIndex  = i*2 + 1;
            //Even Row start
            evenRowStartRef.setX(rowXPositionOffset[evenRowIndex]*gridSize + evenRowStartIndex*gridSize);
            if(evenRowStartIndex<=crossRowsReferences[evenRowIndex])
            {
                evenRowStartRef.setY(yOffset + evenRowIndex*gridSize  + i*(colorPlanGap));
            }
            else
            {
                evenRowStartRef.setY(yOffset + evenRowIndex*gridSize  + i*(colorPlanGap) - (2*gridSize + colorPlanGap));
            }
            //Even Row End
            evenRowEndRef.setX(rowXPositionOffset[evenRowIndex]*gridSize + evenRowEndIndex*gridSize);
            if(evenRowEndIndex<=crossRowsReferences[evenRowIndex])
            {
                evenRowEndRef.setY(yOffset + evenRowIndex*gridSize  + i*(colorPlanGap));
            }
            else
            {
                evenRowEndRef.setY(yOffset + evenRowIndex*gridSize  + i*(colorPlanGap) - (2*gridSize + colorPlanGap));
            }
            //Odd Row start
            oddRowStartRef.setX(rowXPositionOffset[oddRowIndex]*gridSize + oddRowStartIndex*gridSize + gridSize/2.0f);
            if(oddRowStartIndex<=crossRowsReferences[oddRowIndex])
            {
                oddRowStartRef.setY(yOffset + oddRowIndex*gridSize  + i*(colorPlanGap));
            }
            else
            {
                oddRowStartRef.setY(yOffset + oddRowIndex*gridSize  + i*(colorPlanGap) - (2*gridSize + colorPlanGap));
            }

            //Odd Row End
            oddRowEndRef.setX(rowXPositionOffset[oddRowIndex]*gridSize + oddRowEndIndex*gridSize + gridSize/2.0f);
            if(oddRowEndIndex<=crossRowsReferences[oddRowIndex])
            {
                oddRowEndRef.setY(yOffset + oddRowIndex*gridSize  + i*(colorPlanGap));
            }
            else
            {
                oddRowEndRef.setY(yOffset + oddRowIndex*gridSize  + i*(colorPlanGap) - (2*gridSize + colorPlanGap));
            }
            evenRowStartRef.setX(evenRowStartRef.x() - gridSize/2); evenRowStartRef.setY(evenRowStartRef.y() - gridSize/2);
            evenRowEndRef.setX(evenRowEndRef.x() + gridSize/2);     evenRowEndRef.setY(evenRowEndRef.y() - gridSize/2);

            oddRowStartRef.setX(oddRowStartRef.x() - gridSize/2); oddRowStartRef.setY(oddRowStartRef.y() + gridSize/2);
            oddRowEndRef.setX(oddRowEndRef.x() + gridSize/2); oddRowEndRef.setY(oddRowEndRef.y() + gridSize/2);

            painter->drawLine(evenRowStartRef,evenRowEndRef);
            painter->drawLine(evenRowStartRef,QPointF(evenRowStartRef.x(),evenRowStartRef.y()+gridSize));
            painter->drawLine(evenRowEndRef,QPointF(evenRowEndRef.x(),evenRowEndRef.y()+gridSize));
            painter->drawLine(oddRowStartRef,oddRowEndRef);
            painter->drawLine(oddRowStartRef,QPointF(oddRowStartRef.x(),oddRowStartRef.y()-gridSize));
            painter->drawLine(oddRowEndRef,QPointF(oddRowEndRef.x(),oddRowEndRef.y()-gridSize));

            painter->drawLine(QPointF(evenRowStartRef.x(),evenRowStartRef.y()+gridSize),QPointF(oddRowStartRef.x(),oddRowStartRef.y()-gridSize));
            painter->drawLine(QPointF(evenRowEndRef.x(),evenRowEndRef.y()+gridSize),QPointF(oddRowEndRef.x(),oddRowEndRef.y()-gridSize));
        }
    }
//    qDebug()<<"Printing Font for chip:"<<chipID;
//    QFont font("Times", 12);
//    font.setStyleStrategy(QFont::ForceOutline);
//    painter->setFont(font);
//    painter->save();
//    painter->drawText(10, 10, QString("Chip %1").arg(chipID));
//    painter->restore();
//    Q_UNUSED(widget);
//    QRadialGradient gradient(-3, -3, 10);
//    if (option->state & QStyle::State_Sunken) {
//        gradient.setCenter(3, 3);
//        gradient.setFocalPoint(3, 3);
//        gradient.setColorAt(1, QColor(Qt::yellow).light(120));
//        gradient.setColorAt(0, QColor(Qt::darkYellow).light(120));
//    } else {
//        gradient.setColorAt(0, Qt::yellow);
//        gradient.setColorAt(1, Qt::darkYellow);
//    }
////    painter->setPen(Qt::NoPen);
////    painter->setBrush(Qt::darkGray);
////    painter->drawEllipse(0, 0, 20, 20);
//
//    painter->setBrush(gradient);
//    painter->setPen(QPen(Qt::black, 0));
//    int gridSize = 30;
//    int gap = 5;
//    int ellipseR = gridSize - gap;
//    int startX,startY,yOffset;
//    yOffset = 90;
//    for(int i=0;i<10;i++)
//    {
//        for(int j=0;j<640;j++)
//        {
//            startX = rowXPositionOffset[i]*gridSize + j*gridSize;
//            if(j>crossRowsReferences[i])
//            {
//                startY = yOffset + i*gridSize - gridSize*3 + int(i/2.0f)*gridSize;
//            }
//            else
//            {
//                startY = yOffset + i*gridSize  + int(i/2.0f)*gridSize;
//            }
//            if((i%2)!=0)
//                startX +=(gridSize/2.0f);
//            painter->drawEllipse(startX,startY, ellipseR, ellipseR);
//        }
//    }
}

QVariant ChipGraphicItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
//    switch (change) {
//    case ItemPositionHasChanged:
//        foreach (Edge *edge, edgeList)
//            edge->adjust();
//        graph->itemMoved();
//        break;
//    default:
//        break;
//    };
//
    return QGraphicsItem::itemChange(change, value);
}

void ChipGraphicItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    update();
//    mousePressedLocation.setX(event->pos().x());
//    mousePressedLocation.setY(event->pos().y());
//    mousePressed = true;
    QGraphicsItem::mousePressEvent(event);
}

void ChipGraphicItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    update();
//    mousePressed = false;
    QGraphicsItem::mouseReleaseEvent(event);
}

void ChipGraphicItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    update();
//    QPoint deltaMove(mousePressedLocation.x() - event->pos().x(),mousePressedLocation.y()-event->pos().y());
//    qDebug()<<" Delta move x:"<<deltaMove.x()<<" y:"<<deltaMove.y();
//    if(mousePressed)
//        translate(deltaMove.x(),deltaMove.y());
    QGraphicsItem::mouseMoveEvent(event);
}
