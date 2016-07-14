#ifndef IFILELISTWIDGET_H
#define IFILELISTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>

class IFileListWidget : public QTreeWidget
{
       Q_OBJECT
public:
    IFileListWidget(QWidget *parent=0);

private:
    virtual bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
    void mouseMoveEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *event);

private:
    QString destination;
    void performDrag();

    QPoint startPos;
};

#endif // IFILELISTWIDGET_H
