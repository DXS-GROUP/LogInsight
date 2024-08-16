#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowFlag(Qt::FramelessWindowHint);
    createConnects();

    sizeGrip = new QSizeGrip(this);
    sizeGrip->setFixedSize(20, 20);
    sizeGrip->setStyleSheet("background-color: rgba(255, 255, 255, 0); border-radius: 2px;");

    globalSettings = new QSettings("DXS_GROUP", "CodeKeeper");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createConnects() {
    QObject::connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(closeApp()));
    QObject::connect(ui->actionMinimize, SIGNAL(triggered()), this, SLOT(minimizeApp()));
    QObject::connect(ui->actionMaximize, SIGNAL(triggered()), this, SLOT(maximizeApp()));
}

void MainWindow::closeApp() {
    this->close();
}

void MainWindow::minimizeApp() {
    this->showMinimized();
}

void MainWindow::maximizeApp() {
    if (!this->isMaximized()) {
        this->showMaximized();
    }
    else {
        this->showNormal();
    }
}
