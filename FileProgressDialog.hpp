#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFileInfoList>
#include <QSettings>
#include <QPushButton>
#include "ui_progressDialog.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <set>
#include "FileMoverDelegate.hpp"

namespace Ui {
	class ProgressDialog;
}

class ProgressDialog : public QDialog
{
	Q_OBJECT

	//friend class MainWindow;
public:
	explicit ProgressDialog(QWidget *parent = 0, Qt::WindowFlags f = Qt::WindowFlags() | Qt::Window);
	~ProgressDialog();

private:
	Ui::ProgressDialog *progress;
	QFuture<void> stub;
	bool status;
	QWaitCondition cond;
	QWaitCondition condStatus;
	QString pauseButtonLabels[2]= {"Continue","Pause"};
	QMutex moverBlocker;
	int counter = 0;

	void DoSomething(void);

	QMessageBox::StandardButton showError(int result);

signals:
	void sendErrMsg(QString errorText);
	void hideDialogSignal();
	void dirMoved(int);
	void setStatus(int status);

public slots:
	void onWrite( uint );
	void movementResult(int);
	void dirMovementResult(int);
	void errorMsg(QString errorText);
	void hideDialogSlot();
	void processFileAction(QFileInfoList fileList, QString destination );

};

#endif // DIALOG_H
