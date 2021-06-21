//
// Created by michael on 16.06.2021.
//

#include "useful_funcs.h"

QMap<QString, QString> funcs::compress(const QStringList &a, const QStringList &b) {
    assert(a.size() == b.size());
    QMap<QString, QString> res;
    for (int i = 0; i < a.size(); i++) {

        res[a[i]] = b[i];
    }

    return res;
}

QPair<QString, QStringList> funcs::getPupilClassQuery(const QString &column, const QString &num,
                                                      const QString &let, const bool &in) {
    QString q;
    QStringList data;
    QString add = (in ? "" : "NOT");

    QStringList nums = parseNum(num);
    QStringList lets = parseLet(let);

    q += column + " " + add + " IN (SELECT class_id FROM classes WHERE (";
    for (int i = 0; i < nums.size(); i++) {
        q += "class_number = ?";
        if (i != nums.size() - 1) q += " OR ";
    }
    q += ") AND (";
    for (int i = 0; i < lets.size(); i++) {
        q += "class_letter = ?";
        if (i != lets.size() - 1) q += " OR ";
    }
    q += ")";
    q += ")";
    data << nums << lets;

    return QPair<QString, QStringList>(q, data);
}

QStringList funcs::parseNum(const QString &s) {
    QStringList tokens = s.split(',');
    QStringList ans;
    for (auto &i : tokens) {
        QStringList nowToken = i.split('-');
        if (nowToken.size() == 1) {
            ans << nowToken[0];
        } else if (nowToken.size() == 2) {
            int l = nowToken[0].toInt(), r = nowToken[1].toInt();
            for (int j = l; j <= r; j++) {
                ans << QString::number(j);
            }
        }
    }
    return ans;
}

QStringList funcs::parseLet(const QString &s) {
    QStringList tokens = s.split(',');
    QStringList ans;
    for (auto &i : tokens) {
        QStringList nowToken = i.split('-');
        if (nowToken.size() == 1) {
            ans << nowToken[0];
        } else if (nowToken.size() == 2) {
            ushort l = nowToken[0][0].unicode(), r = nowToken[1][0].unicode();
            for (ushort j = l; j <= r; j++) {
                ans << QString(QChar(j));
            }
        }
    }
    return ans;
}

QStringList funcs::getColumns(const QSqlQuery &q) {
    QStringList columns;
    QSqlRecord localRecord = q.record();
    for (int var = 0; var < localRecord.count(); var++) {
        columns.append(localRecord.fieldName(var));
    }
    return columns;
}
