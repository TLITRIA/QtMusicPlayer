#ifndef PLAYCONTROLLER_H
#define PLAYCONTROLLER_H

#include <QMediaPlayer>
#include <QObject>
#include "common.h"
using namespace C;

struct PlayControllerPrivate;



class PlayController : public QObject
{
    Q_OBJECT
public:
    PlayController();

    void setMusicInfo(QCStrRef audio); // 设置本地
    void showState();
    int getState();
    void setPosition(qint64 position);
    void setVolumn(int newVolume);

public slots:
    void play();
    void pause();

signals:
    void updateDuration(quint64 duration);
    void updatePosition(quint64 position);
    void EndOfMedia();
    void changeLCR(qint64 position);
    void playbackStateChanged(QMediaPlayer::PlaybackState newState);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);


private:
    PlayControllerPrivate* p;
};

#endif // PLAYCONTROLLER_H
