#include "ui_mainwindow.h"
#include "MainWindow.hpp"
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>

#ifdef VALIDATE_LICENSE
#include <sodium.h>
#endif
#ifdef _WIN32
#include <QtWinExtras/QWinTaskbarButton>
#include <QtWinExtras/QWinTaskbarProgress>
#endif

static char aboutText[] =
	"Tropical 2018 Collection\n"
	"For questions:\n"
	"support@clipartcrew.com\n\n"
	"Copyright 2018 Clipart Crew.\n"
	"Content rights belong to their respective owners.";


static char pubKey[] =
	"uFsUig1mNYoTGFnaClEW/2svEZeiBIwdWS9KTiIb+rz0I7gLpJj/o57Yki/jQHHpjI3Hs0o2Riyg3qOBubQR3rhbFIoNZjWKExhZ2gpRFv9rLxGXogSMHVkvSk4iG/q8";
// static char secKey[] =
// "9CO4C6SY/6Oe2JIv40Bx6YyNx7NKNkYsoN6jgbm0Ed64WxSKDWY1ihMYWdoKURb/ay8Rl6IEjB1ZL0pOIhv6vA==";
// static char licenseExample[] =
// "xEhRziT2LDKspOpdEm09vctAFj+ULC85fVMgzAyVYPxPKly6K1XzS49MkcUFvW7v/dfTgZkv2MLe4L68VpPbCHRlc3Q=";
MainWindow::MainWindow(QString argv, QWidget *parent)
	: QMainWindow(parent), ui(new Ui::MainWindow), args(argv)
#ifdef _WIN32
	  ,
	  progress(nullptr)
#endif
{

	qRegisterMetaType<QVector<int>>("QVector<int>");
	qRegisterMetaType<QList<QPersistentModelIndex>>(
		"QList<QPersistentModelIndex>");
	qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>(
		"QAbstractItemModel::LayoutChangeHint");
	qApp->installEventFilter(this);

	// ui->mainToolBar->hide();
}

MainWindow::~MainWindow() {
	// qDebug()<<"exiting from main";

	ui->imagesView->prepareExit();
	ui->fileTree->prepareExit();
	saveSettings();
	delete ui;

	// qDebug()<<"ui deleted";
}

void MainWindow::saveSettings() {
	QSettings settings;
	startDir = ui->fileTree->getCurrentDir();
	// qDebug()<<"Save: "<<startDir;
	settings.setValue("StartDir", startDir);
	settings.setValue("sortByPath", ui->actionSort_by_full_path->isChecked());
	settings.setValue("sortByName", ui->actionSort_by_file_name->isChecked());
	settings.setValue("scrollWhenLoading",
					  ui->actionScroll_when_loading->isChecked());
	settings.setValue("showPreview", ui->actionShow_Preview->isChecked());
	settings.setValue("HiQPreview",
					  ui->actionHigh_Quality_Preview->isChecked());
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("TreeWidth", ui->fileTree->width());
	settings.setValue("splitterSizes", ui->splitter->saveState());
	settings.endGroup();
}

void MainWindow::setFileInfo(int total, int visible) {
	if (visible < total) {
		info = QString::number(visible);
		info += " files of ";
		info += QString::number(total);
		info += " visible";
	} else {
		info = QString::number(total) + " visible files";
	}

	ui->infoBox->setText(info);
}

void MainWindow::setScanDirMsg(QString msg) { ui->infoBox->setText(msg); }

void MainWindow::showAbout() {
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("About");
	msgBox.setText(QByteArray::fromBase64(aboutText));
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.setDefaultButton(QMessageBox::Ok);
	msgBox.exec();
}

void MainWindow::init() {
	/***
	 * Read folders
	 * */
	QSettings settings;
	startDir = settings.value("StartDir", QDir::rootPath()).toString();
	// qDebug()<<"Read: "<<startDir;

	rootDir = settings.value("RootDir", QDir::rootPath()).toString();

	if (args.length()) {
		if (0 == args.trimmed().compare("--filesystem"))
			rootDir = QDir::rootPath();
		else {
			QStringList argList = args.split("=");
			QDir dir(argList.last());
			if (1 < argList.length()
				&& 0 == argList.first().compare("--setrootfolder")
				&& dir.exists())
				rootDir = dir.absolutePath();
			// qDebug()<<QDir::rootPath();
			// qDebug()<<argList.last();
			// qDebug()<<"Root: "<<rootDir;
			settings.setValue("RootDir", rootDir);
		}
	}
	// qDebug()<<"Root: "<<rootDir;

	/***
	 * End read folders
	 * */

	ui->setupUi(this);

	/***
	 * Setup menu actions
	 * */
	sortingGroup = new QActionGroup(this);
	sortingGroup->addAction(ui->actionSort_by_full_path);
	sortingGroup->addAction(ui->actionSort_by_file_name);
	qDebug() << "setup showpreview in Main";
	connect(ui->actionSort_by_full_path, SIGNAL(toggled(bool)), ui->imagesView,
			SIGNAL(sortByPath(bool)));
	connect(ui->actionShow_Preview, SIGNAL(toggled(bool)), ui->imagesView,
			SIGNAL(showPreview(bool)));
	connect(ui->actionHigh_Quality_Preview, SIGNAL(toggled(bool)),
			ui->imagesView, SIGNAL(enableHiQPreview(bool)));
	connect(ui->actionScroll_when_loading, SIGNAL(toggled(bool)),
			ui->imagesView, SLOT(setScrolling(bool)));
	ui->actionSort_by_full_path->setChecked(
		settings.value("sortByPath", true).toBool());
	ui->actionSort_by_file_name->setChecked(
		settings.value("sortByName", true).toBool());
	ui->actionScroll_when_loading->setChecked(
		settings.value("scrollWhenLoading", true).toBool());

	ui->actionHigh_Quality_Preview->setChecked(
		settings.value("HiQPreview", true).toBool());
	ui->actionHigh_Quality_Preview->setEnabled(false);
	ui->actionShow_Preview->setChecked(false);
	connect(ui->actionShow_Preview, &QAction::toggled, [&](bool checked) {
		ui->actionHigh_Quality_Preview->setEnabled(checked);
	});

	ui->actionShow_Preview->setChecked(
		settings.value("showPreview", true).toBool());

	connect(ui->actionZoom_In, &QAction::triggered,
			[&]() { ui->imagesView->setZoom(1); });

	connect(ui->actionZoom_Out, &QAction::triggered,
			[&]() { ui->imagesView->setZoom(-1); });

	connect(ui->actionReset_zoom, &QAction::triggered,
			[&]() { ui->imagesView->setZoom(0); });

	ui->menuBar->addAction("About", this, SLOT(showAbout()));
	connect(ui->actionExit, &QAction::triggered,
			[&]() { QApplication::quit(); });
	connect(ui->actionExport_Images, SIGNAL(triggered(bool)), ui->imagesView,
			SLOT(exportImages()));

	ui->actionExit->setShortcut(QKeySequence::Quit);
	ui->actionExit->setShortcut(QKeySequence(Qt::ALT + Qt::Key_X));

	ui->actionZoom_In->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus));
	ui->actionZoom_Out->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
	ui->actionZoom_In->setShortcut(QKeySequence(Qt::CTRL + Qt::WheelFocus));
	ui->actionZoom_Out->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus));
	ui->actionReset_zoom->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Slash));

	/***
	 * End setup menu actions
	 * */

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

	ui->imagesView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui->filterBox->setPlaceholderText("insert search string");

	if (!ui->splitter->restoreState(splitterSizes))
		ui->splitter->setSizes({200, 200});
	connect(ui->imagesView, SIGNAL(numFiles(int, int)), this,
			SLOT(setFileInfo(int, int)), Qt::QueuedConnection);

	connect(ui->imagesView, SIGNAL(genericMessage(QString)), this,
			SLOT(setScanDirMsg(QString)), Qt::QueuedConnection);
#ifdef _WIN32
	connect(ui->imagesView, SIGNAL(taskBarSetMaximum(int)), this,
			SLOT(setProgressMax(int)));

	connect(ui->imagesView, SIGNAL(taskBarSetValue(int)), this,
			SLOT(setProgressValue(int)));
#endif
	connect(ui->fileTree, SIGNAL(changeDir(QString)), ui->imagesView,
			SLOT(changeDir(QString)));

	connect(ui->fileTree, &SystemTreeView::splashText,
			[&](const QString &message, int alignment, const QColor &color) {
				emit splashText(message, alignment, color);
			});

	// emit splashText("aaaa", 1, Qt::blue);
	ui->fileTree->init(startDir);
	ui->infoBox->setEnabled(false);
	connect(ui->filterBox, SIGNAL(textChanged(QString)), ui->imagesView,
			SLOT(applyFilter(QString)));

	setWindowState(Qt::WindowMaximized);

	statusBar()->hide();
	ui->centralWidget->setContentsMargins(0, 0, 0, 0);
	ui->fileTree->setMouseTracking(true);

#ifdef VALIDATE_LICENSE
	licenseKey = settings.value("licenseKey", "1234").toByteArray();

	QTimer *timer = new QTimer();
	QObject::connect(timer, &QTimer::timeout, [&, timer]() {
		timer->moveToThread(timer->thread());
		initActivation();
		timer->deleteLater();
	});
	timer->start(10000000);
#endif
}

void MainWindow::initTree() { ui->fileTree->init(startDir); }

void MainWindow::initActivation() {
#ifdef VALIDATE_LICENSE

#ifdef QT_DEBUG
	return;
#endif
	QByteArray publ(pubKey);
	//	QByteArray secret(secKey);

	unsigned char *pk = reinterpret_cast<unsigned char *>(
		(QByteArray::fromBase64(publ)).data());
	unsigned char *enc = reinterpret_cast<unsigned char *>(
		(QByteArray::fromBase64(licenseKey)).data());
	// unsigned char *sk = reinterpret_cast<unsigned
	// char*>((QByteArray::fromBase64(secret)).data());


	unsigned char decodedLicense[256] = {0};
	unsigned long long decodedLicenseLength;
	if (crypto_sign_open(decodedLicense, &decodedLicenseLength, enc,
						 (QByteArray::fromBase64(licenseKey)).length(), pk)
		== 0) {
		QTextStream cout(stdout);
		cout << "Key validated successfully\n";
		cout.flush();
		return;
	}


	qDebug() << "Unable to decode message 1\n";

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

	connect(buttonBox, &QDialogButtonBox::accepted, [&]() {
		// qDebug()<<m_lineEdit->toPlainText();
		licenseKey = m_lineEdit->toPlainText().toLatin1();
		unsigned char *enc = reinterpret_cast<unsigned char *>(
			(QByteArray::fromBase64(licenseKey)).data());
		unsigned char *pk = reinterpret_cast<unsigned char *>(
			(QByteArray::fromBase64(publ)).data());
		if (crypto_sign_open(decodedLicense, &decodedLicenseLength, enc,
							 (QByteArray::fromBase64(licenseKey)).length(), pk)
			!= 0) {
			qlabel->setText("Entered license is incorrect");
			return;
		}
		QTextStream cout(stdout);
		cout << "Key validated successfully\n";
		cout.flush();
		QSettings settings;
		settings.setValue("licenseKey", licenseKey);
		activationDlg.hide();
	});
	connect(buttonBox, &QDialogButtonBox::rejected, []() {
		QTextStream err(stderr);
		err << "Unable to validate key\n";
		err.flush();
		QApplication::exit();
	});

	activationDlg.setModal(true);
	activationDlg.exec();
#endif
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() == QEvent::Wheel) {
		QWheelEvent *wheel = static_cast<QWheelEvent *>(event);
		// qDebug()<<"Wheel: "<<wheel;
		// qDebug()<<wheel->modifiers();
		// qDebug()<<wheel->delta();
		if (wheel->modifiers() == Qt::ControlModifier) {
			ui->imagesView->setZoom(wheel->delta());
			return true;
		}
	}
	return QObject::eventFilter(obj, event);
}

QSize MainWindow::getTreeWidgetSize() const { return ui->fileTree->size(); }

QPoint MainWindow::getTreeWidgetPos() const {
	// qDebug()<<ui->fileTree->frameGeometry();
	// qDebug()<<mapToGlobal( ui->fileTree->pos());
	// qDebug()<<ui->fileTree->mapTo(this, ui->fileTree->pos());
	// return ui->fileTree->mapFromGlobal(ui->fileTree->pos());
	return ui->fileTree->mapTo(this, ui->fileTree->pos());
}
#ifdef _WIN32
void MainWindow::initProgressTaskbar() {
	QWinTaskbarButton *button = new QWinTaskbarButton(this);
	button->moveToThread(this->thread());
	button->setWindow(windowHandle());
	// button->setOverlayIcon(QIcon(":/loading.png"));

	progress = button->progress();
	progress->setVisible(true);
	progress->setMinimum(0);
	progress->setMaximum(cachedProgress);
}

void MainWindow::setProgressMax(int max) {
	if (!progress)
		cachedProgress = max;
	else
		progress->setMaximum(max);
}
void MainWindow::setProgressValue(int value) {
	if (!progress)
		return;
	if (progress->maximum() == value)
		progress->setValue(0);
	else
		progress->setValue(value);
}
#endif
