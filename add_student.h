//
// Created by michael on 17.06.2021.
//

#ifndef SCHOOLPROJECTC_ADD_STUDENT_H
#define SCHOOLPROJECTC_ADD_STUDENT_H

#include <QDialog>
#include <QtSql/QSqlDatabase>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include "useful_funcs.h"


QT_BEGIN_NAMESPACE
namespace Ui { class addStudent; }
QT_END_NAMESPACE

class addStudent : public QDialog {
Q_OBJECT

public:
    explicit addStudent(QWidget *parent = nullptr);

    ~addStudent() override;

public slots:

    void setDatabase(const QSqlDatabase &);

    void add();

private:
    Ui::addStudent *ui;
    QSqlDatabase conn;

    void connectSlots();
};


#endif //SCHOOLPROJECTC_ADD_STUDENT_H
