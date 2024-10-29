#ifndef ALBUMSHOWPAGE_H
#define ALBUMSHOWPAGE_H

#include "common.h"

#include <QQueue>
#include <QWidget>
#include <QGridLayout>
#include <QResizeEvent>

namespace Ui {
class AlbumShowPage;
}

using namespace C;

class AlbumShowPage : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumShowPage(QWidget *parent = nullptr);
    ~AlbumShowPage();
    void resizeEvent(QResizeEvent *event) override;

    void setLogin(bool val);
    void INPUT(const QVector<QPair<CollectionInfo, QStringList>>& album_input, bool if_login); // 专辑
    void CLEAN();

private:
    void Rerange(int col = 3); // 重新排列到指定列数

    int calculateGridLayoutWidth(QGridLayout *layout);

signals:
    void COLLECT_ALBUM(const quint64& a_id, bool val);


private:
    Ui::AlbumShowPage *ui;
    QGridLayout* content;
    bool ifLogin = false;
    int ColNum; // 行数
    bool oneAction = true;
};

#endif // ALBUMSHOWPAGE_H
