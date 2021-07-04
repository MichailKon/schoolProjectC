//
// Created by michael on 16.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_students_review.h" resolved

#include "students_review.h"
#include "uis/ui_students_review.h"


studentsReview::studentsReview(QWidget *parent) :
        QWidget(), parent(parent), ui(new Ui::studentsReview) {
    ui->setupUi(this);

    connectSlots();

    ui->spinBox_year->blockSignals(true);
    ui->lineEdit_classLetter->blockSignals(true);
    ui->lineEdit_classNum->blockSignals(true);
    ui->spinBox_year->setValue(kAnyYear);
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setHorizontalHeaderLabels({
                                                       "Класс", "Дата рождения",
                                                       "Дата начала обучения", "Адрес", "ФИО родителя", "Пол"
                                               });
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotCustomMenuRequested(QPoint)));

    QSqlQuery q("SELECT gender_type FROM gender_types");
    q.exec();
    ui->comboBox_gender->addItem(kAnyGender);
    while (q.next()) {
        ui->comboBox_gender->addItem(q.value(0).toString());
    }
}

studentsReview::~studentsReview() {
    delete ui;
}

void studentsReview::setDatabase(const QSqlDatabase &newConn) {
    conn = newConn;
}

void studentsReview::cancel() {
    parent->show();
    hide();
}

void studentsReview::connectSlots() {
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &studentsReview::cancel);
    connect(ui->pushButton_printStudent, &QPushButton::clicked, this, &studentsReview::printStudents);
}

void studentsReview::printStudents() {
    ui->tableWidget->setVisible(false);
    displayedStudents.clear();

    QSqlQuery query = generateQuery();
    query.exec();

    QStringList columns;
    {
        QSqlRecord localRecord = query.record();
        for (int var = 0; var < localRecord.count(); var++) {
            QString fieldName = localRecord.fieldName(var);
            columns.append(fieldName);
        }
    }
    ui->tableWidget->setRowCount(0);

    QStringList fullNames;
    while (query.next()) {
        displayedStudents.push_back(query.value(0).toInt());
        int nowRow = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(nowRow + 1);

        QStringList nowValues;
        for (int i = 0; i < columns.size(); i++) {
            nowValues.append(query.value(i).toString());
        }
        auto data = funcs::compress(columns, nowValues);
        data["class"] = data["class_number"] + data["class_letter"];
        data.erase(data.find("class_number"));
        data.erase(data.find("class_letter"));
        fullNames.append(data["full_name"]);
        data.erase(data.find("full_name"));
        // place data
        for (int i = 0; i < str2column.size(); i++) {
            if (str2column[i].second == kText) {
                QTableWidgetItem *newItem;
                newItem = new QTableWidgetItem(data[str2column[i].first]);
                ui->tableWidget->setItem(nowRow, i, newItem);
            } else if (str2column[i].second == kDate) {
                QTableWidgetItem *newItem;
                QDate now = QDate::fromString(data[str2column[i].first], "yyyyMMdd");
                newItem = new QTableWidgetItem(now.toString());
                ui->tableWidget->setItem(nowRow, i, newItem);
            }
        }
    }
    ui->tableWidget->setVerticalHeaderLabels(fullNames);

    ui->tableWidget->setVisible(true);
}

QSqlQuery studentsReview::generateQuery() {
    QStringList data;
    QString q = "SELECT student_id, class_number, class_letter, full_name, birth_date, start_study_date, "
                "address, parent_full_name, gender_type, kicked FROM pupils "
                "INNER JOIN gender_types gt on pupils.student_gender = gt.gender_type_id "
                "INNER JOIN classes c on pupils.pupil_class = c.class_id ";
    QPair<QString, QStringList> classInfo =
            funcs::getPupilClassQuery("pupil_class", ui->lineEdit_classNum->text(), ui->lineEdit_classLetter->text());

    q += " WHERE " + classInfo.first + " ";
    for (const auto &i : classInfo.second) {
        data.append(i);
    }

    if (ui->comboBox_gender->currentText() != kAnyGender) {
        q += " AND student_gender=(SELECT gender_type_id FROM gender_types WHERE gender_type=?)";
        data.append(ui->comboBox_gender->currentText());
    }
    if (ui->spinBox_year->value() != kAnyYear) {
        q += " AND strftime('%Y', birth_date) == ?";
        data.append(QString::number(ui->spinBox_year->value()));
    }
    q += " AND kicked=?";
    data.append(QString::number(ui->comboBox_isKicked->currentIndex()));
    q += " ORDER BY full_name";

    QSqlQuery res(conn);
    res.prepare(q);
    for (auto &i : data) {
        res.addBindValue(i);
    }
    return res;
}

void studentsReview::slotCustomMenuRequested(QPoint pos) {
    auto *menu = new QMenu(this);
    auto *editStudent = new QAction("Редактировать", this);
    auto *kickStudent = new QAction("Исключить/вернуть", this);
    auto *deleteStudent = new QAction("Полностью удалить", this);
    auto *addStudent = new QAction("Добавить ученика", this);
    connect(editStudent, SIGNAL(triggered()),
            this, SLOT(slotEditStudent()));
    connect(kickStudent, SIGNAL(triggered()),
            this, SLOT(slotRemoveStudent()));
    connect(deleteStudent, SIGNAL(triggered()),
            this, SLOT(slotDeleteStudent()));
    connect(addStudent, SIGNAL(triggered()),
            this, SLOT(slotAddStudent()));
    menu->addAction(editStudent);
    menu->addAction(kickStudent);
    menu->addAction(deleteStudent);
    menu->addAction(addStudent);
    menu->popup(ui->tableWidget->viewport()->mapToGlobal(pos));
}

void studentsReview::slotEditStudent() {
    int row = ui->tableWidget->selectionModel()->currentIndex().row();
    if (row < 0 || row >= displayedStudents.size()) return;
    auto adding = new addEditStudent(this);
    adding->setDatabase(conn);
    adding->setNowType(kEditStudent);
    QSqlQuery q(conn);
    q.prepare("SELECT full_name, pupil_class, birth_date, start_study_date,"
              "address, parent_full_name, student_gender FROM pupils "
              "WHERE student_id=?");
    q.addBindValue(displayedStudents[row]);
    if (!q.exec()) {
        qDebug() << q.lastError();
        return;
    }
    q.next();
    adding->setData(q.value(0).toString(), q.value(1).toInt(),
                    QDate::fromString(q.value(2).toString(), "yyyy-MM-dd"),
                    QDate::fromString(q.value(3).toString(), "yyyy-MM-dd"),
                    q.value(4).toString(), q.value(5).toString(), q.value(6).toInt());
    adding->setStudentId(displayedStudents[row]);
    adding->exec();
    printStudents();
}

void studentsReview::slotRemoveStudent() {
    int row = ui->tableWidget->selectionModel()->currentIndex().row();
    if (row < 0 || row >= displayedStudents.size()) return;
    if (QMessageBox::warning(this,
                             "Исключить/вернуть",
                             "Вы уверены, что хотите исключить/удалить этого ученика(цу)",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(conn);
        q.prepare("UPDATE pupils SET kicked=NOT kicked WHERE student_id=?");
        q.addBindValue(displayedStudents[row]);
        q.exec();
        printStudents();
    }
}

void studentsReview::slotDeleteStudent() {
    int row = ui->tableWidget->selectionModel()->currentIndex().row();
    if (row < 0 || row >= displayedStudents.size()) return;
    int id = displayedStudents[row];
    if (QMessageBox::warning(this,
                             "Исключить/вернуть",
                             "Вы уверены, что хотите исключить/удалить этого ученика(цу)\n"
                             "Это действие нельзя отменить!",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        QSqlQuery q(conn);
        q.prepare("DELETE FROM pupils WHERE student_id=?");
        q.addBindValue(id);
        if (!q.exec()) {
            qDebug() << q.lastError();
        }

        q.clear();
        q.prepare("DELETE FROM marks WHERE which_mark=?");
        q.addBindValue(id);
        if (!q.exec()) {
            qDebug() << q.lastError();
        }

        q.clear();
        q.prepare("DELETE FROM diplomas WHERE which_diploma=?");
        q.addBindValue(id);
        if (!q.exec()) {
            qDebug() << q.lastError();
        }
        printStudents();
    }
}

void studentsReview::slotAddStudent() {
    auto adding = new addEditStudent(this);
    adding->setDatabase(conn);
    adding->setNowType(kAddStudent);
    adding->exec();
    printStudents();
}
