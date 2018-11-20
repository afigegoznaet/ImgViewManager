#ifndef IMGLISTVIEW_H
#define IMGLISTVIEW_H

#include <QListView>
#include <QMenu>
#include <QtConcurrent>
#include <QMessageBox>

class ProgressDialog;
class QProgressBar;
class ThumbnailsSorter;
class QStandardItemModel;
class ImgThumbnailDelegate;
class QStandardItem;

class ImgListView : public QListView {
	Q_OBJECT
public:
	explicit ImgListView(QWidget *parent = nullptr);
	void prepareExit();
	~ImgListView();
	const QString getFileName(const QModelIndex &index) const;

signals:
	void callUpdate(const QString &);
	void emitUpdate(const QModelIndex &);
	void numFiles(int total, int visible);
	void setFileAction(QStringList fileList, QString destination);
	void callFullUpdate();
	void genericMessage(QString dir);

	void progressSetVisible(bool flag);
	void progressSetMaximum(int value);
	void progressSetValue(int value);
	void taskBarSetMaximum(int value);
	void taskBarSetValue(int value);
	void newRowsInserted(const QModelIndex &parent, int first, int last);
	void rowsAboutToBeInserted(const QModelIndex &parent, int start, int end);
	void filterSignal(QString inFilter);
	void resetViewSignal();
	void sortByPath(bool flag);
	void showPreview(bool flag);
	void enableHiQPreview(bool flag);
	void showError();
	void scrollToIndex(const QModelIndex idx);

public slots:
	void changeDir(QString dir);
	void onDoubleClicked();
	void applyFilter(const QString &inFilter);
	void exportImages();
	void openSource();
	void checkSelections(const QItemSelection &, const QItemSelection &);
	void resetViewSlot();
	void synchronizedUpdate(const QString &fileName);
	void setZoom(int zoomDirection);
	void setScrolling(bool flag) { autoScroll = flag; }

protected:
	void keyPressEvent(QKeyEvent *event) override;
	void prefetchThumbnails();
	void mousePressEvent(QMouseEvent *event) override;
	void getDirs(const QString &rootDir, QStringList &dirList);
	QString getTotalSize(QStringList &files, int skipFirstNfiles = 0);
	void addHiddenFiles(QStringList &fileList);
	void leaveEvent(QEvent *) override;
	void paintEvent(QPaintEvent *event) override;

	// QFileSystemModel* fsModel;
	// QSortFilterProxyModel* proxy;
private:
	ThumbnailsSorter *newProxy{};
	QStandardItemModel *newModel{};
	ThumbnailsSorter *oldProxy{};
	QStandardItemModel *oldModel{};

	ThumbnailsSorter *proxy0;
	ThumbnailsSorter *proxy1;
	QStandardItemModel *recursiveModel0;
	QStandardItemModel *recursiveModel1;

	ImgThumbnailDelegate *thumbnailPainter;


	QFuture<void> prefetchProc;
	QFuture<void> cleanerProc;
	std::atomic_bool stopPrefetching;
	std::atomic_bool autoScroll{};
	QString filterText;
	// QMap<QString, QPixmap> thumbnailsCache;
	ProgressDialog *copyDialog;
	QProgressBar *dirLoadBar;
	QMenu m_menu;
	QString currentDir;
	QString exportDir;
	QIcon spinner;
	QMessageBox mb;
	QStringList namedFilters;
	const QStringList sourceExtensons;

	QAction *exportAction;
	QAction *openAction;
	QAction *openSourceAction;
	QAction *fi_selectedFiles;
	QAction *fi_fileFormat;
	QAction *fi_bitDepth;
	QAction *fi_grayScale;
	QAction *fi_size;
	QAction *fi_alpha;
	QList<QStandardItem *> items;

	QMutex cleanerMutex;
	QWaitCondition synchronizer;

	int default_icon_size;
	int default_grid_spacing{};
};

#endif // IMGLISTVIEW_H
