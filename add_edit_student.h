//
// Created by michael on 17.06.2021.
//

#ifndef SCHOOLPROJECTC_ADD_EDIT_STUDENT_H
#define SCHOOLPROJECTC_ADD_EDIT_STUDENT_H

#include <QDialog>
#include <QtSql/QSqlDatabase>
#include <QMessageBox>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include "useful_funcs.h"

enum WindowTypes {
    kAddStudent,
    kEditStudent,
};

QT_BEGIN_NAMESPACE
namespace Ui { class addEditStudent; }
QT_END_NAMESPACE

class addEditStudent : public QDialog {
Q_OBJECT

public:
    explicit addEditStudent(QWidget *parent = nullptr);

    ~addEditStudent() override;

public slots:

    void setDatabase(const QSqlDatabase &);

    void setStudentId(const int &other);

    void save();

    void cancel();

    void setData(const QString &pupilName,
                 const int &pupilClass,
                 const QDate &pupilBirth,
                 const QDate &pupilStart,
                 const QString &pupilAddress,
                 const QString &pupilParent,
                 const int &pupilGender);

    void setNowType(const WindowTypes &type);

private slots:

    void prepareToEdit();

    void prepareToAdd();

private:
    Ui::addEditStudent *ui;
    QSqlDatabase conn;
    WindowTypes nowType;
    int studentId{};

    void connectSlots();

    void addStudent();

    void editStudent();
};


#endif //SCHOOLPROJECTC_ADD_EDIT_STUDENT_H
