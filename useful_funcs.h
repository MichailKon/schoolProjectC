//
// Created by michael on 16.06.2021.
//

#ifndef SCHOOLPROJECTC_USEFUL_FUNCS_H
#define SCHOOLPROJECTC_USEFUL_FUNCS_H

#include <QMap>
#include <QVector>
#include <QString>
#include <QPushButton>
#include <QObject>
#include <QDebug>

namespace funcs {
//    template<class T, class C>
    QMap<QString, QString> compress(const QVector<QString> &, const QVector<QString> &);

    QPair<QString, QStringList> getPupilClassQuery(const QString &column, const QString &num,
                                                   const QString &let, const bool &in=true);

    QStringList parseNum(const QString &s);

    QStringList parseLet(const QString &s);
}

namespace classes {
}


#endif //SCHOOLPROJECTC_USEFUL_FUNCS_H
