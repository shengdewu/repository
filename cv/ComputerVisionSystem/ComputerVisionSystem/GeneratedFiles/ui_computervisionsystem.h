/********************************************************************************
** Form generated from reading UI file 'computervisionsystem.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COMPUTERVISIONSYSTEM_H
#define UI_COMPUTERVISIONSYSTEM_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ComputerVisionSystemClass
{
public:
    QWidget *centralWidget;
    QPushButton *OpenImg_PB;
    QTextBrowser *DebugInfo_TB;
    QTextBrowser *textBrowser_2;
    QPushButton *Train_PB;
    QPushButton *Discern_PB;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *ComputerVisionSystemClass)
    {
        if (ComputerVisionSystemClass->objectName().isEmpty())
            ComputerVisionSystemClass->setObjectName(QStringLiteral("ComputerVisionSystemClass"));
        ComputerVisionSystemClass->resize(600, 400);
        centralWidget = new QWidget(ComputerVisionSystemClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        OpenImg_PB = new QPushButton(centralWidget);
        OpenImg_PB->setObjectName(QStringLiteral("OpenImg_PB"));
        OpenImg_PB->setGeometry(QRect(20, 10, 75, 23));
        DebugInfo_TB = new QTextBrowser(centralWidget);
        DebugInfo_TB->setObjectName(QStringLiteral("DebugInfo_TB"));
        DebugInfo_TB->setGeometry(QRect(20, 40, 271, 311));
        textBrowser_2 = new QTextBrowser(centralWidget);
        textBrowser_2->setObjectName(QStringLiteral("textBrowser_2"));
        textBrowser_2->setGeometry(QRect(300, 40, 256, 301));
        Train_PB = new QPushButton(centralWidget);
        Train_PB->setObjectName(QStringLiteral("Train_PB"));
        Train_PB->setGeometry(QRect(120, 10, 75, 23));
        Discern_PB = new QPushButton(centralWidget);
        Discern_PB->setObjectName(QStringLiteral("Discern_PB"));
        Discern_PB->setGeometry(QRect(220, 10, 75, 23));
        ComputerVisionSystemClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(ComputerVisionSystemClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 23));
        ComputerVisionSystemClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(ComputerVisionSystemClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        ComputerVisionSystemClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(ComputerVisionSystemClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        ComputerVisionSystemClass->setStatusBar(statusBar);

        retranslateUi(ComputerVisionSystemClass);

        QMetaObject::connectSlotsByName(ComputerVisionSystemClass);
    } // setupUi

    void retranslateUi(QMainWindow *ComputerVisionSystemClass)
    {
        ComputerVisionSystemClass->setWindowTitle(QApplication::translate("ComputerVisionSystemClass", "ComputerVisionSystem", 0));
        OpenImg_PB->setText(QApplication::translate("ComputerVisionSystemClass", "\346\211\223\345\274\200\345\233\276\347\211\207", 0));
        Train_PB->setText(QApplication::translate("ComputerVisionSystemClass", "\350\256\255\347\273\203", 0));
        Discern_PB->setText(QApplication::translate("ComputerVisionSystemClass", "\350\257\206\345\210\253", 0));
    } // retranslateUi

};

namespace Ui {
    class ComputerVisionSystemClass: public Ui_ComputerVisionSystemClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COMPUTERVISIONSYSTEM_H
