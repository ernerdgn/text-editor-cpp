#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>

namespace Ui {
class SearchDialog;
}

class SearchDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SearchDialog(QWidget *parent = nullptr);
    ~SearchDialog();

    //--//
    QString getFindText();

signals:
    void findClicked(const QString &text);
    void replaceClicked(const QString &findText, const QString &replaceText);
    void replaceAllClicked(const QString &findText, const QString &replaceText);

private slots:
    void on_findNextButton_clicked();
    void on_replaceButton_clicked();
    void on_replaceAllButton_clicked();

private:
    Ui::SearchDialog *ui;
};

#endif // SEARCHDIALOG_H
