#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    globalSettings = new QSettings("DXS_GROUP", "CodeKeeper");

}

MainWindow::~MainWindow()
{
    delete ui;
}
