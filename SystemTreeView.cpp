#include "SystemTreeView.hpp"
#include "MainWindow.hpp"
#include "ThumbnailsFileModel.hpp"

SystemTreeView::SystemTreeView(QWidget *parent) : QTreeView(parent) {
	fsModel = new ThumbnailsFileModel(this);
	auto model = new QFileSystemModel(this);

	auto parentWindow = qobject_cast<MainWindow *>(parent);
	auto parentObject = parent->parent();
	while (nullptr == parentWindow) {
		parentWindow = qobject_cast<MainWindow *>(parentObject);
		parentObject = parentObject->parent();
	}
	auto rootPath = parentWindow->getRoot();

	connect(fsModel, SIGNAL(splashText(QString, int, QColor)), this,
			SIGNAL(splashText(QString, int, QColor)));

	// auto runner =
	// QtConcurrent::run([&, rootPath]() { fsModel->scanRoot(rootPath); });

	// fsModel->scanTreeFully(rootPath);

	// qDebug() << "Root: " << rootPath;
	auto rootIndex = model->setRootPath(rootPath);
	model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	fsModel->setSourceModel(model);
	fsModel->setDynamicSortFilter(false);
	setModel(fsModel);
	setRootIndex(fsModel->mapFromSource(rootIndex));
	setMouseTracking(true);

	setStyleSheet(
		"QTreeView::item:hover { background: #e5f3ff; } QTreeView::item:selected { background: #cce8ff; }\
				  QTreeView::item:selected:active { background: #cce8ff; }\
				  QTreeView::item:selected:!active { background: #cce8ff; }\
				 ");

	for (int i = 1; i < model->columnCount(); ++i)
		hideColumn(i);

	connect(selectionModel(), &QItemSelectionModel::currentChanged, this,
			[this](QModelIndex current, QModelIndex) {
				emit changeDir(fsModel->fileInfo(current).absoluteFilePath());
			});
}

void SystemTreeView::init(const QString &startDir) {

	// qDebug()<<"startDir: "<<startDir;
	runner.waitForFinished();


	QDir dir(startDir);
	auto idx = fsModel->fileIndex(startDir);
	if (idx.isValid()) {
		expand(idx);
		scrollTo(idx);
	}

	while (dir.cdUp()) {

		runner = fsModel->scanTreeAsync(dir.absolutePath());
		runner.waitForFinished();
		auto idx = fsModel->fileIndex(dir.absolutePath());
		if (idx.isValid()) {
			expand(idx);
			scrollTo(idx);
		}
	}


	idx = fsModel->fileIndex(startDir);
	// qDebug()<<"idx: "<<idx.isValid();

	expand(idx);
	scrollTo(idx);
	setCurrentIndex(idx);
}

QString SystemTreeView::getCurrentDir() {
	auto idx = currentIndex();
	// qDebug()<<"currentDir";
	return fsModel->fileInfo(idx).absoluteFilePath();
}

void SystemTreeView::initDir(QString &startDir) {
	auto idx = fsModel->fileIndex(startDir);
	// qDebug()<<"idx: "<<idx.isValid();
	setCurrentIndex(idx);
	expand(idx);
	scrollTo(idx);
	setCurrentIndex(idx);
}

void SystemTreeView::prepareExit() { fsModel->prepareExit(); }

void SystemTreeView::resizeEvent(QResizeEvent *event) {
	QTreeView::resizeEvent(event);
	emit resized();
}
