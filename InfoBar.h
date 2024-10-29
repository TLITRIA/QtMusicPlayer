#ifndef INFOBAR_H
#define INFOBAR_H

#include <QWidget>
#include "common.h"

using namespace C;

namespace Ui {
class InfoBar;
}
struct InfoBarPrivate;
class InfoBar : public QWidget
{
    Q_OBJECT

public:
    explicit InfoBar(const MusicInfo& info,
                     bool ifLogin = true,
                     QWidget *parent = nullptr);
    ~InfoBar();

    void setDown();
    void setLike();
    void setCollect();

public slots:
    void clickDown();
    void clickLike();
    void clickCollect();
signals:
    void DOWN(QCStrRef m_id); // 下载资源
    void LIKE(QCStrRef m_id, bool value); // 将歌曲标记为喜欢/不喜欢
    void COLLECT(); // 复用批量编辑收藏夹的功能
private:
    Ui::InfoBar *ui;
    InfoBarPrivate *p;
};

#endif // INFOBAR_H
