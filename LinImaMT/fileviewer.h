#ifndef FILEVIEWER_H
#define FILEVIEWER_H

#include <QDialog>
#include <QByteArray>

namespace Ui {
class fileViewer;
}

class fileViewer : public QDialog
{
    Q_OBJECT

public:
    explicit fileViewer(QString source, QString settingsPath, QString fileName, QWidget *parent = 0);
    ~fileViewer();

private slots:
    void on_cbWordWrap_clicked(bool checked);

    void on_cbDispType_currentIndexChanged(int index);

    void on_btnOpen_clicked();

    void on_btnExport_clicked();

private:

    void showFile();
    void saveSettings();

    Ui::fileViewer *ui;
    QByteArray data;
    QString name;
    QString configu;
    QString path;
    QList <QString> programs;
};

#endif // FILEVIEWER_H
