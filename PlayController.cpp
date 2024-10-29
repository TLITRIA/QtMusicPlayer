#include "PlayController.h"
#include <QMediaPlayer>
#include <QAudioOutput>

#include <QTimer>
struct PlayControllerPrivate
{
    QMediaPlayer *player;
    QAudioOutput *audioOut;
    QTimer LCRTimer;
};

PlayController::PlayController()
    :p (new PlayControllerPrivate)
{
    p->audioOut = new QAudioOutput;
    p->player = new QMediaPlayer;
    p->player->setAudioOutput(p->audioOut);

    connect(p->player, &QMediaPlayer::positionChanged, this, &PlayController::updatePosition);
    connect(p->player, &QMediaPlayer::durationChanged, this, &PlayController::updateDuration);
    connect(p->player, &QMediaPlayer::mediaStatusChanged, [this](QMediaPlayer::MediaStatus status){
        if(status == QMediaPlayer::EndOfMedia) { emit EndOfMedia(); }  });
    connect(p->player, &QMediaPlayer::playbackStateChanged, this, &PlayController::playbackStateChanged);
    connect(p->player, &QMediaPlayer::mediaStatusChanged, this, &PlayController::mediaStatusChanged);
    connect(&p->LCRTimer, &QTimer::timeout, [this](){emit changeLCR(p->player->position()); });
}

void PlayController::setMusicInfo(QCStrRef audio)
{
    p->player->setSource(C::ClientSetting::getInstance().getCurDirPath()
                         + C::Path[C::PathType::Audio] + "/" + audio);

}

void PlayController::showState()
{
    MYLOG<<p->player->playbackState()<<p->player->mediaStatus();
}

int PlayController::getState()
{
    return p->player->playbackState();
}

void PlayController::play()
{
    p->player->play();
    p->LCRTimer.start(500);
}

void PlayController::pause()
{
    p->player->pause();
    p->LCRTimer.stop();
}

void PlayController::setPosition(qint64 position)
{
    p->player->setPosition(position);
}

void PlayController::setVolumn(int newVolume)
{
    p->audioOut->setVolume((float)newVolume / 100);
}
