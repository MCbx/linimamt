#ifndef IDIRTREEWIDGET_H
#define IDIRTREEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QTreeWidget>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QApplication>
#include <QDrag>

class IDirTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    IDirTreeWidget(QWidget *parent=0);


private:
    virtual bool dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action);
    QStringList mimeTypes() const;
    Qt::DropActions supportedDropActions() const;
  //  void mousePressEvent(QMouseEvent *event);
 //   void mouseMoveEvent(QMouseEvent *event);
    void dropEvent(QDropEvent *event);

    QTreeWidgetItem * destination;
 //   void performDrag();

   // QPoint startPos;
  //  ImageFile * img; //the control must be aware of the image file it uses
  //  QString currentDir; //the control must be aware of the current directory viewed!

signals:
    void sigDragDrop(QStringList from, QString to); //this one gets emitted when drag-drop actio has been made
};

#endif // IDIRTREEWIDGET_H
