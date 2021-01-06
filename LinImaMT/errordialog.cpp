#include "errordialog.h"
#include "ui_errordialog.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QStyle>
#include <QDebug>

ErrorDialog::ErrorDialog(QWidget *parent, QString text, QString cons) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
//    ui->setupUi(this);

//    QGraphicsScene * sc = new QGraphicsScene(ui->graphicsView);
//    sc->setSceneRect(0,0,64,64);
//    ui->graphicsView->setScene(sc);
//    QGraphicsRectItem * ri = new QGraphicsRectItem(sc->sceneRect());
//    sc->addPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton).pixmap(64,64));
//            //QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)
//    ui->lbErrorTitle->setText(text);
//    ui->peConsole->setPlainText(cons);
//    ui->peConsole->setReadOnly(1);
    this->console=cons;
    this->text=text;
//    qInfo()<<"Error window call\n";

}

void ErrorDialog::append(int code, QString text, QString console)
{
    if (this->code>0)
    {
        this->code=code;
        this->console.append("\n----------------------------------------------------\n"+console+"\nEXIT CODE: "+QString::number(code));
    }
    else
    {
        this->console.append("\n"+console+"\nCode: 0");
    }
    this->text=text;
}

void ErrorDialog::showIt(bool always)
{
    if ((this->code>0)||(always))
    {
        ui->setupUi(this);
        QGraphicsScene * sc = new QGraphicsScene(ui->graphicsView);
        sc->setSceneRect(0,0,64,64);
        ui->graphicsView->setScene(sc);
        QGraphicsRectItem * ri = new QGraphicsRectItem(sc->sceneRect());
        sc->addPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton).pixmap(64,64));
                //QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)
        ui->lbErrorTitle->setText(this->text);
        ui->peConsole->setPlainText(this->console);
        ui->peConsole->setReadOnly(1);
        this->show();
    }
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}
