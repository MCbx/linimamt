#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QTreeWidgetItem>

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

private:
    Ui::MainWindow *ui;
    QProcess *process;
    bool needsSave;
    QString currentFile;
    QString currentDir;
    QString label;
    QString serial;
    unsigned int freeSpace;
    unsigned int usedSpace;

    //structure
    struct fileEntry {
        QString name; //name contains folder.
     //   bool dir;
        unsigned long size;
        QString date;
        QString attrib;
    };
    QList<fileEntry>dirs;

    //functions
    int execute(QString command, QString parameters, QString & result);
    int loadFile(QString fileName);
    int prepareDirDump(QString home);
    int errorMessage(QString text, QString console);
    void visualize();
    void statusBarNormal();

    //ui components
    QLineEdit * leAddress;
    QLineEdit * leLabel;
};

#endif // MAINWINDOW_H
