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

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();

signals:
	void callUpdate(const QModelIndex &);
public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();


	QFileSystemModel* fsModel;
	ImgThumbnailDelegate* thumbnailPainter;
	QStringList filter;
	QFuture<void> prefetchProc;
	std::atomic_bool isExiting;
};

#endif // IMGLISTVIEW_H
