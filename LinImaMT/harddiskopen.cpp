#include "harddiskopen.h"
#include "ui_harddiskopen.h"

HardDiskOpen::HardDiskOpen(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HardDiskOpen)
{
    ui->setupUi(this);
}

HardDiskOpen::~HardDiskOpen()
{
    delete ui;
}
