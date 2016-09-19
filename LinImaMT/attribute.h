#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QDialog>
#include <QString>

namespace Ui {
class Attribute;
}

class Attribute : public QDialog
{
    Q_OBJECT

public:
    explicit Attribute(QList<QString> situation, QWidget *parent = 0);
    ~Attribute();
    QString result;

private slots:
    void on_buttonBox_accepted();

    void on_cbReadOnly_clicked();

private:
    Ui::Attribute *ui;
};

#endif // ATTRIBUTE_H
