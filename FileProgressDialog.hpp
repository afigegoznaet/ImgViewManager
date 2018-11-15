#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>

namespace Ui {
	class ProgressDialog;
}

class ProgressDialog : public QDialog {
	Q_OBJECT

	// friend class MainWindow;
public:
	explicit ProgressDialog(QWidget *parent = 0,
							Qt::WindowFlags f = Qt::WindowFlags() | Qt::Window);
	~ProgressDialog();

signals:
	void sendErrMsg(QString errorText);
	void hideDialogSignal();
	void dirMoved(int);
	void setStatus(int status);

public slots:
	void onWrite(uint);
	void movementResult(int);
	void dirMovementResult(int);
	void errorMsg(const QString& errorText);
	void hideDialogSlot();
	void processFileAction(QStringList fileList, const QString& destination);

private:
	Ui::ProgressDialog *progress;
	QFuture<void> stub;
	bool status;
	QWaitCondition cond;
	QWaitCondition condStatus;
	QString pauseButtonLabels[2] = {"Continue", "Pause"};
	QMutex moverBlocker;
	int counter = 0;
	QFutureWatcher<void> watcher;

	void DoSomething(void);
	QMessageBox::StandardButton showError(int result);
};

#endif // DIALOG_H
