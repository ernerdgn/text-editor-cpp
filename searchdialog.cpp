#include "searchdialog.h"
#include "ui_searchdialog.h"

SearchDialog::SearchDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SearchDialog)
{
    ui->setupUi(this);
}

SearchDialog::~SearchDialog()
{
    delete ui;
}

QString SearchDialog::getFindText()
{
    return ui->findLineEdit->text();
}

void SearchDialog::on_findNextButton_clicked()
{
    QString findText = ui->findLineEdit->text();
    emit findClicked(findText);
}

void SearchDialog::on_replaceButton_clicked()
{
    emit replaceClicked(ui->findLineEdit->text(), ui->replaceLineEdit->text());
}

void SearchDialog::on_replaceAllButton_clicked()
{
    emit replaceAllClicked(ui->findLineEdit->text(),
                           ui->replaceLineEdit->text());
}
