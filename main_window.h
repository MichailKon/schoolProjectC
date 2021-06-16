//
// Created by michael on 15.06.2021.
//

#ifndef SCHOOLPROJECTC_MAIN_WINDOW_H
#define SCHOOLPROJECTC_MAIN_WINDOW_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class mainWindow; }
QT_END_NAMESPACE

class mainWindow : public QWidget {
Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);

    ~mainWindow() override;

private:
    Ui::mainWindow *ui;

    void connectSlots();

    void openStats();

    void openReview();

    void openMarks();
};


#endif //SCHOOLPROJECTC_MAIN_WINDOW_H
