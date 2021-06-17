//
// Created by michael on 17.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_add_student.h" resolved

#include "add_student.h"
#include "uis/ui_add_student.h"


addStudent::addStudent(QWidget *parent) :
        QDialog(parent), ui(new Ui::addStudent) {
    ui->setupUi(this);
    connectSlots();

    QSqlQuery q(conn);
    q.exec("SELECT gender_type FROM gender_types");
    while (q.next()) {
        ui->comboBox_gender->addItem(q.value(0).toString());
    }
}

addStudent::~addStudent() {
    delete ui;
}

void addStudent::setDatabase(const QSqlDatabase &other) {
    conn = other;
}

void addStudent::add() {
    int res = QMessageBox::question(this, "Отмена", "Вы точно хотите добавить",
                                    QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QSqlQuery query(conn);
        auto[name, studClass, birthDate, startStudyDate, address, parentName, gender] =
        funcs::getStudentInfo(this, query, ui->lineEdit_name,
                              ui->lineEdit_class, ui->dateEdit_birthDate, ui->dateEdit_startStudy,
                              ui->lineEdit_address, ui->lineEdit_parentName, ui->comboBox_gender);

        query.prepare("INSERT INTO pupils (full_name, pupil_class, birth_date, start_study_date, address,"
                      "parent_full_name, student_gender) VALUES (?,?,?,?,?,?,?)");
        query.addBindValue(QVariant(name));
        query.addBindValue(studClass);
        query.addBindValue(birthDate);
        query.addBindValue(startStudyDate);
        query.addBindValue(address);
        query.addBindValue(parentName);
        query.addBindValue(gender);
        query.exec();
        close();
    }
}

void addStudent::connectSlots() {
    connect(ui->btn_add, &QPushButton::clicked, this, &addStudent::add);
}
