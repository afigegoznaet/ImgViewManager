#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow){

	ui->setupUi(this);
	connect(ui->imagesView, SIGNAL(numFiles(int,int)),
			this, SLOT(setFileInfo(int,int)), Qt::QueuedConnection);
	readSettings();
	connect(ui->fileTree, SIGNAL(changeDir(QString)),
			ui->imagesView, SLOT(changeDir(QString)));
	ui->fileTree->init(startDir);
	ui->infoBox->setEnabled(false);
	connect(ui->filterBox, SIGNAL(textChanged(QString)),
			ui->imagesView, SLOT(applyFilter(QString)));

	ui->menuBar->addAction("About",this, SLOT(showAbout()));
	connect(ui->actionExit, &QAction::triggered, [&](){
		QApplication::quit();
	});
	connect(ui->actionExport_Images, SIGNAL(triggered(bool)),
			ui->imagesView, SLOT(exportImages()));
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
	qDebug()<<"Read: "<<startDir;
	if(!ui->splitter->restoreState(settings.value("splitterSizes").toByteArray()))
		ui->splitter->setSizes({200,200});
	settings.endGroup();

}

void MainWindow::saveSettings(){
	QSettings settings;
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("TreeWidth", ui->fileTree->width());
	startDir = ui->fileTree->getCurrentDir();
	qDebug()<<"Save: "<<startDir;
	settings.setValue("StartDir", startDir);
	settings.setValue("splitterSizes", ui->splitter->saveState());
	settings.endGroup();
}

void MainWindow::setFileInfo(int total, int visible){
	info = QString::number(visible);
	info += " files visible of ";
	info += QString::number(total);
	ui->infoBox->setText(info);
}

void MainWindow::showAbout(){
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("About");
	msgBox.setText("Incepted in 2018 in Chisinau, Moldova");
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}
