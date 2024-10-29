#ifndef MUSICPOSTPAGE_H
#define MUSICPOSTPAGE_H

#include <QWidget>

#include "common.h"
using namespace C;
namespace Ui {
class MusicPostPage;
}
struct MusicPostPrivate;

class MusicPostPage : public QWidget
{
    Q_OBJECT

public:
    explicit MusicPostPage(QWidget *parent = nullptr);
    ~MusicPostPage();

    void setLCR(QCStrRef path);
    void setNoneLCR(); // 无歌词要生成暂无歌词的字样

    void play();
    void pause();

public slots:
    void setMusicInfo(QCStrRef lcr, QCStrRef post);
    void changeLCR(qint64 position); // 切换歌词

private:
    long perserLRCTime(QString& timeStr);// 从[xx:xx.xx]提取时间
    QString GetCorrectUnicode(const QByteArray& ba);

private:
    Ui::MusicPostPage *ui;
    MusicPostPrivate *p;
};

#endif // MUSICPOSTPAGE_H
