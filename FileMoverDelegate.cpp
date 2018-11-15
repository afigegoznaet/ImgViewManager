#include "FileMoverDelegate.hpp"
#include <QDebug>
#include <QStorageInfo>
#include "FileProgressDialog.hpp"

/*
#ifdef _WIN32
#include <windows.h>
#endif
*/

#define MAX_READ 1048576

bool isMovable(QString &from, QString &to){
	QStorageInfo in, out;
	in.setPath(from);
	out.setPath(to.mid(0,to.lastIndexOf('/')));

	//qDebug()<<in.rootPath();
	//qDebug()<<out.rootPath();
	//qDebug()<<(in == out);
	return in == out;
}

int FileMoverDelegate::copy(){
	QFile sourceFile(source);
	QFile destinationFile(destination);

	sourceFile.open(QIODevice::ReadOnly);
	destinationFile.open(QIODevice::WriteOnly | QIODevice::Truncate);//Need to add a question dialog for this

	//quint64 totalSize = sourceFile.size();
	quint64 tempSize = 0;

	char buffer[MAX_READ];//1 Mb
	qint64 bytesRead=0;
	QMutex blocker;
	blocker.lock();

	bytesRead = sourceFile.read(buffer,MAX_READ);

	while(bytesRead>0){
		switch(status){
		case 0:
			cond.wait(&blocker);
			break;
		case -1:
			sourceFile.close();
			destinationFile.close();
			destinationFile.remove();
			return true;
		}

		if(destinationFile.write(buffer, bytesRead) < 0)
			return false;
		tempSize +=bytesRead;
		//emit bytesProgress((uint)(tempSize*100. / totalSize*1.) );
		bytesRead = sourceFile.read(buffer,MAX_READ);
	}

	blocker.unlock();
	sourceFile.close();
	destinationFile.close();
	if(bytesRead < 0){
		qDebug()<<sourceFile.errorString();
		return false;
	}
	return true;
}

int FileMoverDelegate::move(){

	return 10 + (int)QFile::rename(source, destination);
}
/*
void FileMover::execute(){

}
*/
FileMoverDelegate::~FileMoverDelegate(){

	int res = 0;
	if(!action.compare("Copy",Qt::CaseInsensitive)){
		res = this->copy();
	}else{
		if(isMovable(source, destination))
			res = this->move();
		else
			res = this->copy();
	}

	emit completed(res);
	qDebug()<<"FileMover completed?!";

}

FileMoverDelegate::FileMoverDelegate(QString source, QString destination, QString action, QObject *parent) :
	QObject(parent), destination(destination), source(source), action(action), status(true){
	//connect(this, SIGNAL(completed(int)),qobject_cast<ProgressDialog*>(parent),SLOT(movementResult(int)), Qt::QueuedConnection);

	//qDebug()<<"Mover constructor"<<thread();
}

void FileMoverDelegate::setStatus(int status){
	this->status = status;
	qDebug()<<"*************************************";
	qDebug()<<"status "<<status<<" emmitted";
	cond.wakeOne();
}

