QT       += \
    core gui \
    core gui widgets opengl\
    opengl widgets \
    core

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# 如果使用Qt6，还需要添加
greaterThan(QT_MAJOR_VERSION, 5) {
    QT += openglwidgets
}

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    codeeditor.cpp \
    main.cpp \
    mainwindow.cpp \
    syntaxhighlighter.cpp

HEADERS += \
    codeeditor.h \
    json.hpp \
    mainwindow.h \
    mapSizeDialog.h \
    syntaxhighlighter.h \
    version.h

# 添加包含路径（根据实际路径修改）
INCLUDEPATH += $$PWD/third_party/json-develop/include

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    test_zh_CN.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

# Windows特定设置（生成exe时的图标）
win32 {
    RC_ICONS = icons/app_icon.ico
}

DISTFILES += \
    log.md
