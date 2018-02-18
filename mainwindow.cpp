#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow){

	ui->setupUi(this);
	readSettings();
	connect(ui->fileTree, SIGNAL(changeDir(QString)),ui->imagesView, SLOT(changeDir(QString)));
	ui->fileTree->init(startDir);

	ui->menuAbout->exec();
}

MainWindow::~MainWindow(){
	qDebug()<<"exiting from main";
	ui->imagesView->prepareExit();
	saveSettings();
	delete ui;
	qDebug()<<"ui deleted";
}

void MainWindow::readSettings(){

	QSettings settings;

	settings.beginGroup("MainWindow");

	resize(settings.value("size", QSize(400, 400)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	startDir = settings.value("StartDir",QDir::rootPath()).toString();

	settings.endGroup();

}

void MainWindow::saveSettings(){
	QSettings settings;
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	qDebug()<< ui->fileTree->getCurrentDir();
	settings.setValue("StartDir", ui->fileTree->getCurrentDir());
	settings.endGroup();


}
