#ifndef SELECTCONTAIN_H
#define SELECTCONTAIN_H

#include <QWidget>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QSet>

#include "common.h"

using namespace C;

namespace Ui {
class SelectContain;
}

class SelectContain : public QWidget
{
    Q_OBJECT

public:
    explicit SelectContain(QWidget *parent = nullptr);
    ~SelectContain();

    void input(QMap<QString, MusicInfo> source, const QStringList& ids);

signals:
    void UPDATE_COLLECTION(const QStringList& ids); // 更新收藏信息

private slots:
    void ClickExit();
    void ClickSave();
    void ChangeCheckBox(int type);

private:
    void ClearMessage();

    Ui::SelectContain *ui;
    QMap<QString, MusicInfo> source;
};

#endif // SELECTCONTAIN_H
