#include "MusicClient.h"


#include "common.h"
#include "ConfigXml.h"
#include <QFile>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
/* 检查 */
    C::CheckPath();

    if(!C::CheckLogin())
        ConfigXml::getInstance().createNullLoginFile();
    if(!C::CheckLocalConfig())
        ConfigXml::getInstance().createNullLocalConifg();

    a.setWindowIcon(QIcon(":/Img/icon/doge.png"));
/* 主窗口 */
    MusicClient w;
    w.show();

/* 样式 */
    // 比较好看的/Aqua/ConsoleStyle/MacOS/ManjaroMix/Ubuntu/untitled1
    QFile qssfile(":/Style/Ubuntu.qss");
    if(qssfile.open(QIODevice::ReadOnly) == true)
    {
        MYLOG<<"成功加载样式表";
        qApp->setStyleSheet(qssfile.readAll());
        qssfile.close();
    }



    return a.exec();
}
