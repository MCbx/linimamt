#include "ifilelistwidget.h"
#include <QMessageBox>


bool IFileListWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
    QList<QUrl> urlList;
    //QTreeWidgetItem *item;

    urlList = data->urls(); // retrieve list of urls

    foreach(QUrl url, urlList) // iterate over list
    {

        if ((this->destination!="")||( url.isLocalFile() ) )
        {
            QMessageBox::critical(this,"Drag",url.toString()+" into "+this->destination);
            this->clearSelection();
        }
    }

    return true;
}


QStringList IFileListWidget::mimeTypes () const
{
    QStringList qstrList;
    // list of accepted mime types for drop
    qstrList.append("text/uri-list");
    return qstrList;
}


Qt::DropActions IFileListWidget::supportedDropActions () const
{
    // returns what actions are supported when dropping
    return Qt::CopyAction | Qt::MoveAction;
}



IFileListWidget::IFileListWidget(QWidget *parent) : QTreeWidget(parent)
{
    setAcceptDrops(true);
}

//void IFileListWidget::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//        startPos = event->pos();
//    QTreeWidget::mousePressEvent(event);
//}

void IFileListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
    QTreeWidget::mouseMoveEvent(event);
}

void IFileListWidget::performDrag()
{
    for(int i=0;i<this->selectedItems().count();i++)
    {
        QTreeWidgetItem *item = this->selectedItems().at(i);
        if (item) {
            QMimeData *mimeData = new QMimeData;
            QList<QUrl> list;
            for (int i=0;i<this->selectedItems().count();i++)
                list.append(QUrl(this->selectedItems().at(i)->text(0)));

            mimeData->setUrls(list);
            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->start(Qt::CopyAction | Qt::MoveAction);
        }
    }
}

void IFileListWidget::dropEvent(QDropEvent *event)
{
        this->destination="";
        QTreeWidgetItem * a=this->itemAt(event->pos());
        if (a)
        {
            if (a->text(2).at(0)=='D')
            {
                this->destination=a->text(0);
            }
        }
        QTreeWidget::dropEvent(event);
}
