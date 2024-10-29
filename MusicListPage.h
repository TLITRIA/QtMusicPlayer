#ifndef MUSICLISTPAGE_H
#define MUSICLISTPAGE_H
#include "InfoBar.h"

#include <QWidget>
#include <QListWidgetItem>
#include <QScrollBar>

namespace Ui {
class MusicListPage;
}

class MusicListPage : public QWidget
{
    Q_OBJECT

public:
    explicit MusicListPage(QWidget *parent = nullptr);
    ~MusicListPage();

    int currentRow_collection();
    int currentRow_music();
    int count_music();
    int musicScrollBar_position();

    // void ClearMusicList();
    void AddMusicList(InfoBar* newBar);
    void AddCollectionList(QListWidgetItem* newItem);

    void RemoveMusicList();
    void RemoveCollectionList();

    void setCurrentMusicRow(int row);
    void setCurrentCollectionRow(int row);

    void moveMusicScoll(int val);

    void setLogin(bool val);
    void setCollectable(bool val);
    void setIfCollect(bool val);

private:
    void setDown();  // 设置可下载
    void setCollect(); // 设置可收藏

private slots:
    void clickDown();
    void clickCollect();

signals:
    void CollectionDoubleClicked();
    void MusicDoubleClicked();
    void EDITCOLLECTION();

    void DOWN(); // 下载当前列表所有资源
    void COLLECT(bool val); // 将专辑标记为收藏/不收藏

private:
    Ui::MusicListPage *ui;

    bool ifCollect = false; // 对于专辑而言是否已收藏
    bool ifDown = false; // 对于所有成员而言是否全部已载
    bool ifLogin = false;

    // 刚才打开的位置信息，方便更新后移动到最近打开的位置
    int cur_c;      // 播放列表
    int cur_m;      // 列表中的位置
    int cur_val_m;  // 播放列表的位置
};

#endif // MUSICLISTPAGE_H
