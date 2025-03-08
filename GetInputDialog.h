#ifndef GETINPUTDIALOG_H
#define GETINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class GetInputDialog;
}

class GetInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetInputDialog(QWidget *parent = nullptr);
    ~GetInputDialog();

    void SetTitle(QString title);

    QString GetInput();

private:
    Ui::GetInputDialog *ui;
};

#endif // GETINPUTDIALOG_H
