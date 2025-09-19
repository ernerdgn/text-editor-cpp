#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "searchdialog.h"

#include <QAction>  //
#include <QFileInfo>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <fstream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //--//

    // open/load file
    //test.txt
    //editorOpen("C:/Users/erene/Desktop/isler/software/text-editor/test.txt", this->state);
    // QStringList args = QCoreApplication::arguments();
    // if (args.size() > 1)
    // {
    //     QString filePath = args[1];
    //     editorOpen(filePath.toStdString().c_str(), this->state);
    //     // if this->state is empty, opening a blank txt page
    // }

    // QString documentText;
    // for (const auto& line : this->state.lines)
    // {
    //     documentText += QString::fromStdString(line);
    //     documentText += "\n";
    // }

    // if (!documentText.isEmpty()) documentText.chop(1); // remove extra line
    // ui->textEdit->setText(documentText);
    // updateCursorPosition();

    // create child objs
    findDialog = new SearchDialog(this);

    //label for status bar
    cursorPosLabel = new QLabel(this);
    statusBar()->addPermanentWidget(cursorPosLabel);

    // connect the signal and slot
    //save and cursor pos info
    connect(ui->action_Save, &QAction::triggered, this, &MainWindow::saveDocument);
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged, this, &MainWindow::updateCursorPosition);

    // search & change
    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(findDialog, &SearchDialog::findClicked, this, &MainWindow::findText);
    connect(findDialog, &SearchDialog::replaceClicked, this, &MainWindow::replaceText);
    connect(findDialog, &SearchDialog::replaceAllClicked, this, &MainWindow::replaceAllText);
    // //find
    // connect(findDialog, &SearchDialog::findNextClicked, this, &MainWindow::findText);
    // connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    // //replacement
    // connect(findDialog, &SearchDialog::replaceClicked, this, &MainWindow::replaceText);
    // connect(findDialog, &SearchDialog::replaceAllClicked, this, &MainWindow::replaceAllText);

    // edit slots
    connect(ui->actionUndo, &QAction::triggered, ui->textEdit, &QTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->textEdit, &QTextEdit::redo);
    connect(ui->actionCopy, &QAction::triggered, ui->textEdit, &QTextEdit::copy);
    connect(ui->actionCut, &QAction::triggered, ui->textEdit, &QTextEdit::cut);
    connect(ui->actionPaste, &QAction::triggered, ui->textEdit, &QTextEdit::paste);

    // //combine lines to single string
    // QString documentText;
    // for (const auto& line : this->state.lines)
    // {
    //     documentText += QString::fromStdString(line);
    //     documentText += "\n";
    // }

    // //text edit
    // ui->textEdit->setText(documentText);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile(const QString &filePath)
{
    this->state.file_name = filePath.toStdString();
    setWindowTitle(filePath.isEmpty() ? "Untitled" : QFileInfo(filePath).fileName());

    this->state.lines.clear();

    if (!filePath.isEmpty())
    {
        QFile file(filePath);

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream in(&file);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                this->state.lines.push_back(line.toStdString());
            }
            file.close();
        }
    }

    if (this->state.lines.empty()) this->state.lines.push_back("");

    QString documentText;
    for (const auto& line : this->state.lines)
    {
        documentText += QString::fromStdString(line);
        documentText += "\n";
    }
    if (!documentText.isEmpty()) documentText.chop(1);

    ui->textEdit->setText(documentText);
    updateCursorPosition();
}

void MainWindow::saveDocument()
{
    if (this->state.file_name.empty())
    {
        // save as
        QString filePath = QFileDialog::getSaveFileName(this, "Save File As");
        if (filePath.isEmpty()) return;

        this->state.file_name = filePath.toStdString();
        setWindowTitle(QFileInfo(filePath).fileName());
    }

    QString documentText = ui->textEdit->toPlainText();
    this->state.lines.clear();
    QStringList lines = documentText.split('\n');
    for (const QString& line : lines) this->state.lines.push_back(line.toStdString());

    editorSave(this->state);
    statusBar()->showMessage("File saved!", 2000);
}

void MainWindow::updateCursorPosition()
{
    QTextCursor cursor = ui->textEdit->textCursor();

    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber();

    cursorPosLabel->setText(QString("Line %1, Column: %2").arg(line).arg(column));
}

void MainWindow::showFindDialog()
{
    findDialog->show();
    findDialog->activateWindow();
}
// void MainWindow::on_actionFind_triggered()
// {
//     SearchDialog findDialog(this);
//     //findDialog.exec();
//     if (findDialog.exec() == QDialog::Accepted)
//     {
//         QString findText = findDialog.getFindText();
//         if (!findText.isEmpty()) this->findText(findText);
//     }

// }

void MainWindow::findText(const QString& text)
{
    if (!ui->textEdit->find(text)) statusBar()->showMessage("Text not found", 2000);
}

void MainWindow::replaceText(const QString &findText, const QString &replaceText)
{
    if (ui->textEdit->find(findText))
        ui->textEdit->textCursor().insertText(replaceText);
    else
        statusBar()->showMessage("Text not found", 2000);
    // if(ui->textEdit->textCursor().hasSelection())
    //     ui->textEdit->textCursor().insertText(replaceText);

    // findText(findDialog->getFindText());
}

void MainWindow::replaceAllText(const QString &findText, const QString &replaceText)
{
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    ui->textEdit->setTextCursor(cursor);

    int replacement_count = 0;

    while (ui->textEdit->find(findText))
    {
        ui->textEdit->textCursor().insertText(replaceText);
        replacement_count++;
    }

    statusBar()->showMessage(QString("Made %1 replacements").arg(replacement_count), 2000);
}
