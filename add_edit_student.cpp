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
    QString name = ui->lineEdit_name->text();

    QSqlQuery query(conn);
    query.prepare("SELECT class_id FROM classes WHERE class_number=? AND class_letter=?");
    query.addBindValue(ui->lineEdit_classNumber->text());
    query.addBindValue(ui->lineEdit_classLetter->text());
    if (!query.exec()) {
        funcs::dataBaseError(this, query);
        return;
    }
    query.next();
    if (!query.isValid()) {
        QMessageBox::critical(this,
                              "Проблемы с классом",
                              "В базе данных нет класса " + ui->lineEdit_classNumber->text() +
                              ui->lineEdit_classLetter->text(),
                              QMessageBox::Ok);
        return;
    }
    int classId = query.value(0).toInt();

    QString birth = ui->dateEdit_birthDate->date().toString("yyyy-MM-dd");
    QString start = ui->dateEdit_startStudy->date().toString("yyyy-MM-dd");
    QString address = ui->lineEdit_address->text();
    QString parent = ui->lineEdit_parentName->text();

    query.clear();
    query.prepare("SELECT gender_type_id FROM gender_types WHERE gender_type=?");
    query.addBindValue(ui->comboBox_gender->currentText());
    if (!query.exec()) {
        funcs::dataBaseError(this, query);
        return;
    }
    query.next();
    if (!query.isValid()) {
        QMessageBox::critical(this,
                              "Проблемы с полом",
                              "В базе данных нет пола " + ui->comboBox_gender->currentText(),
                              QMessageBox::Ok);
        return;
    }
    int gender = query.value(0).toInt();

    switch (nowType) {
        case kAddStudent:
            addStudent(name, classId, birth, start, address, parent, gender);
            break;
        case kEditStudent:
            editStudent(name, classId, birth, start, address, parent, gender);
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

void addEditStudent::addStudent(const QString &name, const int &studClass, const QString &birthDate,
                                const QString &startStudyDate, const QString &address,
                                const QString &parentName, const int &gender) {
    int res = QMessageBox::question(this, "Отмена", "Вы точно хотите добавить",
                                    QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QSqlQuery query(conn);

        query.prepare("INSERT INTO pupils (full_name, pupil_class, birth_date, start_study_date, address,"
                      "parent_full_name, student_gender) VALUES (?,?,?,?,?,?,?)");
        query.addBindValue(QVariant(name));
        query.addBindValue(studClass);
        query.addBindValue(birthDate);
        query.addBindValue(startStudyDate);
        query.addBindValue(address);
        query.addBindValue(parentName);
        query.addBindValue(gender);
        if (!query.exec()) {
            funcs::dataBaseError(this, query);
        }
        close();
    }
}

void addEditStudent::editStudent(const QString &name, const int &studClass, const QString &birthDate,
                                 const QString &startStudyDate, const QString &address,
                                 const QString &parentName, const int &gender) {
    int res = QMessageBox::warning(this, "Отмена",
                                   "Вы точно хотите изменить информацию? (Это действие нельзя отменить)",
                                   QMessageBox::Yes, QMessageBox::No);
    if (res == QMessageBox::Yes) {
        QSqlQuery query(conn);

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
            funcs::dataBaseError(this, query);
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
        funcs::dataBaseError(this, q);
        return;
    }
    q.next();
    if (!q.isValid()) {
        QMessageBox::critical(this,
                              "Проблемы с учеником",
                              "В базе данных нет класса этого ученика",
                              QMessageBox::Ok);
        cancel();
    }
    ui->lineEdit_classNumber->setText(q.value(0).toString());
    ui->lineEdit_classLetter->setText(q.value(1).toString());
    ui->dateEdit_birthDate->setDate(pupilBirth);
    ui->dateEdit_startStudy->setDate(pupilStart);
    ui->lineEdit_address->setText(pupilAddress);
    ui->lineEdit_parentName->setText(pupilParent);
    q.clear();
    q.prepare("SELECT gender_type FROM gender_types WHERE gender_type_id=?");
    q.addBindValue(pupilGender);
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    q.next();
    if (!q.isValid()) {
        QMessageBox::critical(this,
                              "Проблемы с учеником",
                              "В базе данных нет пола этого ученика",
                              QMessageBox::Ok);
        cancel();
    }
    ui->comboBox_gender->setCurrentText(q.value(0).toString());
}

void addEditStudent::setStudentId(const int &other) {
    studentId = other;
}
