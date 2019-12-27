#-------------------------------------------------
#
# Project created by QtCreator 2018-02-17T12:51:51
#
#-------------------------------------------------

QT       += core gui concurrent widgets

TARGET = ivm
win32 {
	QT       += winextras
}
TEMPLATE = app

CONFIG += c++17

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
	FileMoverDelegate.cpp \
		FileProgressDialog.cpp \
	ImgListView.cpp \
	ImgThumbnailDelegate.cpp \
	MainWindow.cpp \
	SystemTreeView.cpp \
	ThumbnailsFileModel.cpp \
	ThumbnailsSorter.cpp

HEADERS += \
	FileMoverDelegate.hpp \
		FileProgressDialog.hpp \
	ImgListView.hpp \
	ImgThumbnailDelegate.hpp \
	MainWindow.hpp \
	SystemTreeView.hpp \
	ThumbnailsFileModel.hpp \
	ThumbnailsSorter.hpp

FORMS += \
		mainwindow.ui \
	progressDialog.ui

RESOURCES += \
	imgviewmanager.qrc


CONFIG += SODIUM


linux {
	target.path = /usr/local/bin
	desktop.path = /usr/share/applications
	desktop.files += ivm.desktop
	icon.path = /usr/share/icons/hicolor/16x16/apps/
	icon.files = Resources/tropical_paradise_icon.png
	INSTALLS += target desktop icon
	#QMAKE_CXXFLAGS+="-fsanitize=address"
	#QMAKE_CXXFLAGS+="-fsanitize=thread"
	#QMAKE_CFLAGS+="-fsanitize=address -fno-omit-frame-pointer"
	#QMAKE_LFLAGS+="-fsanitize=thread -shared"
	#QMAKE_CXXFLAGS+="-fsanitize=address  -fomit-frame-pointer"
	#QMAKE_LFLAGS+=" -fsanitize=address"
	QMAKE_CXXFLAGS+="-fno-exceptions"
CONFIG(debug, debug|release) {
  message( "debug" )
QMAKE_CXXFLAGS+="-fsanitize=address"
QMAKE_LFLAGS+=" -fsanitize=address"
}
}


win32-* {
	RC_ICONS = Resources\tropical_paradise_icon_meJ_icon.ico
}


win32-msvc* {
        #INCLUDEPATH += "..\\libsodium-1.0.16-msvc\\include"
		QMAKE_LFLAGS_WINDOWS += /STACK:10485760,655360
		QMAKE_CXXFLAGS += /MP
        #LIBS += "..\\libsodium-1.0.16-msvc\\x64\\Release\\v141\\dynamic\\libsodium.lib"
		#LIBS += "..\\libsodium-1.0.16-msvc\\Win32\\Release\\v141\\dynamic\\libsodium.lib"
}
win32-g++ {
        #INCLUDEPATH += "..\\libsodium-win32\\include"
        #LIBS += "..\\libsodium-win32\\lib\\libsodium.dll.a"
}
SODIUM {
    #DEFINES += VALIDATE_LICENSE
    #unix:LIBS += -lsodium

#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Release\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Release\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Debug\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Debug\\v140\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Release\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Release\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\Win32\\Debug\\v141\\dynamic"
#LIBS += "C:\\torrents\\Soft\\libsodium-1.0.16-msvc\\x64\\Debug\\v141\\dynamic"
}
