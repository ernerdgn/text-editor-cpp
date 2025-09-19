#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "editor.h"
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SearchDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadFile(const QString &filePath);

private slots:
    void saveDocument();
    void updateCursorPosition();
    //void on_actionFind_triggered();
    void showFindDialog();

    void findText(const QString& text);
    void replaceText(const QString &findText, const QString &replaceText);
    void replaceAllText(const QString &findText, const QString &replaceText);

private:
    Ui::MainWindow *ui;
    editorState state;
    QLabel *cursorPosLabel;
    SearchDialog *findDialog;
};
#endif // MAINWINDOW_H
