#include "GetInputDialog.h"
#include "ui_GetInputDialog.h"

GetInputDialog::GetInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetInputDialog)
{
    ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

GetInputDialog::~GetInputDialog()
{
    delete ui;
}

void GetInputDialog::SetTitle(QString title)
{
    setWindowTitle(title);
}

QString GetInputDialog::GetInput()
{
    return ui->textInput->text();
}

