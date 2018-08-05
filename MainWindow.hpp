#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QFileDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include "ImgListView.hpp"
#include "SystemTreeView.hpp"
#ifdef VALIDATE_LICENSE
#include <sodium.h>
#endif

namespace Ui { class MainWindow; }
//namespace QtWinExtras { class QWinTaskbarProgress;}
//using namespace QtWinExtras;
class QWinTaskbarProgress;

class MainWindow : public QMainWindow{
	Q_OBJECT

signals:
	void splashText(const QString& message, int alignment, const QColor &color);

public:
	//explicit MainWindow(QWidget *parent = 0);
	explicit MainWindow(QString argv, QWidget *parent = nullptr);
	~MainWindow() override;
	void init();
	void initTree();
	QString getRoot(){return rootDir;}
	bool eventFilter(QObject *watched, QEvent *event) override;

public slots:
	void setFileInfo(int total, int visible);
	void setScanDirMsg(QString msg);
	void showAbout();
#ifdef _WIN32
	void initProgressTaskbar();
	void setProgressMax(int max);
	void setProgressValue(int value);

#endif

private:
	void saveSettings();
	void initActivation();
	QByteArray licenseKey;
	QActionGroup* sortingGroup;
	Ui::MainWindow *ui;
	QString startDir;
	QString rootDir;
	QByteArray splitterSizes;
	QString info = "";
	QString args;
#ifdef _WIN32
	QWinTaskbarProgress *progress;
	int cachedProgress;
#endif
};

#endif // MAINWINDOW_H
