//
// Created by mdavis on 7/30/20.
//

#include "PAVirtualOutputProps.h"
#include "ui_PAVirtualOutputProps.h"

PAVirtualOutputProps::PAVirtualOutputProps(QWidget *parent) : QDialog(parent), ui(new Ui::PAVirtualOutputProps)
{
    ui->setupUi(this);
    connect(ui->pushButton_start,SIGNAL(clicked(bool)), this, SLOT(onStart()));
    connect(ui->pushButton_stop,SIGNAL(clicked(bool)), this, SLOT(onStop()));
}

PAVirtualOutputProps::~PAVirtualOutputProps()
{
    saveSettings();
    delete ui;
}

void PAVirtualOutputProps::closeEvent(QCloseEvent *event)
{
    saveSettings();
}

void PAVirtualOutputProps::saveSettings()
{
    // FIXME: implement
}

void PAVirtualOutputProps::onStart()
{
    pavirtualoutput_enable();
}

void PAVirtualOutputProps::onStop()
{
    pavirtualoutput_disable();
}

