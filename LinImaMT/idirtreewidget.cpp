#include "idirtreewidget.h"
#include <QFileInfo>



IDirTreeWidget::IDirTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
        setAcceptDrops(true);
}

bool IDirTreeWidget::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{

    QList<QUrl> urlList;
    QString dest="";
    if (this->destination!=NULL)
    {
        dest=this->destination->text(1);
    }

    urlList = data->urls(); // retrieve list of urls
    foreach(QUrl url, urlList) // iterate over list
    {
        if ((dest!="")||( url.isLocalFile() ) )
        {
            QString source=url.toString();
            if (!url.isLocalFile())
            {
                source=url.path();
            }
            if (source!=dest) //drag dir to dir
            {
                //last stage verification
                //Move file to its directory -> FAIL
                if ((source.left(source.lastIndexOf("/"))+"/"!=dest) &&
                        (source!=dest))
                {
                    //launch what should be launched
                    emit sigDragDrop(source,dest);
                }
            }
          //  this->clearSelection(); //without it it will lock into multiselect
        }
    }

//    this->setCurrentItem(destination);
    return true;
}

QStringList IDirTreeWidget::mimeTypes () const
{
    QStringList qstrList;
    // list of accepted mime types for drop
    qstrList.append("text/uri-list");
    return qstrList;
}

Qt::DropActions IDirTreeWidget::supportedDropActions () const
{
    // returns what actions are supported when dropping
    return Qt::CopyAction | Qt::MoveAction;
}

void IDirTreeWidget::dropEvent(QDropEvent *event)
{
        this->destination=NULL;
        QTreeWidgetItem * a=this->itemAt(event->pos());
        if (a)
        {
                this->destination=a;
        }
        QTreeWidget::dropEvent(event);
}
