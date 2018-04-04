#-------------------------------------------------
#
# Project created by QtCreator 2018-02-17T12:51:51
#
#-------------------------------------------------

QT       += core gui concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImgViewManager
TEMPLATE = app

#QMAKE_CXXFLAGS+="-fsanitize=address"
#QMAKE_CXXFLAGS+="-fsanitize=thread"
#QMAKE_CFLAGS+="-fsanitize=address -fno-omit-frame-pointer"
#QMAKE_LFLAGS+="-fsanitize=thread -shared"
#QMAKE_CXXFLAGS+="-fsanitize=address  -fomit-frame-pointer"
#QMAKE_LFLAGS+=" -fsanitize=address"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
		main.cpp \
		mainwindow.cpp \
	systemtreeview.cpp \
	imglistview.cpp \
	imgthumbnaildelegate.cpp \
	thumbnailsfilemodel.cpp \
	FileMoverDelegate.cpp \
		FileProgressDialog.cpp

HEADERS += \
		mainwindow.h \
	systemtreeview.h \
	imglistview.h \
	imgthumbnaildelegate.h \
	thumbnailsfilemodel.h \
	FileMoverDelegate.hpp \
		FileProgressDialog.hpp

FORMS += \
		mainwindow.ui \
	progressDialog.ui

RESOURCES += \
	imgviewmanager.qrc


#DEFINES += NO_VALIDATION
CONFIG += SODIUM


win32-msvc* {
	INCLUDEPATH += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\include"
	QMAKE_LFLAGS_WINDOWS += /STACK:10485760,655360
	QMAKE_CXXFLAGS += /MP
}
SODIUM {
	DEFINES += VALIDATE_LICENSE
	unix:LIBS += -lsodium
	win32:LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Release\\v140\\dynamic\\libsodium.lib"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Release\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Release\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Debug\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Debug\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Release\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Release\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Debug\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Debug\\v141\\dynamic"
}
