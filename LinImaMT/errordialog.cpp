#include "errordialog.h"
#include "ui_errordialog.h"
#include <QGraphicsView>
#include <QGraphicsRectItem>

ErrorDialog::ErrorDialog(QWidget *parent, QString text, QString cons) :
    QDialog(parent),
    ui(new Ui::ErrorDialog)
{
    ui->setupUi(this);

    QGraphicsScene * sc = new QGraphicsScene(ui->graphicsView);
    sc->setSceneRect(0,0,64,64);
    ui->graphicsView->setScene(sc);
    QGraphicsRectItem * ri = new QGraphicsRectItem(sc->sceneRect());
    sc->addPixmap(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton).pixmap(64,64));
            //QApplication::style()->standardIcon(QStyle::SP_DriveFDIcon)
    ui->lbErrorTitle->setText(text);
    ui->peConsole->setPlainText(cons);
    ui->peConsole->setReadOnly(1);
}

ErrorDialog::~ErrorDialog()
{
    delete ui;
}
