#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include "imglistview.h"
#include "systemtreeview.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void setFileInfo(int total, int visible);

private:
	void readSettings();
	void saveSettings();

	Ui::MainWindow *ui;
	QString startDir;
};

#endif // MAINWINDOW_H
