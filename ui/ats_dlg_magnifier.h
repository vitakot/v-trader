#ifndef ATS_DLG_MAGNIFIER_H
#define ATS_DLG_MAGNIFIER_H

#include <QDialog>

namespace Ui {
    class ATSDlgMagnifier;
}

class ATSDlgMagnifier : public QDialog {
Q_OBJECT

public:
    explicit ATSDlgMagnifier(QWidget *parent = nullptr);

    ~ATSDlgMagnifier();

private:
    Ui::ATSDlgMagnifier *ui;
};

#endif // ATS_DLG_MAGNIFIER_H
