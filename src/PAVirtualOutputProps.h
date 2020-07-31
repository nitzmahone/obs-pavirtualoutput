//
// Created by mdavis on 7/30/20.
//

#ifndef OBS_PAVIRTUALOUTPUT_PAVIRTUALOUTPUTPROPS_H
#define OBS_PAVIRTUALOUTPUT_PAVIRTUALOUTPUTPROPS_H

#include <QDialog>
#include "pavirtualoutput.h"

namespace Ui {
class PAVirtualOutputProps;
}

class PAVirtualOutputProps : public QDialog
{
    Q_OBJECT

public:
    explicit PAVirtualOutputProps(QWidget *parent = 0);
    ~PAVirtualOutputProps();
    void closeEvent(QCloseEvent *event);
    void saveSettings();

private Q_SLOTS:
    void onStart();
    void onStop();

private:
    Ui::PAVirtualOutputProps *ui;
};


#endif //OBS_PAVIRTUALOUTPUT_PAVIRTUALOUTPUTPROPS_H
