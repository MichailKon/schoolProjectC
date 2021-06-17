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

namespace funcs {
//    template<class T, class C>
    QMap<QString, QString> compress(const QStringList &, const QStringList &);

    QPair<QString, QStringList> getPupilClassQuery(const QString &column, const QString &num,
                                                   const QString &let, const bool &in = true);

    QStringList parseNum(const QString &s);

    QStringList parseLet(const QString &s);

    std::tuple<QString, int, QString, QString, QString, QString, int> getStudentInfo(
            QDialog *win,
            QSqlQuery query,
            QLineEdit *pupilName,
            QLineEdit *pupilClass,
            QDateEdit *pupilBirth,
            QDateEdit *pupilStart,
            QLineEdit *pupilAddress,
            QLineEdit *pupilParent,
            QComboBox *pupilGender
    );

    QPair<QString, QString> getClassNumberLetter(const QString &t);
}

namespace classes {
}


#endif //SCHOOLPROJECTC_USEFUL_FUNCS_H
