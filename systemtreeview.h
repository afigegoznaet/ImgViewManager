#ifndef SYSTEMTREEVIEW_H
#define SYSTEMTREEVIEW_H

#include <QTreeView>
#include <QDebug>
#include <QFileSystemModel>
#include <QHeaderView>

class SystemTreeView : public QTreeView{
	Q_OBJECT
public:
	explicit SystemTreeView(QWidget *parent = nullptr);
	void init(QString& startDir);
	QString getCurrentDir();

signals:
	void changeDir(QString dir);
public slots:
private:
	QFileSystemModel* fsModel;
};

#endif // SYSTEMTREEVIEW_H
