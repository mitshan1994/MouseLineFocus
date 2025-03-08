#include "MainWindow.h"
#include "mylog/mylog.h"
#include "AnchorSettings.h"
#include "HotkeyHook/KeyboardHook.h"

#include <QAction>
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Init log.
    InitLog("./log/AnchorLines.log");
    L_INFO("------------------ Start ------------------");
    L_INFO("Working directory: {}", QDir::currentPath());

    // Initialize setting instance.
    AnchorSettings settings;

    a.setQuitOnLastWindowClosed(false);

//    QWidget widget;
//    MainWindow w(&widget);

    MainWindow w;

    int ret = a.exec();

    KeyboardHook::getInstance().endThread();

    return ret;
}
