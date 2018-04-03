#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QLabel>
#include <QTextEdit>
#include <QDialogButtonBox>
#include "imglistview.h"
#include "systemtreeview.h"
#include <sodium.h>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow{
	Q_OBJECT

signals:
	void splashText(const QString& message, int alignment, const QColor &color);
public:
	//explicit MainWindow(QWidget *parent = 0);
	explicit MainWindow(QString argv, QWidget *parent = 0);
	~MainWindow();
	void init();
	void initTree();
	QString getRoot(){return rootDir;}

public slots:
	void setFileInfo(int total, int visible);
    void setScanDirMsg(QString msg);
	void showAbout();

private:
	void saveSettings();
	void initActivation();
	QByteArray licenseKey;

	Ui::MainWindow *ui;
	QString startDir;
	QString rootDir;
	QByteArray splitterSizes;
	QString info = "";
	QString args;
};

#endif // MAINWINDOW_H
