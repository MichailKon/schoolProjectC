//
// Created by michael on 15.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_main_window.h" resolved

#include "main_window.h"
#include "uis/ui_main_window.h"


mainWindow::mainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainWindow) {
    ui->setupUi(this);
}

mainWindow::~mainWindow() {
    delete ui;
}

void mainWindow::connectSlots() {

}

