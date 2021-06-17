//
// Created by michael on 15.06.2021.
//

#ifndef SCHOOLPROJECTC_MAIN_WINDOW_H
#define SCHOOLPROJECTC_MAIN_WINDOW_H

#include <QWidget>
#include <QDebug>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include "students_review.h"


QT_BEGIN_NAMESPACE
namespace Ui { class mainWindow; }
QT_END_NAMESPACE

class mainWindow : public QWidget {
Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);

    ~mainWindow() override;

private slots:

    void openStats();

    void openReview();

    void openMarks();

private:
    Ui::mainWindow *ui;
    studentsReview *reviewWindow;

    void connectSlots();

    QSqlDatabase conn;
};


#endif //SCHOOLPROJECTC_MAIN_WINDOW_H
