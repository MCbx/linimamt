#include "ifilelistwidget.h"
#include <QMessageBox>
#include <QFileInfo>

void IFileListWidget::setImageFile(ImageFile *img)
{
    this->img=img;
}
void IFileListWidget::setCurrentDir(QString dir)
{
    this->currentDir=dir;
}

bool IFileListWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
    QList<QUrl> urlList;
    //QTreeWidgetItem *item;

    QString dest=currentDir;
    if (this->destination!=NULL)
    {
       // dest=this->currentDir+this->destination->text(0);
        dest=this->destination->text(4);
    }

    urlList = data->urls(); // retrieve list of urls
    foreach(QUrl url, urlList) // iterate over list
    {
        if ((dest!=currentDir)||( url.isLocalFile() ) )
        {
            //QString dest=this->currentDir+this->destination->text(0);
            QString source=url.toString();
            if (!url.isLocalFile())
            {
                source=url.path();
            }
            if (source!=dest) //multiselect + dragging file to sel folder
            {
                //launch what should be launched
                emit sigDragDrop(source,dest);
            }
            this->clearSelection(); //without it it will lock into multiselect
        }
    }

  //  if (this->destination!=NULL)
   //     this->setCurrentItem(destination);

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

void IFileListWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    startPos = event->pos();
    if (DragSelectingState == state())
    {
        setState(NoState);
    }
    QTreeWidget::mousePressEvent(event);
}

void IFileListWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        int distance = (event->pos() - startPos).manhattanLength();
        if (distance >= QApplication::startDragDistance())
            performDrag();
    }
 //   QTreeWidget::mouseMoveEvent(event); //this loops into unfinished tree-type drag-drop causing one click to loose
                                            //Qt bug since Qt3. Do not uncomment
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
                list.append(QUrl(this->selectedItems().at(i)->text(4)));

            mimeData->setUrls(list);

            QDrag *drag = new QDrag(this);
            drag->setMimeData(mimeData);
            drag->start(Qt::CopyAction | Qt::MoveAction);

        }
    }
}

void IFileListWidget::dropEvent(QDropEvent *event)
{
        this->destination=NULL;
        QTreeWidgetItem * a=this->itemAt(event->pos());
        if (a)
        {
            if (a->text(2).at(0)=='D')
            {
                this->destination=a;
            }
        }
        QTreeWidget::dropEvent(event);
}
