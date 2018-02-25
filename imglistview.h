#ifndef IMGLISTVIEW_H
#define IMGLISTVIEW_H

#include <QListView>
#include <QGuiApplication>
#include <QScreen>
#include <QtConcurrent>
#include <atomic>
#include <QKeyEvent>
#include <QDesktopServices>
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
public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(QString namedFilters);
private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();


	//QFileSystemModel* fsModel;
	ThumbnailsFileModel* proxyModel;
	ImgThumbnailDelegate* thumbnailPainter;
	QStringList namedFilters;
	QFuture<void> prefetchProc;
	std::atomic_bool isExiting;
	QString filterText;
};

#endif // IMGLISTVIEW_H
