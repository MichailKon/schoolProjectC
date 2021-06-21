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
    }

    QMapIterator it(convertKicked);
    while (it.hasNext()) {
        it.next();
        ui->comboBox_isKicked->addItem(it.key());
    }
}

void editViewMarks::connectSlots() {
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &editViewMarks::cancel);
    connect(ui->pushButton_printView, &QPushButton::clicked, this, &editViewMarks::printViewMarks);
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

    QSqlQuery q = getStudentQuery(ui->lineEdit_classNum, ui->lineEdit_classLetter, ui->comboBox_year, ui->comboBox_type,
                                  ui->comboBox_property,
                                  convertKicked.find(ui->comboBox_isKicked->currentText()).value());
    if (!q.exec()) {
        funcs::dataBaseError(this, q);
        return;
    }
    QStringList columns = funcs::getColumns(q);
    if (ui->comboBox_property->currentText() != allProps) {
        QStringList pupils;
        auto[cnt, mark] = *propTypes.find(ui->comboBox_property->currentText());
        ui->tableWidget_viewMarks->setColumnCount(cnt);
        for (int i = 1; i <= cnt; i++) {
            auto *item = new QTableWidgetItem("Предмет " + QString::number(i));
            ui->tableWidget_viewMarks->setHorizontalHeaderItem(i - 1, item);
        }

        QMap<QString, int> mustCol;
        while (q.next()) {
            int nowRow = ui->tableWidget_viewMarks->rowCount();
            ui->tableWidget_viewMarks->setRowCount(nowRow + 1);

            QStringList nowValues;
            for (int i = 0; i < columns.size(); i++) {
                nowValues.append(q.value(i).toString());
            }
            auto data = funcs::compress(columns, nowValues);
            // there was a filter in last code, but i think, that it is useless (also i'm too lazy to do it)
            pupils.append(data["full_name"]);

            auto *newItem = new QTableWidgetItem(data["subject_type"]);
            int row = pupils.indexOf(data["full_name"]);
            int col = mustCol[data["full_name"]]++;
            ui->tableWidget_viewMarks->setItem(row, col, newItem);
        }
        ui->tableWidget_viewMarks->setVerticalHeaderLabels(pupils);
    } else {
        QStringList subjects;
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
                if (!subjects.contains(data["subject_type"])) {
                    subjects.append(data["subject_type"]);
                }
            }
            ui->tableWidget_viewMarks->setRowCount(pupilsSet.size());
            for (auto &i : pupilsSet) {
                pupils.append(i);
            }
            qSort(pupils);
            for (int i = 0; i < pupils.size(); i++) {
                auto *newItem = new QTableWidgetItem(pupils[i].first);
                ui->tableWidget_viewMarks->setVerticalHeaderItem(i, newItem);
                pupil2row[pupils[i].second] = i;
            }
        }
        ui->tableWidget_viewMarks->setColumnCount(subjects.size());
        ui->tableWidget_viewMarks->setHorizontalHeaderLabels(subjects);

        q.exec();
        while (q.next()) {
            QStringList nowValues;
            for (int i = 0; i < columns.size(); i++) {
                nowValues.append(q.value(i).toString());
            }
            auto data = funcs::compress(columns, nowValues);
            int row = pupil2row[data["which_mark"].toInt()];
            int col = subjects.indexOf(data["subject_type"]);
            auto *newItem = new QTableWidgetItem(data["mark_value"]);
            ui->tableWidget_viewMarks->setItem(row, col, newItem);
        }
    }
    ui->tableWidget_viewMarks->resizeColumnsToContents();
}

QSqlQuery editViewMarks::getStudentQuery(const QLineEdit *num, const QLineEdit *let, const QComboBox *year,
                                         const QComboBox *markType, const QComboBox *prop, KickedType kicked) {
    QSqlQuery res(conn);
    QString query;
    QStringList data;

    query += "SELECT full_name, subject_type, mark_value, mark_id, which_mark, "
             "subject, mark_year, type, actual_class "
             "FROM marks "
             "INNER JOIN pupils ON which_mark = student_id "
             "INNER JOIN mark_values ON value = mark_value_id "
             "INNER JOIN subject_types ON subject = subject_id "
             "WHERE EXISTS(SELECT * FROM class_subject WHERE subject_id2=subject AND "
             "class_id2=actual_class)";

    QString column = "actual_class";
    QPair<QString, QStringList> classInfo = funcs::getPupilClassQuery(column, num->text(), let->text());
    data.append(classInfo.second);
    query += " AND " + classInfo.first;

    if (year != nullptr) {
        query += " AND mark_year=?";
        data.append(year->currentText());
    }
    if (markType != nullptr) {
        query += " AND type=(SELECT type_mark_id FROM mark_types WHERE type_mark=?)";
        data.append(markType->currentText());
    }

    if (kicked == kKicked) {
        query += " AND kicked=1";
    } else if (kicked == kStudying) {
        query += " AND kicked=0";
    }

    if (prop != nullptr && prop->currentText() != allProps) {
        QString temp_query = "SELECT DISTINCT which_mark, "
                             "SUM (CASE value WHEN (SELECT mark_value_id FROM mark_values WHERE mark_value=5) THEN "
                             "1 ELSE 0 END) AS \"5\", "
                             "SUM (CASE value WHEN (SELECT mark_value_id FROM mark_values WHERE mark_value=4) THEN "
                             "1 ELSE 0 END) AS \"4\", "
                             "SUM (CASE value WHEN (SELECT mark_value_id FROM mark_values WHERE mark_value=3) THEN "
                             "1 ELSE 0 END) AS \"3\", "
                             "SUM (CASE value WHEN (SELECT mark_value_id FROM mark_values WHERE mark_value=2) THEN "
                             "1 ELSE 0 END) AS \"2\", "
                             "SUM (CASE value WHEN (SELECT mark_value_id FROM mark_values WHERE mark_value='Н/А') THEN "
                             "1 ELSE 0 END) AS \"Н/А\" "
                             "FROM marks "
                             "WHERE mark_year=? AND "
                             "EXISTS(SELECT * FROM class_subject WHERE subject_id2=subject AND "
                             "class_id2={table}) "
                             "GROUP BY which_mark";
        temp_query = temp_query.arg(column);
        query += " AND which_mark IN (SELECT which_mark FROM (" + temp_query + ") WHERE \"Н/А\"=0 AND "
                                                                               "\"?\"=?";
        data.append(year->currentText());
        data.append(QString::number(propTypes.find(prop->currentText())->markType()));
        data.append(QString::number(propTypes.find(prop->currentText())->cntMarks()));
        query += ")";
    }
    res.prepare(query);
    for (const auto &i : data) {
        res.addBindValue(i);
    }
    return res;
}
