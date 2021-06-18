//
// Created by michael on 17.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_add_student.h" resolved

#include "add_edit_student.h"
#include "uis/ui_add_edit_student.h"


addEditStudent::addEditStudent(QWidget *parent) :
        QDialog(parent), ui(new Ui::addEditStudent), nowType(kAddStudent) {
    ui->setupUi(this);
    connectSlots();

    QSqlQuery q(conn);
    q.exec("SELECT gender_type FROM gender_types");
    while (q.next()) {
        ui->comboBox_gender->addItem(q.value(0).toString());
    }
}

addEditStudent::~addEditStudent() {
    delete ui;
}

void addEditStudent::setDatabase(const QSqlDatabase &other) {
    conn = other;
}

void addEditStudent::save() {
    switch (nowType) {
        case kAddStudent:
            addStudent();
            break;
        case kEditStudent:
            editStudent();
            break;
    }
}

void addEditStudent::connectSlots() {
    connect(ui->btn_save, &QPushButton::clicked, this, &addEditStudent::save);
    connect(ui->btn_cancel, &QPushButton::clicked, this, &addEditStudent::cancel);
}

void addEditStudent::cancel() {
    close();
}

void addEditStudent::prepareToEdit() {
    ui->btn_save->setText("Сохранить");
}

void addEditStudent::addStudent() {
    int res = QMessageBox::question(this, "Отмена", "Вы точно хотите добавить",
                                    QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QSqlQuery query(conn);
        auto[name, studClass, birthDate, startStudyDate, address, parentName, gender] =
        funcs::getStudentInfo(query, ui->lineEdit_name,
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

void addEditStudent::editStudent() {
    int res = QMessageBox::warning(this, "Отмена",
                                   "Вы точно хотите изменить информацию? (Это действие нельзя отменить)",
                                   QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QSqlQuery query(conn);
        auto[name, studClass, birthDate, startStudyDate, address, parentName, gender] =
        funcs::getStudentInfo(query, ui->lineEdit_name,
                              ui->lineEdit_class, ui->dateEdit_birthDate, ui->dateEdit_startStudy,
                              ui->lineEdit_address, ui->lineEdit_parentName, ui->comboBox_gender);

        query.prepare("UPDATE pupils SET pupil_class=?, full_name=?, birth_date=?, start_study_date=?, "
                      "address=?, parent_full_name=?, student_gender=? WHERE student_id=?");
        query.addBindValue(studClass);
        query.addBindValue(name);
        query.addBindValue(birthDate);
        query.addBindValue(startStudyDate);
        query.addBindValue(address);
        query.addBindValue(parentName);
        query.addBindValue(gender);
        query.addBindValue(studentId);
        if (!query.exec()) {
            qDebug() << query.lastError();
        }
        close();
    }
}

void addEditStudent::setNowType(const WindowTypes &type) {
    nowType = type;
    if (nowType == kEditStudent) {
        prepareToEdit();
    } else if (nowType == kAddStudent) {
        prepareToAdd();
    }
}

void addEditStudent::prepareToAdd() {
    ui->btn_save->setText("Добавить");
}

void addEditStudent::setData(const QString &pupilName, const int &pupilClass, const QDate &pupilBirth,
                             const QDate &pupilStart, const QString &pupilAddress, const QString &pupilParent,
                             const int &pupilGender) {
    ui->lineEdit_name->setText(pupilName);
    QSqlQuery q(conn);
    q.prepare("SELECT class_number, class_letter FROM classes WHERE class_id=?");
    q.addBindValue(pupilClass);
    if (!q.exec()) {
        qDebug() << q.lastError();
        return;
    }
    q.next();
    ui->lineEdit_class->setText(q.value(0).toString() + q.value(1).toString());
    ui->dateEdit_birthDate->setDate(pupilBirth);
    ui->dateEdit_startStudy->setDate(pupilStart);
    ui->lineEdit_address->setText(pupilAddress);
    ui->lineEdit_parentName->setText(pupilParent);
    q.clear();
    q.prepare("SELECT gender_type FROM gender_types WHERE gender_type_id=?");
    q.addBindValue(pupilGender);
    if (!q.exec()) {
        qDebug() << q.lastError();
        return;
    }
    q.next();
    ui->comboBox_gender->setCurrentText(q.value(0).toString());
}

void addEditStudent::setStudentId(const int &other) {
    studentId = other;
}
