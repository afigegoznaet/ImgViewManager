#include "FileProgressDialog.hpp"

ProgressDialog::ProgressDialog(QWidget *parent, Qt::WindowFlags f) :
	QDialog(parent, f),
	progress(new Ui::ProgressDialog),
	status(1){
	progress->setupUi(this);
	progress->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	progress->tableWidget->setColumnCount(2);
	progress->tableWidget->verticalHeader()->setDefaultSectionSize(this->fontMetrics().height()+6);

	progress->tableWidget->horizontalHeader()->setStretchLastSection(true);
	progress->tableWidget->horizontalHeader()->setSectionsMovable(true);
	progress->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

	QStringList m_TableHeader;
	m_TableHeader<<"Filename"<<"Destination folder";
	progress->tableWidget->setHorizontalHeaderLabels(m_TableHeader);
	progress->progressBar->setMinimum(0);
	progress->progressBar->setMaximum( 100 );
	//connect(this, SIGNAL(dirMoved(int)),this,SLOT(movementResult(int)));

	connect(this, SIGNAL(sendErrMsg(QString )), this,SLOT(errorMsg(QString )), Qt::QueuedConnection);
	connect(this, SIGNAL(hideDialogSignal()), this,SLOT(hideDialogSlot()), Qt::QueuedConnection);

	QSettings settings;
	auto headerState = settings.value("ProgressColumns").toByteArray();
	progress->tableWidget->horizontalHeader()->restoreState(headerState);
}

ProgressDialog::~ProgressDialog(){
	QSettings settings;
	settings.setValue("ProgressColumns", progress->tableWidget->horizontalHeader()->saveState());
	delete progress;
}

void ProgressDialog::processFileAction(QStringList fileList,
						QString destination){
	if(isHidden())
		show();
	if(!progress->tableWidget->rowCount())
		status = 1;

	int initialCount = progress->tableWidget->rowCount();
	int newRow = progress->tableWidget->rowCount();


	foreach (auto fileName, fileList) {

		if(!fileName.compare("..", Qt::CaseInsensitive)
				|| !fileName.compare(".", Qt::CaseInsensitive)  )
			continue;


		qDebug()<<fileName;

		QString item = "Move "+ fileName + " to "+destination;
		QString newName = destination+"/"+fileName.split('/').last();

		progress->tableWidget->insertRow( newRow );

		progress->tableWidget->setItem(newRow,0,new QTableWidgetItem(fileName));
		progress->tableWidget->setItem(newRow,1,new QTableWidgetItem(destination));
	}

	counter = 0;
	progress->progressBar->setMaximum( progress->tableWidget->rowCount() );
	progress->progressBar->setValue(counter);
	if(0==initialCount)
		QtConcurrent::run(this, &ProgressDialog::DoSomething);

}

void ProgressDialog::onWrite(uint percentsWritten){
	if(percentsWritten > (uint)progress->progressBar->value())
		progress->progressBar->setValue(percentsWritten);
	if(100 == percentsWritten)
		progress->progressBar->setValue(0);
}

QMessageBox::StandardButton ProgressDialog::showError(int result){

	status &= (result & 1);

	static const QString errorCopyMasg = "Copying file failed!\nContinue?";
	static const QString errorMoveMasg = "Moving file failed!\nContinue?";
	static const QString errorCopyEOLMasg = "Copying file failed!";
	static const QString errorMoveEOLMasg = "Moving file failed!";
	QMessageBox::StandardButton reply = QMessageBox::Yes;
	switch(result){
		case 10://Move failed
			if(progress->tableWidget->rowCount()>1)
				reply = QMessageBox::question(this, "Error!!!", errorMoveMasg,
											QMessageBox::Yes|QMessageBox::No);
			if(reply == QMessageBox::Yes)
				status = 1;
			else
				QMessageBox::warning(this, "Error!!!", errorMoveEOLMasg);



			break;
		case 0:
			if(progress->tableWidget->rowCount()>1)
				reply = QMessageBox::question(this, "Error!!!", errorCopyMasg,
											QMessageBox::Yes|QMessageBox::No);
			if(reply == QMessageBox::Yes)
				status = 1;
			else
				reply = QMessageBox::question(this, "Error!!!", errorCopyEOLMasg);

			break;
	}
	return reply;
}

void ProgressDialog::movementResult(int result){

	moverBlocker.lock();

	auto reply = showError(result);
	if(reply == QMessageBox::Yes){
		//status = 1;
		progress->progressBar->setValue(++counter);
		if(progress->tableWidget->rowCount())
			progress->tableWidget->removeRow(0);
		QtConcurrent::run(this, &ProgressDialog::DoSomething);
	}
	moverBlocker.unlock();
}


void ProgressDialog::dirMovementResult(int result){
	moverBlocker.lock();
	showError(result);
	moverBlocker.unlock();
}


void ProgressDialog::errorMsg(QString errorText){
	QMessageBox::warning(this, "Error",errorText);
	cond.wakeOne();
}
void ProgressDialog::hideDialogSlot(){
	this->hide();
}


void ProgressDialog::DoSomething(void){
	if (status && progress->tableWidget->rowCount()) {
			//stub.waitForFinished();
		QString source( progress->tableWidget->item(0,0)->text() );
		QString destination( progress->tableWidget->item(0,1)->text() );
		destination.append("/");
		QFileInfo fileInfo(source);
		QString fileName(fileInfo.fileName());


		qDebug()<<source;
		destination.append(fileName);
		FileMoverDelegate mover(source, destination, "Copy");

		//connect(&mover,SIGNAL(bytesProgress(uint)), this, SLOT(onWrite(uint)));
		connect(&mover, SIGNAL(completed(int)),this,SLOT(movementResult(int)), Qt::QueuedConnection);
		connect(this, SIGNAL(setStatus(int)),&mover,SLOT(setStatus(int)), Qt::QueuedConnection);
		//emit setStatus(status);

		//delete mover;

	}else{
		//setWindowTitle("");
		//this->hide();
		emit hideDialogSignal();
	}

}
