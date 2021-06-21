//
// Created by michael on 15.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_main_window.h" resolved

#include "main_window.h"
#include "uis/ui_main_window.h"


mainWindow::mainWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainWindow),
        conn(QSqlDatabase::addDatabase("QSQLITE")),
        reviewWindow(nullptr),
        marksWindow(nullptr) {
    ui->setupUi(this);
    connectSlots();

    conn.setDatabaseName("databases/students.sqlite3");
    if (!conn.open()) {
        qDebug() << conn.lastError().text();
    }
}

mainWindow::~mainWindow() {
    delete ui;
    conn.close();
}

void mainWindow::connectSlots() {
    connect(ui->pushButton_pupils, &QPushButton::clicked, this, &mainWindow::openReview);
    connect(ui->pushButton_marks, &QPushButton::clicked, this, &mainWindow::openMarks);
}

void mainWindow::openReview() {
    if (reviewWindow == nullptr) {
        reviewWindow = new studentsReview(this);
    }
    reviewWindow->setDatabase(conn);
    reviewWindow->show();
    this->hide();
}

void mainWindow::openMarks() {
    if (marksWindow == nullptr) {
        marksWindow = new editViewMarks(this);
    }
    marksWindow->setDatabase(conn);
    marksWindow->show();
    this->hide();
}

void mainWindow::openStats() {

}
