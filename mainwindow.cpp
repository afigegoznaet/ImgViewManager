#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QString argv, QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow), args(argv){

	qRegisterMetaType<QVector<int> >("QVector<int>");
	qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
	qRegisterMetaType<QAbstractItemModel::LayoutChangeHint >("QAbstractItemModel::LayoutChangeHint");


}

MainWindow::~MainWindow(){
	qDebug()<<"exiting from main";
	ui->imagesView->prepareExit();
	saveSettings();
	delete ui;
	qDebug()<<"ui deleted";
}

void MainWindow::saveSettings(){
	QSettings settings;
	startDir = ui->fileTree->getCurrentDir();
	qDebug()<<"Save: "<<startDir;
	settings.setValue("StartDir", startDir);
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("TreeWidth", ui->fileTree->width());
	settings.setValue("splitterSizes", ui->splitter->saveState());
	settings.endGroup();
}

void MainWindow::setFileInfo(int total, int visible){
	if(visible < total){
		info = QString::number(visible);
		info += " files of ";
		info += QString::number(total);
		info += " visible";
	}else{
		info = QString::number(total) + " visible files";
	}

	ui->infoBox->setText(info);
}

void MainWindow::setScanDirMsg(QString msg){

	ui->infoBox->setText(msg);
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

void MainWindow::init(){
	/***
	 * Read folders
	 * */
	QSettings settings;
	startDir = settings.value("StartDir",QDir::rootPath()).toString();
	qDebug()<<"Read: "<<startDir;

	rootDir = settings.value("RootDir",QDir::rootPath()).toString();

	if(args.length()){
		if(0 == args.trimmed().compare("--filesystem"))
			rootDir = QDir::rootPath();
		else{
			QStringList argList = args.split("=");
			QDir dir(argList.last());
			if( 1 < argList.length() && 0 == argList.first().compare("--setrootfolder") && dir.exists())
				rootDir = dir.absolutePath();
			qDebug()<<QDir::rootPath();
			qDebug()<<argList.last();
			qDebug()<<"Root: "<<rootDir;
			settings.setValue("RootDir", rootDir);
		}
	}
	qDebug()<<"Root: "<<rootDir;
	/***
	 * End read folders
	 * */
	ui->setupUi(this);
	/***
	 * Restore UI
	 * */
	settings.beginGroup("MainWindow");
	resize(settings.value("size", QSize(400, 400)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	splitterSizes = settings.value("splitterSizes").toByteArray();
	settings.endGroup();

	/***
	 * End restore UI
	 * */
	if(!ui->splitter->restoreState(splitterSizes))
		ui->splitter->setSizes({200,200});
	connect(ui->imagesView, SIGNAL(numFiles(int,int)),
			this, SLOT(setFileInfo(int,int)), Qt::QueuedConnection);

	connect(ui->imagesView, SIGNAL(genericMessage(QString)),
			this, SLOT(setScanDirMsg(QString)), Qt::QueuedConnection);

	connect(ui->fileTree, SIGNAL(changeDir(QString)),
			ui->imagesView, SLOT(changeDir(QString)));

	connect(ui->fileTree, &SystemTreeView::splashText,
			[&](const QString& message, int alignment, const QColor &color){
		emit splashText(message, alignment, color);
	});

	emit splashText("aaaa", 1, Qt::blue);
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

	ui->actionExit->setShortcut(QKeySequence::Quit);
	ui->actionExit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_X));


}

void MainWindow::initTree(){
	ui->fileTree->init(startDir);
}
