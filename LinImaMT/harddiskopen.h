#ifndef HARDDISKOPEN_H
#define HARDDISKOPEN_H

#include <QDialog>

namespace Ui {
class HardDiskOpen;
}

class HardDiskOpen : public QDialog
{
    Q_OBJECT

public:
    explicit HardDiskOpen(QWidget *parent = 0);
    ~HardDiskOpen();

private:
    Ui::HardDiskOpen *ui;
};

#endif // HARDDISKOPEN_H
