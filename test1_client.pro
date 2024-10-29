QT       += core gui xml network multimedia core5compat

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AccountWidget.cpp \
    AlbumInfo.cpp \
    AlbumShowPage.cpp \
    ConfigXml.cpp \
    EditCollect.cpp \
    InfoBar.cpp \
    Login.cpp \
    MusicClient.cpp \
    MusicListPage.cpp \
    MusicPostPage.cpp \
    MusicTcpClient.cpp \
    PlayController.cpp \
    PostWidget.cpp \
    ReigsterWidget.cpp \
    SelectContain.cpp \
    SettingPage.cpp \
    common.cpp \
    main.cpp

HEADERS += \
    common.h \
    AccountWidget.h \
    AlbumInfo.h \
    AlbumShowPage.h \
    ConfigXml.h \
    EditCollect.h \
    InfoBar.h \
    Login.h \
    MusicClient.h \
    MusicListPage.h \
    MusicPostPage.h \
    MusicTcpClient.h \
    PlayController.h \
    PostWidget.h \
    ReigsterWidget.h \
    SelectContain.h \
    SettingPage.h

FORMS += \
    AccountWidget.ui \
    AlbumInfo.ui \
    AlbumShowPage.ui \
    EditCollect.ui \
    InfoBar.ui \
    Login.ui \
    MusicClient.ui \
    MusicListPage.ui \
    MusicPostPage.ui \
    ReigsterWidget.ui \
    SelectContain.ui \
    SettingPage.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

DISTFILES +=
