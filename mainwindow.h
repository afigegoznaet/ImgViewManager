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

signals:
	void splashText(const QString& message, int alignment, const QColor &color);
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	void init();

public slots:
	void setFileInfo(int total, int visible);
	void showAbout();

private:
	void readSettings();
	void saveSettings();

	Ui::MainWindow *ui;
	QString startDir;
	QString info = "";
};

#endif // MAINWINDOW_H
