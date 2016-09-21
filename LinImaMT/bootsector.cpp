#include "bootsector.h"
#include "ui_bootsector.h"


//This is a ompletely separate subroutine
//It must get from input a path to image.
//Then it takes its boot sector, visualizes in hex, displays OEM stron in another box.
//It allows to dump these 512 bytes into file or read another 512 bytes from file
//and apply it to the image.

bootSector::bootSector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::bootSector)
{    
    ui->setupUi(this);
}

bootSector::~bootSector()
{
    delete ui;
}
