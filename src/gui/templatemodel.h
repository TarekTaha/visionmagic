/*
 * AutoFocus.h
 *
 *  Created on: 20/01/2008
 *      Author: ttaha
 */
#ifndef TEMPLATEMODEL_H
#define TEMPLATEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QPixmap>
#include <QPoint>
#include <QIcon>
#include <QDir>
#include <QMimeData>
#include <QStringList>
#include <QKeyEvent>

class TemplateModel : public QAbstractListModel
{
    Q_OBJECT
public:
    TemplateModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool removeRows(int row, int count, const QModelIndex &parent);
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column, const QModelIndex &parent);
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    QStringList mimeTypes() const;
    int rowCount(const QModelIndex &parent) const;
    Qt::DropActions supportedDropActions() const;
    void addPiece(const QPixmap &pixmap, const int &row);
    void addPieces(const QString& path);
    void reloadPieces();
    const QFileInfoList & getTemplatesList();
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void mouseReleaseEvent(QMouseEvent *event);
signals:
    void sendTemplatesList(const QFileInfoList & );
private:
    QString templatesPath;
    QList<QPoint> locations;
    QList<QPixmap> pixmaps;
    QFileInfoList templatesList;
    QDir dir1;
};

#endif // TEMPLATEMODEL_H
