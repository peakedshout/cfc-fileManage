QT       += core gui charts
#QT += webenginewidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17


#QMAKE_CXXFLAGS += /source-charset:utf-8
#QMAKE_CXXFLAGS += /execution-charset:utf-8

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AsycTask/AsycDownloadFile.cpp \
    AsycTask/AsycGetProgress.cpp \
    AsycTask/AsycNetStatus.cpp \
    AsycTask/AsycReadFile.cpp \
    AsycTask/AsycUploadFile.cpp \
    ConnectServer/CManage.cpp \
    ConnectServer/ConnectInfoDialog.cpp \
    ConnectServer/CountDown.cpp \
    DebugMsg/DebugMsgWidget.cpp \
    DownloadFile/DownloadFileItem.cpp \
    DownloadFile/DownloadFileWidget.cpp \
    Encryption/AEC_Encryption.cpp \
    Encryption/qaesencryption.cpp \
    FileManageApi/LocalFileManageApi.cpp \
    FileManageApi/RemoteFileManageApi.cpp \
    FileManageWidget.cpp \
    FileManageWidget/LocalFileManageMainWin.cpp \
    FileManageWidget/RemoteFileManageMainWin.cpp \
    FileManageWidget/TableView.cpp \
    FileOperation/FileOperation.cpp \
    RewriteApi/CFCFileApiParse.cpp \
    RewriteApi/GoStr.cpp \
    ScanCFCFile/ScanCFCFileWidget.cpp \
    SessionMsg/SessionMsg.cpp \
    SessionMsg/SessionMsgDialog.cpp \
    System/SystemInfo.cpp \
    UploadFile/UploadFileItem.cpp \
    UploadFile/UploadFileWidget.cpp \
    Widget.cpp \
    main.cpp \

HEADERS += \
    AsycTask/AsycDownloadFile.h \
    AsycTask/AsycGetProgress.h \
    AsycTask/AsycNetStatus.h \
    AsycTask/AsycReadFile.h \
    AsycTask/AsycUploadFile.h \
    ConnectServer/CManage.h \
    ConnectServer/ConnectInfoDialog.h \
    ConnectServer/CountDown.h \
    DebugMsg/DebugMsgWidget.h \
    DownloadFile/DownloadFileItem.h \
    DownloadFile/DownloadFileWidget.h \
    Encryption/AEC_Encryption.h \
    Encryption/qaesencryption.h \
    FileManageApi/LocalFileManageApi.h \
    FileManageApi/RemoteFileManageApi.h \
    FileManageApi/VirtualFileManageApi.h \
    FileManageWidget.h \
    FileManageWidget/LocalFileManageMainWin.h \
    FileManageWidget/RemoteFileManageMainWin.h \
    FileManageWidget/TableView.h \
    FileOperation/FileOperation.h \
    RewriteApi/CFCFileApiParse.h \
    RewriteApi/GoStr.h \
    RewriteApi/ScanCFCFile.h \
    RewriteApi/cfcfile_api_win_amd64.h \
    RewriteApi/cfcfile_api2.h \
    ScanCFCFile/ScanCFCFileWidget.h \
    SessionMsg/SessionMsg.h \
    SessionMsg/SessionMsgDialog.h \
    System/SystemInfo.h \
    UploadFile/UploadFileItem.h \
    UploadFile/UploadFileWidget.h \
    Widget.h \

FORMS += \
    ConnectServer/ConnectInfoDialog.ui \
    ConnectServer/CountDown.ui \
    DebugMsg/DebugMsgWidget.ui \
    DownloadFile/DownloadFileItem.ui \
    DownloadFile/DownloadFileWidget.ui \
    FileManageWidget/LocalFileManageMainWin.ui \
    FileManageWidget/RemoteFileManageMainWin.ui \
    FileOperation/FileOperation.ui \
    ScanCFCFile/ScanCFCFileWidget.ui \
    SessionMsg/SessionMsgDialog.ui \
    UploadFile/UploadFileItem.ui \
    UploadFile/UploadFileWidget.ui \
    Widget.ui

include(base.pri)

INCLUDEPATH += $$PDW
LIBS += -L$$LIBDIR -lcfcfile_api_win_amd64

RC_ICONS = cfloge.ico



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
