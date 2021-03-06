//
// Created by michael on 20.06.2021.
//

#ifndef SCHOOLPROJECTC_EDIT_VIEW_MARKS_H
#define SCHOOLPROJECTC_EDIT_VIEW_MARKS_H

#include <QWidget>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QtAlgorithms>
#include <QMenu>
#include <QAction>
#include <QTableWidgetItem>
#include "useful_funcs.h"

enum KickedType {
    kAny,
    kStudying,
    kKicked,
};

enum ShownSubjectsType {
    kShowMarks,
    kShowExist
};

QT_BEGIN_NAMESPACE
namespace Ui { class editViewMarks; }
QT_END_NAMESPACE

class propType : public QPair<int, int> {
 public:
    propType(int a, int b) {
        this->first = a;
        this->second = b;
    }

    [[nodiscard]] int cntMarks() const {
        return this->first;
    }

    [[nodiscard]] int markType() const {
        return this->second;
    }
};

class editViewMarks : public QWidget {
 Q_OBJECT

 public:
    explicit editViewMarks(QWidget *parent = nullptr);

    ~editViewMarks() override;

 public slots:

    void cancel();

    void setDatabase(const QSqlDatabase &other);

 private slots:

    void printViewMarks();

    void printEditMarks();

    void editMark(const QTableWidgetItem *item);

    void printSubjects();

    void subjectsEditMenuRequested(QPoint pos);

    void toggleSubject();

 private:
    Ui::editViewMarks *ui;
    QWidget *parent;
    QSqlDatabase conn;

    const QMap<QString, propType> propTypes = {
        {"Одна 2", {1, 2}},
        {"Одна 3", {1, 3}},
        {"Одна 4", {1, 4}},
        {"Одна 5", {1, 5}},
        {"Две 2", {2, 2}},
        {"Две 3", {2, 3}},
        {"Две 4", {2, 4}},
        {"Две 5", {2, 5}},
    };

    const QString allProps = "Все люди";
    const QString allClasses = "Вся параллель";

    const QMap<QString, KickedType> convertKicked = {
        {"Оба типа", kAny},
        {"Обучаются", kStudying},
        {"Отчислены", kKicked}
    };

    const QMap<QString, ShownSubjectsType> convertShown = {
        {"Только предметы с оценками", kShowMarks},
        {"Только предметы из 3 вкладки", kShowExist}
    };

    QVector<QPair<QString, int>> editStudentIds, editSubjectIds;
    QVector<QPair<int, QString>> classes, subjects;

    void prepare();

    void connectSlots();

    QSqlQuery getStudentQuery(const QString &num, const QString &let, const QString &year,
                              const QString &markType, ShownSubjectsType shownSubject, KickedType kicked,
                              const QCheckBox *haveMarks = nullptr);
};

#endif //SCHOOLPROJECTC_EDIT_VIEW_MARKS_H
