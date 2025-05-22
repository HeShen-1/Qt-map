#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QIcon>

#include "version.h"

int main(int argc, char *argv[])
{
    // 初始化Qt应用框架
    QApplication a(argc, argv);

    // 在设置图标前添加资源初始化
    Q_INIT_RESOURCE(resources);

    // 版本
    a.setApplicationVersion(VERSION_STRING(APP_VERSION_MAJOR,
                                           APP_VERSION_MINOR,
                                           APP_VERSION_PATCH));

    // 设置应用程序元数据
    a.setApplicationName("RasterMapEditor");
    a.setOrganizationName("GeoSoft");
     // 设置窗口图标（需确保资源文件存在）
    a.setWindowIcon(QIcon(":/icons/app_icon.ico"));

    // 多语言支持模块
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) { // 遍历系统语言列表
        const QString baseName = "RasterMapEditor_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {    // 从资源文件加载翻译
            a.installTranslator(&translator);
            break;  // 使用第一个匹配的语言文件
        }
    }

    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("栅格地图编辑器");
    parser.addHelpOption();     // --help选项
    parser.addVersionOption();  // 正确的版本选项
    parser.addPositionalArgument("file", "要打开的地图文件");   // 文件参数
    parser.process(a);   // 执行解析

    const QStringList args = parser.positionalArguments();
    QString initialFile;
    if (!args.isEmpty()) {
        initialFile = args.first();
    }

    // 主窗口模块（含异常捕获）
    try {
        MainWindow w;   // 创建主窗口

        // 添加文件加载逻辑
        if (!initialFile.isEmpty()) {
            if (!initialFile.isEmpty()) {
                w.onOpenFileTriggered(initialFile); // 直接传递文件名
            }
        }

        w.show();   // 显示主窗口
        return a.exec();    // 进入事件循环
    } catch (const std::exception& e) {
        qCritical() << "致命错误: " << e.what();  // 标准异常捕获
        return EXIT_FAILURE;
    } catch (...) {
        qCritical() << "未知错误发生";  // 未知异常处理
        return EXIT_FAILURE;
    }
}
