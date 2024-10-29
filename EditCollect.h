#ifndef EDITCOLLECT_H
#define EDITCOLLECT_H

#include <QWidget>
#include <QFileDialog>
#include <QTableWidget>
#include <QApplication>

#include "SelectContain.h"



#include "common.h"
using namespace C;

namespace Ui {
class EditCollect;
}

enum STATUS  // 需要手动确认
{
    UNCHANGED = 0, // 未修改的
    UNSAVED, // 未保存，需要上传
    SAVED, // 已保存的
};



class EditCollect : public QWidget
{
    Q_OBJECT

public:
    explicit EditCollect(QWidget *parent = nullptr);
    ~EditCollect();

    void inputMusic(const QVector<QPair<CollectionInfo, QStringList>>& collection_input,
                    const QMap<QString, MusicInfo>& source_input);

signals:
    void UPDATE_COLLECTIONS(QVector<QPair<CollectionInfo, QStringList>> toUpdate,
                            QSet<quint64> toDelete); // 返回需要更新的收藏夹和信息，返回需要删除的收藏夹

private:
    void clearALL();
    void updateCollectList();

private slots:
    void SetCurUnsaved(); // 将当前收藏夹设置为未保存状态

    void ClickCollection(); // 点击收藏列表
    void button_add(); // 添加一项
    void button_del(); // 删除不会消除，只是将选项打上横线、颜色变灰，在退出时才操作
    void Click_Undo(); // 撤销所有操作
    void ClickSave(); // 保存并退出
    void ClickExit(); // 直接退出

    void fillFullPostPath(); // 打开dialog，输入海报地址并复制文件
    void fillFullContain(int curRow); // 补全收藏夹包含的音频信息
    void ClickSaveTmp(); // 点击暂存
    void ContainSaveTmp(const QStringList& ids); // 编辑收藏夹的音频编辑暂存
    void ClickEditContain(); // 点击编辑收藏内容
private:
    Ui::EditCollect *ui;
    QVector<QPair<CollectionInfo, QStringList>> back; // 备份
    QVector<QPair<QPair<CollectionInfo, QStringList>,int>> edit; // 修改后的信息:status
    QMap<QString, MusicInfo> source;
    SelectContain* selectPage;

    bool isUserAction = true; // 是否是用户行为
};

#endif // EDITCOLLECT_H
