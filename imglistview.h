#ifndef IMGLISTVIEW_H
#define IMGLISTVIEW_H

#include <QListView>
#include <QGuiApplication>
#include <QScreen>
#include "imgthumbnaildelegate.h"

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);

signals:

public slots:
	void changeDir(QString dir);
private:
	QFileSystemModel* fsModel;
	ImgThumbnailDelegate* thumbnailPainter;
};

#endif // IMGLISTVIEW_H
