/*
 * AutoFocus.h
 *
 *  Created on: 20/01/2008
 *      Author: ttaha
 */
#include "templatemodel.h"

TemplateModel::TemplateModel(QObject *parent)
    : QAbstractListModel(parent)
{

}

QVariant TemplateModel::data(const QModelIndex &index, int role) const
{
//    qDebug("1");
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole)
        return QIcon(pixmaps.value(index.row()).scaled(60, 60,
                         Qt::KeepAspectRatio, Qt::SmoothTransformation));
    else if (role == Qt::UserRole)
        return pixmaps.value(index.row());
    else if (role == Qt::UserRole + 1)
        return locations.value(index.row());

    return QVariant();
}

Qt::ItemFlags TemplateModel::flags(const QModelIndex &index) const
{
//    qDebug("Index Column:%d Row:%d",index.column(),index.row());
    if (index.isValid())
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    return Qt::ItemIsDropEnabled;
}

bool TemplateModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug("3");
    if (parent.isValid())
        return false;

    if (row >= pixmaps.size() || row + count <= 0)
        return false;

    int beginRow = qMax(0, row);
    int endRow = qMin(row + count - 1, pixmaps.size() - 1);

    beginRemoveRows(parent, beginRow, endRow);

    while (beginRow <= endRow)
    {
        pixmaps.removeAt(beginRow);
        locations.removeAt(beginRow);
        ++beginRow;
    }

    endRemoveRows();
    return true;
}

QStringList TemplateModel::mimeTypes() const
{
    qDebug("4");
    QStringList types;
    types << "image/x-template";
    return types;
}

QMimeData *TemplateModel::mimeData(const QModelIndexList &indexes) const
{
    qDebug("5");
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            QPixmap pixmap = qVariantValue<QPixmap>(data(index, Qt::UserRole));
            QPoint location = data(index, Qt::UserRole+1).toPoint();
            stream << pixmap << location;
        }
    }

    mimeData->setData("image/x-template", encodedData);
    return mimeData;
}

bool TemplateModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                               int row, int column, const QModelIndex &parent)
{
    qDebug("6");
    if (!data->hasFormat("image/x-template"))
        return false;

    if (action == Qt::IgnoreAction)
        return true;

    if (column > 0)
        return false;

    int endRow;

    if (!parent.isValid())
    {
        if (row < 0)
            endRow = pixmaps.size();
        else
            endRow = qMin(row, pixmaps.size());
    } else
        endRow = parent.row();

    QByteArray encodedData = data->data("image/x-template");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
        QPixmap pixmap;
        QPoint location;
        stream >> pixmap >> location;

        beginInsertRows(QModelIndex(), endRow, endRow);
        pixmaps.insert(endRow, pixmap);
        locations.insert(endRow, location);
        endInsertRows();

        ++endRow;
    }

    return true;
}

int TemplateModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    else
        return pixmaps.size();
}

Qt::DropActions TemplateModel::supportedDropActions() const
{
    qDebug("8");
    return Qt::CopyAction | Qt::MoveAction;
}

const QFileInfoList &  TemplateModel::getTemplatesList()
{
    return templatesList;
}

void TemplateModel::reloadPieces()
{
    beginRemoveRows(QModelIndex(), 0, 24);
    pixmaps.clear();
    locations.clear();
    endRemoveRows();
    addPieces(templatesPath);
    emit sendTemplatesList(templatesList);
}

void TemplateModel::addPiece(const QPixmap &pixmap, const int& row)
{
    beginInsertRows(QModelIndex(), row, row);
    pixmaps.insert(row, pixmap);
    endInsertRows();
}

void TemplateModel::addPieces(const QString& path)
{
    templatesPath = path;
    templatesList.clear();
    dir1.setPath(path);
    dir1.setFilter(QDir::Files | QDir::NoSymLinks);
    dir1.setSorting(QDir::Name | QDir::Reversed);
    QStringList filters;
    filters << "*.bmp" << "*.jpg" << "*.png";
    dir1.setNameFilters(filters);
    templatesList = dir1.entryInfoList();

    for(int i=0;i<templatesList.size();i++)
    {
        addPiece(QPixmap(qPrintable(templatesList[i].absoluteFilePath())),i);
    }
}

void TemplateModel::keyPressEvent( QKeyEvent *e )
{
    switch( e->key() )
    {
    case Qt::Key_Up:
        qDebug("Up");
        break;
    case Qt::Key_Down:
        qDebug("down");
        break;
    case Qt::Key_Right:
        qDebug("right");
        break;
    case Qt::Key_Left:
        qDebug("left");
        break;
    case Qt::Key_Space:
        qDebug("Space");
        break;
    }
}

void TemplateModel::mouseReleaseEvent(QMouseEvent *event)
{
    QPoint p = event->pos();
    qDebug("Mouse Clicked");
}
