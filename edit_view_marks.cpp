//
// Created by michael on 20.06.2021.
//

// You may need to build the project (run Qt uic code generator) to get "ui_edit_view_marks.h" resolved

#include "edit_view_marks.h"
#include "uis/ui_edit_view_marks.h"

editViewMarks::editViewMarks(QWidget *parent) :
    QWidget(), parent(parent), ui(new Ui::editViewMarks) {
    ui->setupUi(this);

    ui->tableWidget_viewMarks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget_editSubjects->setContextMenuPolicy(Qt::CustomContextMenu);
    prepare();
    connectSlots();
}

editViewMarks::~editViewMarks() {
    delete ui;
}

void editViewMarks::prepare() {
    QSqlQuery q(conn);
    if (!q.exec("SELECT DISTINCT type_mark FROM mark_types ORDER BY type_mark")) {
        funcs::dataBaseError(this, q);
        cancel();
    }
    while (q.next()) {
        ui->comboBox_type->addItem(q.value(0).toString());
        ui->comboBox_type2->addItem(q.value(0).toString());
    }
    // make kicked
    {
        QMapIterator it(convertKicked);
        while (it.hasNext()) {
            it.next();
            ui->comboBox_isKicked->addItem(it.key());
            ui->comboBox_isKicked2->addItem(it.key());
        }
    }
    // make shown
    {
        QMapIterator it(convertShown);
        while (it.hasNext()) {
            it.next();
            ui->comboBox_shownSubjects->addItem(it.key());
            ui->comboBox_shownSubjects2->addItem(it.key());
        }
    }
    // add numbers
    if (!q.exec("SELECT DISTINCT class_number FROM classes ORDER BY class_number")) {
        funcs::dataBaseError(this, q);
        cancel();
    }
    ui->comboBox_classNum3->clear();
    while (q.next()) {
        ui->comboBox_classNum3->addItem(q.value(0).toString());
    }
    // add letters
    if (!q.exec("SELECT DISTINCT class_letter FROM classes ORDER BY class_letter")) {
        funcs::dataBaseError(this, q);
        cancel();
    }
    ui->comboBox_classLetter3->clear();
    ui->comboBox_classLetter3->addItem(allClasses);
    while (q.next()) {
        ui->comboBox_classLetter3->addItem(q.value(0).toString());
    }
    printSubjects();
}

void editViewMarks::connectSlots() {
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &editViewMarks::cancel);
    connect(ui->pushButton_printView, &QPushButton::clicked, this, &editViewMarks::printViewMarks);
    connect(ui->pushButton_printEdit, &QPushButton::clicked, this, &editViewMarks::printEditMarks);
    connect(ui->tableWidget_editMarks, &QTableWidget::itemChanged, this, &editViewMarks::editMark);
    connect(ui->tableWidget_editSubjects, &QTableWidget::customContextMenuRequested,
            this, &editViewMarks::subjectsEditMenuRequested);
    connect(ui->comboBox_classNum3, &QComboBox::currentTextChanged, this, &editViewMarks::printSubjects);
    connect(ui->comboBox_classLetter3, &QComboBox::currentTextChanged, this, &editViewMarks::printSubjects);
}

void editViewMarks::cancel() {
    parent->show();
    this->hide();
}

void editViewMarks::setDatabase(const QSqlDatabase &other) {
    conn = other;
}

void editViewMarks::printViewMarks() {
    ui->tableWidget_viewMarks->setRowCount(0);
    ui->tableWidget_viewMarks->setColumnCount(0);

    QSqlQuery q = getStudentQuery(ui->lineEdit_classNum->text(), ui->lineEdit_classLetter->text(),
                                  ui->comboBox_year->currentText(), ui->comboBox_type->currentText(),
                                  convertShown.find(ui->comboBox_shownSubjects->currentText()).value(),
                                  convertKicked.find(ui->comboBox_isKicked->currentText()).value());
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    QStringList columns = funcs::getColumns(q);
    if (ui->comboBox_property->currentText() != allProps) {
        QStringList pupils;
        auto [cnt, mark] = *propTypes.find(ui->comboBox_property->currentText());
        QMap<QString, int> cntMarks;
        ui->tableWidget_viewMarks->setColumnCount(cnt);
        for (int i = 1; i <= cnt; i++) {
            auto *item = new QTableWidgetItem("Предмет " + QString::number(i));
            ui->tableWidget_viewMarks->setHorizontalHeaderItem(i - 1, item);
        }

        QMap<int, int> mustCol;
        int mt = propTypes.find(ui->comboBox_property->currentText())->markType();
        int mc = propTypes.find(ui->comboBox_property->currentText())->cntMarks();
        while (q.next()) {
            QStringList nowValues;
            for (int i = 0; i < columns.size(); i++) {
                nowValues.append(q.value(i).toString());
            }
            auto data = funcs::compress(columns, nowValues);
            if (data["mark_value"].toInt() == mt) {
                cntMarks[data["full_name"]]++;
            }
        }

        QMapIterator it(cntMarks);
        while (it.hasNext()) {
            it.next();
            if (it.value() == mc) {
                pupils.append(it.key());
            }
        }

        ui->tableWidget_viewMarks->setRowCount(pupils.size());
        qSort(pupils);
        ui->tableWidget_viewMarks->setVerticalHeaderLabels(pupils);

        if (!q.first()) return;
        do {
            QStringList nowValues;
            for (int i = 0; i < columns.size(); i++) {
                nowValues.append(q.value(i).toString());
            }
            auto data = funcs::compress(columns, nowValues);
            if (data["mark_value"].toInt() == mt && pupils.contains(data["full_name"])) {
                int row = pupils.indexOf(data["full_name"]);
                int col = mustCol[data["which_mark"].toInt()]++;
                auto *newItem = new QTableWidgetItem(data["subject_type"]);
                ui->tableWidget_viewMarks->setItem(row, col, newItem);
            }
        } while (q.next());
    } else {
        QStringList curSubjects;
        QMap<int, int> pupil2row;
        QVector<QPair<QString, int>> pupils;
        {
            QSet<QPair<QString, int>> pupilsSet;
            while (q.next()) {
                QStringList nowValues;
                for (int i = 0; i < columns.size(); i++) {
                    nowValues.append(q.value(i).toString());
                }
                auto data = funcs::compress(columns, nowValues);
                pupilsSet.insert({data["full_name"], data["which_mark"].toInt()});
                if (!curSubjects.contains(data["subject_type"])) {
                    curSubjects.append(data["subject_type"]);
                }
            }
            ui->tableWidget_viewMarks->setRowCount(pupilsSet.size());
            for (auto &i: pupilsSet) {
                pupils.append(i);
            }
            qSort(pupils);
            for (int i = 0; i < pupils.size(); i++) {
                auto *newItem = new QTableWidgetItem(pupils[i].first);
                ui->tableWidget_viewMarks->setVerticalHeaderItem(i, newItem);
                pupil2row[pupils[i].second] = i;
            }
        }

        qSort(curSubjects);
        ui->tableWidget_viewMarks->setColumnCount(curSubjects.size());
        ui->tableWidget_viewMarks->setHorizontalHeaderLabels(curSubjects);

        q.exec();
        while (q.next()) {
            QStringList nowValues;
            for (int i = 0; i < columns.size(); i++) {
                nowValues.append(q.value(i).toString());
            }
            auto data = funcs::compress(columns, nowValues);
            int row = pupil2row[data["which_mark"].toInt()];
            int col = curSubjects.indexOf(data["subject_type"]);
            auto *newItem = new QTableWidgetItem(data["mark_value"]);
            ui->tableWidget_viewMarks->setItem(row, col, newItem);
        }
    }
    ui->tableWidget_viewMarks->resizeColumnsToContents();
}

QSqlQuery editViewMarks::getStudentQuery(const QString &num, const QString &let, const QString &year,
                                         const QString &markType, ShownSubjectsType shownSubject, KickedType kicked,
                                         const QCheckBox *haveMarks) {
    QSqlQuery res(conn);
    QString query;
    QStringList data;
    bool marks = (haveMarks == nullptr || haveMarks->isChecked());

    QString tempQuery = "NOT EXISTS "
                        "(SELECT null FROM marks m "
                        "WHERE m.which_mark = student_id";
    QStringList tempData;

    if (!marks) {
        query += "SELECT full_name, student_id as which_mark, pupil_class as actual_class FROM pupils ";
    } else {
        query += "SELECT full_name, subject_type, mark_value, mark_id, which_mark, "
                 "subject, mark_year, type, actual_class, mark_value_id "
                 "FROM marks "
                 "INNER JOIN pupils ON which_mark = student_id "
                 "INNER JOIN mark_values ON value = mark_value_id "
                 "INNER JOIN subject_types ON subject = subject_id";
    }
    if (shownSubject == kShowExist && marks) {
        query += " WHERE EXISTS(SELECT * FROM class_subject WHERE subject_id2=subject AND class_id2=actual_class)";
    } else {
        query += " WHERE 1=1";
    }

    QPair<QString, QStringList> classInfo = funcs::getPupilClassQuery("actual_class", num, let);
    data.append(classInfo.second);
    query += " AND " + classInfo.first;

    classInfo = funcs::getPupilClassQuery("m.actual_class", num, let);
    tempData.append(classInfo.second);
    tempQuery += " AND " + classInfo.first;

    if (marks) {
        query += " AND mark_year=?";
        data.append(year);

        query += " AND type=(SELECT type_mark_id FROM mark_types WHERE type_mark=?)";
        data.append(markType);
    } else {
        tempQuery += " AND mark_year=?";
        tempData.append(year);

        tempQuery += " AND type=(SELECT type_mark_id FROM mark_types WHERE type_mark=?)";
        tempData.append(markType);
    }

    if (kicked == kKicked) {
        query += " AND kicked=1";
    } else if (kicked == kStudying) {
        query += " AND kicked=0";
    }

    if (!marks) {
        query += " AND " + tempQuery + ")";
        data += tempData;
    }

    res.prepare(query);
    for (const auto &i: data) {
        res.addBindValue(i);
    }

    return res;
}

void editViewMarks::printEditMarks() {
    ui->tableWidget_editMarks->blockSignals(true);
    ui->tableWidget_editMarks->setRowCount(0);
    ui->tableWidget_editMarks->setColumnCount(0);
    editStudentIds.clear(), editSubjectIds.clear();

    QSqlQuery q = getStudentQuery(ui->comboBox_classNum2->currentText(), ui->comboBox_classLetter2->currentText(),
                                  ui->comboBox_year2->currentText(), ui->comboBox_type2->currentText(),
                                  convertShown.find(ui->comboBox_shownSubjects2->currentText()).value(),
                                  convertKicked.find(ui->comboBox_isKicked2->currentText()).value(),
                                  ui->checkBox);
    if (!q.exec()) {
        qDebug() << q.lastQuery();
        funcs::dataBaseError(this, q);
        return;
    }
    QStringList columns = funcs::getColumns(q);
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }

    QMap<int, int> pupil2row;
    while (q.next()) {
        QStringList nowValues;
        for (int i = 0; i < columns.size(); i++) {
            nowValues.append(q.value(i).toString());
        }
        auto data = funcs::compress(columns, nowValues);
        if (!editStudentIds.contains({data["full_name"], data["which_mark"].toInt()})) {
            editStudentIds.append({data["full_name"], data["which_mark"].toInt()});
        }
        if (data.contains("subject")) {
            if (!editSubjectIds.contains({data["subject_type"], data["subject"].toInt()})) {
                editSubjectIds.append({data["subject_type"], data["subject"].toInt()});
            }
        }
    }

    if (!ui->checkBox->isChecked() ||
        convertShown.find(ui->comboBox_shownSubjects2->currentText()).value() == kShowExist) {

        QSqlQuery tmp(conn);
        tmp.prepare("SELECT subject_type, subject_id2 FROM class_subject "
                    "INNER JOIN subject_types st on st.subject_id = class_subject.subject_id2 "
                    "WHERE class_id2=(SELECT class_id FROM classes WHERE class_number=? AND class_letter=?)");
        tmp.addBindValue(ui->comboBox_classNum2->currentText());
        tmp.addBindValue(ui->comboBox_classLetter2->currentText());
        if (!tmp.exec()) {
            funcs::dataBaseError(this, tmp);
            qDebug() << tmp.lastError();
            return;
        }
        while (tmp.next()) {
            if (!editSubjectIds.contains({tmp.value(0).toString(), tmp.value(1).toInt()})) {
                editSubjectIds.append({tmp.value(0).toString(), tmp.value(1).toInt()});
            }
        }
    }

    qSort(editSubjectIds);
    ui->tableWidget_editMarks->setColumnCount(editSubjectIds.size());
    for (int i = 0; i < editSubjectIds.size(); i++) {
        auto *newItem = new QTableWidgetItem(editSubjectIds[i].first);
        ui->tableWidget_editMarks->setHorizontalHeaderItem(i, newItem);
    }

    qSort(editStudentIds);
    ui->tableWidget_editMarks->setRowCount(editStudentIds.size());
    for (int i = 0; i < editStudentIds.size(); i++) {
        pupil2row[editStudentIds[i].second] = i;
        auto *newItem = new QTableWidgetItem(editStudentIds[i].first);
        ui->tableWidget_editMarks->setVerticalHeaderItem(i, newItem);
    }

    q.first();
    while (q.isValid()) {
        QStringList nowValues;
        for (int i = 0; i < columns.size(); i++) {
            nowValues.append(q.value(i).toString());
        }
        auto data = funcs::compress(columns, nowValues);
        int row = editStudentIds.indexOf({data["full_name"], data["which_mark"].toInt()});
        int col = editSubjectIds.indexOf({data["subject_type"], data["subject"].toInt()});
        auto newItem = new QTableWidgetItem(data["mark_value"]);
        ui->tableWidget_editMarks->setItem(row, col, newItem);
        q.next();
    }
    ui->tableWidget_editMarks->resizeColumnsToContents();
    ui->tableWidget_editMarks->blockSignals(false);
}

void editViewMarks::editMark(const QTableWidgetItem *item) {
    conn.commit();
    int row = item->row(), col = item->column();
    QString newVal = item->text();

    if (newVal.isEmpty()) {
        QSqlQuery q(conn);
        q.prepare("SELECT mark_id FROM marks "
                  "WHERE which_mark=? AND mark_year=? AND type=(SELECT type_mark_id FROM mark_types WHERE type_mark=?) "
                  "AND subject=? AND actual_class=(SELECT class_id FROM classes WHERE class_number=? AND class_letter=?)");
        q.addBindValue(editStudentIds[row].second);
        q.addBindValue(ui->comboBox_year2->currentText());
        q.addBindValue(ui->comboBox_type2->currentText());
        q.addBindValue(editSubjectIds[col].second);
        q.addBindValue(ui->comboBox_classNum2->currentText());
        q.addBindValue(ui->comboBox_classLetter2->currentText());
        if (!q.exec()) {
            qDebug() << 0 << q.lastError();
            return;
        }
        q.next();
        if (!q.isValid()) {
            QMessageBox::warning(this, "Ошибка", "Не получается найти оценку", QMessageBox::Ok);
            return;
        }

        int markId = q.value(0).toInt();
        q.clear();
        q.prepare("DELETE FROM marks WHERE mark_id=?");
        q.addBindValue(markId);
        q.exec();
        return;
    }

    QSqlQuery q(conn);
    q.prepare("SELECT mark_value_id FROM mark_values WHERE mark_value=?");
    q.addBindValue(newVal);
    if (!q.exec()) {
        qDebug() << 1 << q.lastError();
        return;
    }
    q.next();
    if (!q.isValid()) {
        qDebug() << 2 << q.lastError();
        return;
    }
    int val = q.value(0).toInt();
    q.clear();

    q.prepare("SELECT mark_id FROM marks "
              "WHERE which_mark=? AND mark_year=? AND type=(SELECT type_mark_id FROM mark_types WHERE type_mark=?) "
              "AND subject=? AND actual_class=(SELECT class_id FROM classes WHERE class_number=? AND class_letter=?)");
    q.addBindValue(editStudentIds[row].second);
    q.addBindValue(ui->comboBox_year2->currentText());
    q.addBindValue(ui->comboBox_type2->currentText());
    q.addBindValue(editSubjectIds[col].second);
    q.addBindValue(ui->comboBox_classNum2->currentText());
    q.addBindValue(ui->comboBox_classLetter2->currentText());
    if (!q.exec()) {
        qDebug() << 3 << q.lastError();
        return;
    }
    q.next();
    if (q.isValid()) {
        int markId = q.value(0).toInt();
        q.clear();
        q.prepare("UPDATE marks SET value=? WHERE mark_id=?");
        q.addBindValue(val);
        q.addBindValue(markId);
        if (!q.exec()) {
            qDebug() << 5 << q.lastError();
        }
        return;
    }
    q.prepare("INSERT INTO marks(value,which_mark,mark_year,type,subject,actual_class) VALUES (?,?,?,"
              "(SELECT type_mark_id FROM mark_types WHERE type_mark=?),"
              "?,"
              "(SELECT class_id FROM classes WHERE class_number=? AND class_letter=?))");
    q.addBindValue(newVal);
    q.addBindValue(editStudentIds[row].second);
    q.addBindValue(ui->comboBox_year2->currentText());
    q.addBindValue(ui->comboBox_type2->currentText());
    q.addBindValue(editSubjectIds[col].second);
    q.addBindValue(ui->comboBox_classNum2->currentText());
    q.addBindValue(ui->comboBox_classLetter2->currentText());
    if (!q.exec()) {
        qDebug() << 6 << q.lastError();
        return;
    }
}

void editViewMarks::printSubjects() {
    ui->tableWidget_editSubjects->setRowCount(0);
    ui->tableWidget_editSubjects->setColumnCount(0);
    classes.clear(), subjects.clear();

    QSqlQuery q(conn);
    q.prepare("SELECT subject_id, subject_type FROM subject_types");
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    while (q.next()) {
        subjects.push_back({q.value(0).toInt(), q.value(1).toString()});
        int row = ui->tableWidget_editSubjects->rowCount();
        ui->tableWidget_editSubjects->setRowCount(row + 1);
        ui->tableWidget_editSubjects->setVerticalHeaderItem(row, new QTableWidgetItem(subjects.back().second));
    }

    if (ui->comboBox_classLetter3->currentText() != allClasses) {
        q.prepare("SELECT class_id, class_number, class_letter FROM classes "
                  "WHERE class_number=? AND class_letter=?");
        q.addBindValue(ui->comboBox_classNum3->currentText());
        q.addBindValue(ui->comboBox_classLetter3->currentText());
    } else {
        q.prepare("SELECT class_id, class_number, class_letter FROM classes WHERE class_number=?");
        q.addBindValue(ui->comboBox_classNum3->currentText());
    }
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    while (q.next()) {
        classes.push_back({q.value(0).toInt(), q.value(1).toString() + q.value(2).toString()});
        int col = ui->tableWidget_editSubjects->columnCount();
        ui->tableWidget_editSubjects->setColumnCount(col + 1);
        ui->tableWidget_editSubjects->setHorizontalHeaderItem(col, new QTableWidgetItem(classes.back().second));
    }

    for (int i = 0; i < ui->tableWidget_editSubjects->rowCount(); i++) {
        for (int j = 0; j < ui->tableWidget_editSubjects->columnCount(); j++) {
            auto item = new QTableWidgetItem("");
            item->setBackground(Qt::red);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            ui->tableWidget_editSubjects->setItem(i, j, item);
        }
    }

    if (ui->comboBox_classLetter3->currentText() != allClasses) {
        q.prepare("SELECT subject_id2, class_id2 FROM class_subject "
                  "WHERE class_id2 IN (SELECT class_id FROM classes WHERE class_number=? AND class_letter=?)");
        q.addBindValue(ui->comboBox_classNum3->currentText());
        q.addBindValue(ui->comboBox_classLetter3->currentText());
    } else {
        q.prepare("SELECT subject_id2, class_id2 FROM class_subject "
                  "WHERE class_id2 IN (SELECT class_id FROM classes WHERE class_number=?)");
        q.addBindValue(ui->comboBox_classNum3->currentText());
    }
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    while (q.next()) {
        int subj = q.value(0).toInt(), cl = q.value(1).toInt();
        int x = -1, y = -1;
        for (int i = 0; i < subjects.size(); i++) {
            for (int j = 0; j < classes.size(); j++) {
                if (subjects[i].first == subj && classes[j].first == cl) {
                    if (x == -1) {
                        x = i, y = j;
                    } else {
                        assert(false);
                    }
                }
            }
        }
        assert(x != -1);
        ui->tableWidget_editSubjects->item(x, y)->setBackground(Qt::green);
    }
}

void editViewMarks::subjectsEditMenuRequested(QPoint pos) {
    auto *menu = new QMenu(this);
    if (ui->tableWidget_editSubjects->selectedItems().size() != 1) {
        return;
    }
    auto *toggleSubject = new QAction("Добавить/убрать предмет", this);
    connect(toggleSubject, &QAction::triggered, this, &editViewMarks::toggleSubject);
    menu->addAction(toggleSubject);
    menu->popup(ui->tableWidget_editSubjects->viewport()->mapToGlobal(pos));
}

void editViewMarks::toggleSubject() {
    int row = ui->tableWidget_editSubjects->selectionModel()->currentIndex().row();
    int col = ui->tableWidget_editSubjects->selectionModel()->currentIndex().column();
    QSqlQuery q(conn);
    q.prepare("SELECT COUNT(*) FROM class_subject WHERE class_id2=? AND subject_id2=?");
    q.addBindValue(classes[col].first);
    q.addBindValue(subjects[row].first);
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    q.next();
    int cnt = q.value(0).toInt();
    if (cnt == 0) {
        q.prepare("INSERT INTO class_subject VALUES (?, ?)");
    } else {
        q.prepare("DELETE FROM class_subject WHERE class_id2=? AND subject_id2=?");
    }
    q.addBindValue(classes[col].first);
    q.addBindValue(subjects[row].first);
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    ui->tableWidget_editSubjects->item(row, col)->setBackground(cnt == 0 ? Qt::green : Qt::red);
}
