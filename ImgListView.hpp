#ifndef IMGLISTVIEW_H
#define IMGLISTVIEW_H

#include <QListView>
#include <QGuiApplication>
#include <QScreen>
#include <QtConcurrent>
#include <atomic>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QMessageBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include "FileProgressDialog.hpp"
#include "ImgThumbnailDelegate.hpp"
#include "ThumbnailsSorter.hpp"
#include <QMenu>
#include <memory>

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();
	~ImgListView();

signals:
	void callUpdate(const QString&);
	void emitUpdate(const QModelIndex&);
	void numFiles(int total, int visible);
	void setFileAction(QStringList fileList, QString destination);
	void callFullUpdate();
	void genericMessage(QString dir);

	void progressSetVisible(bool flag);
	void progressSetMaximum(int value);
	void progressSetValue(int value);
	void newRowsInserted(const QModelIndex &parent, int first, int last);
	void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
	void filterSignal(QString inFilter);
	void resetViewSignal();
	void sortByPath(bool flag);
	void showError();

public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(QString inFilter);
	void exportImages();
	void openSource();
	void checkSelections(QItemSelection, QItemSelection);
	void resetViewSlot();
	void synchronizedUpdate(const QString &fileName);
	void setZoom(int zoomDirection);

private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();
	void mousePressEvent(QMouseEvent *event) override;
	void getDirs(const QString &rootDir, QStringList& dirList);
	QString getTotalSize(QStringList& files, int skipFirstNfiles=0);
	void addHiddenFiles(QStringList& fileList);
	void leaveEvent(QEvent *) override;

	//QFileSystemModel* fsModel;
	//QSortFilterProxyModel* proxy;

	ThumbnailsSorter* newProxy;
	QStandardItemModel *newModel;
	ThumbnailsSorter* oldProxy;
	QStandardItemModel *oldModel;
	QStandardItemModel *emptyModel;

	ThumbnailsSorter* proxy0;
	ThumbnailsSorter* proxy1;
	QStandardItemModel *recursiveModel0;
	QStandardItemModel *recursiveModel1;


	ImgThumbnailDelegate* thumbnailPainter;
	QStringList namedFilters;
	QStringList sourceExtensons;
	QFuture<void> prefetchProc;
	QFuture<void> cleanerProc;
	std::atomic_bool stopPrefetching;
	QString filterText;
	//QMap<QString, QPixmap> thumbnailsCache;
	ProgressDialog* copyDialog;
	QProgressBar* dirLoadBar;
	QMenu m_menu;
	QString currentDir;
	QString exportDir;
	QIcon spinner;
	QMessageBox mb;

	QAction* exportAction;
	QAction* openAction;
	QAction* openSourceAction;
	QAction* fi_selectedFiles;
	QAction* fi_fileFormat;
	QAction* fi_bitDepth;
	QAction* fi_grayScale;
	QAction* fi_size;
	QAction* fi_alpha;
	QList<QStandardItem*> items;

	QMutex cleanerMutex;
	QWaitCondition synchronizer;

	int default_icon_size;
	int default_grid_spacing;
};

#endif // IMGLISTVIEW_H
