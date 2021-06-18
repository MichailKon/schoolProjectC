//
// Created by michael on 16.06.2021.
//

#ifndef SCHOOLPROJECTC_STUDENTS_REVIEW_H
#define SCHOOLPROJECTC_STUDENTS_REVIEW_H

#include <QWidget>
#include <QMap>
#include <QDebug>
#include <QPair>
#include <QDate>
#include <QMenu>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include "useful_funcs.h"
#include "add_edit_student.h"

enum ColumnTypes {
    kText,
    kDate,
};

const QVector<QPair<QString, ColumnTypes>> str2column = {
        {"class",            kText},
        {"birth_date",       kDate},
        {"start_study_date", kDate},
        {"address",          kText},
        {"parent_full_name", kText},
        {"gender_type",      kText},
};

const QString kAnyGender = "Любой";
const int kAnyYear = -1;

QT_BEGIN_NAMESPACE
namespace Ui { class studentsReview; }
QT_END_NAMESPACE

class studentsReview : public QWidget {
Q_OBJECT

public:
    explicit studentsReview(QWidget *parent = nullptr);

    ~studentsReview() override;

    void printStudents();

private slots:

    void cancel();

    void slotEditStudent();

    void slotRemoveStudent();

    void slotDeleteStudent();

    void slotAddStudent();

    void slotCustomMenuRequested(QPoint pos);

public slots:

    void setDatabase(const QSqlDatabase &conn);

private:
    Ui::studentsReview *ui;
    QSqlDatabase conn;
    QWidget *parent;
    QVector<int> displayedStudents;

    void connectSlots();

    QSqlQuery generateQuery();
};


#endif //SCHOOLPROJECTC_STUDENTS_REVIEW_H
