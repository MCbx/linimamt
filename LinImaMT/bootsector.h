#ifndef BOOTSECTOR_H
#define BOOTSECTOR_H

#include <QDialog>

namespace Ui {
class bootSector;
}

class bootSector : public QDialog
{
    Q_OBJECT

public:
    explicit bootSector(QWidget *parent = 0);
    ~bootSector();

private:
    Ui::bootSector *ui;
};

#endif // BOOTSECTOR_H
