#ifndef SYSTEMTREEVIEW_H
#define SYSTEMTREEVIEW_H

#include <QTreeView>
#include <QHeaderView>
#include "thumbnailsfilemodel.h"

class SystemTreeView : public QTreeView{
	Q_OBJECT
public:
	explicit SystemTreeView(QWidget *parent = nullptr);
	void init(QString& startDir);
	QString getCurrentDir();

signals:
	void changeDir(QString dir);
	void splashText(const QString& message, int alignment, const QColor &color);
public slots:
	void expanSionSlot(const QModelIndex &index);
private:
	ThumbnailsFileModel* fsModel;
	QFuture<bool> runner;
};

#endif // SYSTEMTREEVIEW_H
