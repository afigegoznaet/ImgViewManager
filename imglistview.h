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
#include "imgthumbnaildelegate.h"
#include <QMenu>
#include <memory>

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();
	~ImgListView(){delete exportAction;}

signals:
	void callUpdate(const QModelIndex &);
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

public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(QString inFilter);
	void exportImages();
	void checkSelections(QItemSelection, QItemSelection);
	void resetViewSlot();
private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();
	void mousePressEvent(QMouseEvent *event) override;
	void getDirs(const QString &rootDir, QStringList& dirList);
	QString getTotalSize(QStringList& files);

	//QFileSystemModel* fsModel;
	//QSortFilterProxyModel* proxy;

	QSortFilterProxyModel* newProxy;
	QStandardItemModel *newModel;
	QSortFilterProxyModel* oldProxy;
	QStandardItemModel *oldModel;

	QSortFilterProxyModel* proxy0;
	QSortFilterProxyModel* proxy1;
	QStandardItemModel *recursiveModel0;
	QStandardItemModel *recursiveModel1;
	ImgThumbnailDelegate* thumbnailPainter;
	QStringList namedFilters;
	QFuture<void> prefetchProc;
	std::atomic_bool stopPrefetching;
	QString filterText;
	QHash<QString, QPixmap> thumbnailsCache;
	ProgressDialog* copyDialog;
	QProgressBar* dirLoadBar;
	QMenu m_menu;
	QString currentDir;

	QAction* exportAction;
	QAction* openAction;
	QAction* fi_selectedFiles;
	QAction* fi_fileFormat;
	QAction* fi_bitDepth;
	QAction* fi_grayScale;
	QAction* fi_size;
	QAction* fi_alpha;
	QList<QStandardItem*> items;
};

#endif // IMGLISTVIEW_H
