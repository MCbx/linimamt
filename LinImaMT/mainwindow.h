#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTreeWidgetItem>
#include "imagefile.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_twDirTree_currentItemChanged(QTreeWidgetItem *current);

    void customSortByColumn(int column);

    void on_twFileTree_itemDoubleClicked(QTreeWidgetItem *item);

    void on_actionGoUp_triggered();

    void on_actionAddress_bar_triggered();
    void on_label_edit();

    void on_twFileTree_itemSelectionChanged();

    void on_actionSave_As_triggered();

    void on_actionSave_triggered();

    void on_actionRename_triggered();

    void on_actionCreate_Directory_triggered();

    void on_actionVolume_Serial_triggered();

    void on_actionExtract_selected_triggered();

private:
    Ui::MainWindow *ui;
    QProcess *process;
    QString currentFile;
    QString currentDir;
    ImageFile * img;

    //structure
    QList<ImageFile::fileEntry>dirs;

    //functions
    int execute(QString command, QString parameters, QString & result);
    int loadFile(QString fileName);
    int prepareDirDump(QString home);
    int errorMessage(QString text, QString console);
    void visualize();
    void statusBarNormal();
    void visualizeModified();

    //ui components
    QLineEdit * leAddress;
    QLineEdit * leLabel;
};

#endif // MAINWINDOW_H
