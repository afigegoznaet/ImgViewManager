#include "mainwindow.h"
#include "ui_mainwindow.h"




static char pubKey[] = "uFsUig1mNYoTGFnaClEW/2svEZeiBIwdWS9KTiIb+rz0I7gLpJj/o57Yki/jQHHpjI3Hs0o2Riyg3qOBubQR3rhbFIoNZjWKExhZ2gpRFv9rLxGXogSMHVkvSk4iG/q8";
static char secKey[] = "9CO4C6SY/6Oe2JIv40Bx6YyNx7NKNkYsoN6jgbm0Ed64WxSKDWY1ihMYWdoKURb/ay8Rl6IEjB1ZL0pOIhv6vA==";
static char licenseExample[] = "xEhRziT2LDKspOpdEm09vctAFj+ULC85fVMgzAyVYPxPKly6K1XzS49MkcUFvW7v/dfTgZkv2MLe4L68VpPbCHRlc3Q=";
MainWindow::MainWindow(QString argv, QWidget *parent) :
	QMainWindow(parent), ui(new Ui::MainWindow), args(argv){

	qRegisterMetaType<QVector<int> >("QVector<int>");
	qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
	qRegisterMetaType<QAbstractItemModel::LayoutChangeHint >("QAbstractItemModel::LayoutChangeHint");
}

MainWindow::~MainWindow(){
	qDebug()<<"exiting from main";
	if(isActivated){
		ui->imagesView->prepareExit();
		ui->fileTree->prepareExit();
		saveSettings();
		delete ui;
	}

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

#ifdef VALIDATE_LICENSE
	licenseKey = settings.value("licenseKey","1234").toByteArray();
	initActivation();
#endif
	/***
	 * End read folders
	 * */

	if(!isActivated){
		QApplication::quit();
		return;
	}

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

	ui->filterBox->setPlaceholderText("insert search string");

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

	//emit splashText("aaaa", 1, Qt::blue);
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

void MainWindow::initActivation(){
#ifdef VALIDATE_LICENSE


	QByteArray publ(pubKey);
	QByteArray secret(secKey);

	unsigned char *pk = reinterpret_cast<unsigned char*>((QByteArray::fromBase64(publ)).data());
	unsigned char *enc = reinterpret_cast<unsigned char*>((QByteArray::fromBase64(licenseKey)).data());
	//unsigned char *sk = reinterpret_cast<unsigned char*>((QByteArray::fromBase64(secret)).data());


	unsigned char decodedLicense[256] = {0};
	unsigned long long decodedLicenseLength;
	if (crypto_sign_open(decodedLicense, &decodedLicenseLength,
						 enc, (QByteArray::fromBase64(licenseKey)).length(), pk) == 0)
		return;


	qDebug()<<"Unable to decode message 1\n";

	/**
	 * @brief Create dialog
	 */
	QLabel *qlabel = new QLabel();
	auto m_lineEdit = new QPlainTextEdit();
	m_lineEdit->setPlaceholderText("Enter license here");

	QPushButton *createButton = new QPushButton(tr("Ok"));
	createButton->setDefault(true);

	QPushButton *cancelButton = new QPushButton(tr("Cancel"));

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(createButton, QDialogButtonBox::AcceptRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

	QVBoxLayout *lt = new QVBoxLayout;
	qlabel->adjustSize();
	lt->addWidget(qlabel);
	lt->addWidget(m_lineEdit);
	lt->addWidget(buttonBox);

	QDialog activationDlg;
	activationDlg.setLayout(lt);

	/**
	 * Dialog created
	 */

	connect(buttonBox, &QDialogButtonBox::accepted, [&](){
		qDebug()<<m_lineEdit->toPlainText();
		licenseKey = m_lineEdit->toPlainText().toLatin1();
		unsigned char *enc = reinterpret_cast<unsigned char*>((QByteArray::fromBase64(licenseKey)).data());
		unsigned char *pk = reinterpret_cast<unsigned char*>((QByteArray::fromBase64(publ)).data());
		if (crypto_sign_open(decodedLicense, &decodedLicenseLength,
							 enc, (QByteArray::fromBase64(licenseKey)).length(), pk) != 0){
			qlabel->setText("Entered license is incorrect");
			return;
		}
		QSettings settings;
		settings.setValue("licenseKey", licenseKey);
		isActivated = true;
		activationDlg.hide();
	});
	connect(buttonBox, &QDialogButtonBox::rejected, [&](){
		QApplication::quit();
	});

	qDebug()<<activationDlg.exec();
#endif
}
