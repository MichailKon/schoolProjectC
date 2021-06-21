//
// Created by michael on 16.06.2021.
//

#ifndef SCHOOLPROJECTC_USEFUL_FUNCS_H
#define SCHOOLPROJECTC_USEFUL_FUNCS_H

#include <QMap>
#include <QVector>
#include <QString>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
#include <QObject>
#include <QDebug>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QMessageBox>

namespace funcs {
    QMap<QString, QString> compress(const QStringList &, const QStringList &);

    QPair<QString, QStringList> getPupilClassQuery(const QString &column, const QString &num,
                                                   const QString &let, const bool &in = true);

    QStringList parseNum(const QString &s);

    QStringList parseLet(const QString &s);

    template<class T>
    void dataBaseError(T *win, const QSqlQuery &query) {
        qDebug() << query.lastError();
        QMessageBox::critical(win,
                              "Проблемы с базой данных",
                              "Проблема с базой данных: " + query.lastError().text(),
                              QMessageBox::Ok);
    }

    QStringList getColumns(const QSqlQuery &q);
}

namespace classes {
}


#endif //SCHOOLPROJECTC_USEFUL_FUNCS_H
