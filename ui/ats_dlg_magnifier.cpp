#include "ats_dlg_magnifier.h"
#include "ui_ats_dlg_magnifier.h"

ATSDlgMagnifier::ATSDlgMagnifier(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::ATSDlgMagnifier) {
    ui->setupUi(this);
}

ATSDlgMagnifier::~ATSDlgMagnifier() {
    delete ui;
}
