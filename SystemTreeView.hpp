#ifndef SYSTEMTREEVIEW_H
#define SYSTEMTREEVIEW_H

#include <QTreeView>
#include <QHeaderView>
#include "ThumbnailsFileModel.hpp"

class SystemTreeView : public QTreeView{
	Q_OBJECT
public:
	explicit SystemTreeView(QWidget *parent = nullptr);
	void init(QString& startDir);
	void initDir(QString& startDir);
	QString getCurrentDir();
	void prepareExit(){fsModel->prepareExit();}

signals:
	void changeDir(QString dir);
	void splashText(const QString& message, int alignment, const QColor &color);
public slots:

private:
	ThumbnailsFileModel* fsModel;
	QFuture<bool> runner;
	QFutureWatcher<void> watcher;
};

#endif // SYSTEMTREEVIEW_H
