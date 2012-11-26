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
#ifndef CHIPGRAPHICITEM_H
#define CHIPGRAPHICITEM_H

#include <QObject>
#include <QGraphicsItem>
#include <QList>
#include "chip.h"

class NozzleHealthViewer;

QT_BEGIN_NAMESPACE
class QGraphicsSceneMouseEvent;
QT_END_NAMESPACE

class SingleNozzle : public QGraphicsItem
{
public:
    SingleNozzle(QGraphicsItem *parent = 0) : QGraphicsItem(parent){}
    SingleNozzle(QGraphicsItem *parent,int chipID, int row,int column,double status);
    QPainterPath shape() const;
    int getChipID()         {return this->chipID;}
    int getRow()            {return this->row;}
    int getAbsPosition()    {return this->absPosition;}
    int getColumn()         {return this->column;}
    int getColorChannel()   {return this->colorChannel;}
    double getStatus()      {return this->status;}
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void dragEnterEvent(QGraphicsSceneDragDropEvent *event);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent *event);
    void dropEvent(QGraphicsSceneDragDropEvent *event);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QRectF boundingRect() const;
    enum { Type = UserType + 1 };
    int type() const;
    QPixmap pixmap;
    QColor color;
    bool dragOver;
    int chipID,row,column,colorChannel,absPosition;
    double status;
};

class ChipGraphicItem : public SingleNozzle
{
public:
    ChipGraphicItem(NozzleHealthViewer *graphWidget,Chip chip);
    enum { Type = UserType + 1 };
    int type() const { return Type; }
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

private:
    QPointF newPos;
//    QPoint mousePressedLocation;
//    bool mousePressed;
    NozzleHealthViewer *graph;
    int crossRowsReferences[10];
    int rowXPositionOffset[10];
    int chipID;
};

#endif // CHIPGRAPHICITEM_H
