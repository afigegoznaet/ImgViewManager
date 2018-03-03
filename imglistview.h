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
#include "FileProgressDialog.hpp"
#include "imgthumbnaildelegate.h"
#include "thumbnailsfilemodel.h"

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();

signals:
	void callUpdate(const QModelIndex &);
	void numFiles(int total, int visible);
	void setFileAction(QFileInfoList fileList, QString destination);
	void callFullUpdate();
public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(QString namedFilters);
	void exportImages();
private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();


	//QFileSystemModel* fsModel;
	ThumbnailsFileModel* proxyModel;
	ImgThumbnailDelegate* thumbnailPainter;
	QStringList namedFilters;
	QFuture<void> prefetchProc;
	std::atomic_bool stopPrefetching;
	QString filterText;
	QHash<QString, QPixmap> thumbnailsCache;
	ProgressDialog* copyDialog;
};

#endif // IMGLISTVIEW_H
