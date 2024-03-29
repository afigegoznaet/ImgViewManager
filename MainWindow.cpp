#include "ui_mainwindow.h"
#include "MainWindow.hpp"
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPlainTextEdit>
#include <utility>
#include <utility>

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
	: QMainWindow(parent), ui(new Ui::MainWindow), args(std::move(argv))
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
	settings.setValue("showPreview", ui->actionShow_Preview->isChecked());
	settings.setValue("HiQPreview",
					  ui->actionHigh_Quality_Preview->isChecked());
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("TreeWidth", ui->fileTree->width());
	settings.setValue("splitterSizes", ui->splitter->saveState());
	settings.setValue("preloadImages", ui->actionPreload_Images->isChecked());

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

void MainWindow::setScanDirMsg(const QString &msg) {
	ui->infoBox->setText(msg);
}

void MainWindow::showAbout() {
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Information);
	msgBox.setWindowTitle("About");
	msgBox.setText(aboutText);
	// msgBox.setText(QByteArray::fromBase64(aboutText));
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

	rootDir = settings.value("RootDir", QDir::root().path()).toString();

	if (args.length()) {
		if (0 == args.trimmed().compare("--filesystem"))
			rootDir = QDir::rootPath();
		else {
			QStringList argList = args.split("=");
			QDir		dir(argList.last());

			if (dir.exists()) {
				startDir = argList.last();
				while (dir.cdUp())
					;
				rootDir = dir.absolutePath();
			}
			settings.setValue("RootDir", rootDir);
		}
	}
	qDebug() << "Root: " << rootDir;

	/***
	 * End read folders
	 * */

	ui->setupUi(this);
	ui->progressBar->hide();

	/***
	 * Setup menu actions
	 * */

	auto preview = ui->actionHigh_Quality_Preview;
	auto imView = ui->imagesView;
	imView->setProgressBar(ui->progressBar);
	imView->init();
	connect(ui->fileTree, SIGNAL(resized()), imView, SIGNAL(adjustSize()));

	sortingGroup = new QActionGroup(this);
	sortingGroup->addAction(ui->actionSort_by_full_path);
	sortingGroup->addAction(ui->actionSort_by_file_name);
	// qDebug() << "setup showpreview in Main";
	connect(ui->actionSort_by_full_path, SIGNAL(toggled(bool)), imView,
			SIGNAL(sortByPath(bool)));
	connect(ui->actionShow_Preview, SIGNAL(toggled(bool)), imView,
			SIGNAL(showPreview(bool)));

	connect(preview, SIGNAL(toggled(bool)), imView,
			SIGNAL(enableHiQPreview(bool)));
	connect(ui->actionPreload_Images, SIGNAL(toggled(bool)), imView,
			SLOT(setPrefetchImages(bool)));

	ui->actionSort_by_full_path->setChecked(
		settings.value("sortByPath", true).toBool());
	ui->actionSort_by_file_name->setChecked(
		settings.value("sortByName", true).toBool());
	ui->actionPreload_Images->setChecked(
		settings.value("preloadImages", true).toBool());

	ui->actionHigh_Quality_Preview->setChecked(
		settings.value("HiQPreview", true).toBool());
	ui->actionHigh_Quality_Preview->setEnabled(false);
	ui->actionShow_Preview->setChecked(false);
	connect(ui->actionShow_Preview, &QAction::toggled, preview,
			[preview](bool checked) { preview->setEnabled(checked); });

	ui->actionShow_Preview->setChecked(
		settings.value("showPreview", true).toBool());

	connect(ui->actionZoom_In, &QAction::triggered, imView,
			[imView]() { imView->setZoom(1); });

	connect(ui->actionZoom_Out, &QAction::triggered, imView,
			[imView]() { imView->setZoom(-1); });

	connect(ui->actionReset_zoom, &QAction::triggered, imView,
			[imView]() { imView->setZoom(0); });

	ui->menuBar->addAction("About", this, SLOT(showAbout()));
	connect(ui->actionExit, &QAction::triggered,
			[]() { QApplication::quit(); });
	connect(ui->actionExport_Images, SIGNAL(triggered(bool)), imView,
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
	connect(imView, SIGNAL(taskBarSetMaximum(int)), this,
			SLOT(setProgressMax(int)));

	connect(imView, SIGNAL(taskBarSetValue(int)), this,
			SLOT(setProgressValue(int)));
#endif
	connect(ui->fileTree, SIGNAL(changeDir(QString)), imView,
			SLOT(changeDir(QString)));

	connect(ui->fileTree, &SystemTreeView::splashText, this,
			[this](const QString &message, int alignment, const QColor &color) {
				emit splashText(message, alignment, color);
			});


	// emit splashText("aaaa", 1, Qt::blue);
	ui->fileTree->init(startDir);

	ui->infoBox->setEnabled(false);
	connect(ui->filterBox, SIGNAL(textChanged(QString)), imView,
			SLOT(applyFilter(QString)));

	setWindowState(Qt::WindowMaximized);

	statusBar()->hide();
	ui->centralWidget->setContentsMargins(0, 0, 0, 0);
	ui->fileTree->setMouseTracking(true);

#ifdef VALIDATE_LICENSE
	licenseKey = settings.value("licenseKey", "1234").toByteArray();

	auto *timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, [this, timer]() {
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


	unsigned char	   decodedLicense[256] = {0};
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
	auto	m_lineEdit = new QPlainTextEdit();
	m_lineEdit->setPlaceholderText("Enter license here");

	QPushButton *createButton = new QPushButton(tr("Ok"));
	createButton->setDefault(true);

	QPushButton *cancelButton = new QPushButton(tr("Cancel"));

	auto *buttonBox = new QDialogButtonBox(Qt::Horizontal);
	buttonBox->addButton(createButton, QDialogButtonBox::AcceptRole);
	buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

	auto *lt = new QVBoxLayout;
	qlabel->adjustSize();
	lt->addWidget(qlabel);
	lt->addWidget(m_lineEdit);
	lt->addWidget(buttonBox);

	QDialog activationDlg;
	activationDlg.setLayout(lt);

	/**
	 * Dialog created
	 */

	connect(buttonBox, &QDialogButtonBox::accepted, this, [&]() {
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
		auto *wheel = dynamic_cast<QWheelEvent *>(event);
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
