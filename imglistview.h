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
#include "FileProgressDialog.hpp"
#include "imgthumbnaildelegate.h"
#include <QMenu>

class ImgListView : public QListView{
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();
	~ImgListView(){delete exportAction;}

signals:
	void callUpdate(const QModelIndex &);
	void numFiles(int total, int visible);
	void setFileAction(QFileInfoList fileList, QString destination);
	void callFullUpdate();

	void progressSetVisible(bool flag);
	void progressSetMaximum(int value);
	void progressSetValue(int value);


public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(QString inFilter);
	void exportImages();
private:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();
	void mousePressEvent(QMouseEvent *event) override;

	QFileSystemModel* fsModel;
	//QFileSystemModel* proxyModel;
	ImgThumbnailDelegate* thumbnailPainter;
	QStringList namedFilters;
	QFuture<void> prefetchProc;
	std::atomic_bool stopPrefetching;
	QString filterText;
	QHash<QString, QImage> thumbnailsCache;
	ProgressDialog* copyDialog;
	QProgressBar* dirLoadBar;
	QMenu m_menu;
	QAction* exportAction;
	QAction* openAction;
	QAction* fi_selectedFiles;
	QAction* fi_fileFormat;
	QAction* fi_bitDepth;
	QAction* fi_grayScale;
	QAction* fi_size;
	QAction* fi_alpha;

};

#endif // IMGLISTVIEW_H
