#ifndef MUSICCLIENT_H
#define MUSICCLIENT_H

#include <QWidget>
#include <QMediaPlayer>



#include "common.h"


namespace Ui {
class MusicClient;
}

struct MusicClientPrivate;
struct ClientPrivate; // 用户信息
struct LocalPrivate; // 本地信息
class MusicClient : public QWidget
{
    Q_OBJECT

public:
    explicit MusicClient(QWidget *parent = nullptr);
    ~MusicClient();

    // Task
    void randomListGenerate(); // vector生成随机列表

    // MessageBox
    int DisplayLogoutMessageBox();

    // pixmap
    QPixmap createCircularPixmap(const QPixmap &source);

    // UI
    void setUIMode(int mode);

public slots:

    // PostWidget
    void SwitchShow();
    void SwitchAlbum();
    void SwitchSetting();

    void updatePosition(quint64 position);
    void updateDuration(quint64 duration);

    // 播放控制
    void clickPlayButton();     // 播放/暂停
    void setPlayButton(QMediaPlayer::PlaybackState newState); // 设置按钮图标
    void player_next();         // 当前列表下一首
    void player_prev();         // 当前列表上一首
    void player_changemode();   // 切换播放模式
    void Play();
    void Pause();

    // 播放列表
    void updateMusicList();  // 更新播放列表
    void setMusicPlay(); // 设置音乐信息
    void editCollection(); // 编辑收藏信息
    void setAlbumCollection(bool val); // 标记专辑
    void downloadAllPlaylist(); // 下载当前所有列表

    // Task
    bool LoadSourceXML(); // 载入公共资源信息
    bool LoadCollectionXML(QCStrRef ID = ""); // 载入收藏信息， 用户私有信息
    void updateCollectionList(); // 更新收藏夹
    void setLoginButtonIcon(QCStrRef headIconFilePath); // 设置头像

    // MusicTcpClient
    void doLogin(QCStrRef ID, QCStrRef userName, QCStrRef pwd,
                 bool ifAutolog, bool ifRempwd);
    void doRegister(QCStrRef userName, QCStrRef pwd,
                    bool ifAutolog, bool ifRempwd);
    void LoginSuccess(QCStrRef message, QCStrRef ID);
    void LoginFail(QCStrRef ans);
    void Disconnect();
    void LoginTimeout();

    // 对infobar操作
    // 下载资源
    void Like(QCStrRef m_id, bool value); // 将歌曲标记为喜欢/不喜欢
    // collect复用 editcollect

private:
    Ui::MusicClient *ui;
    MusicClientPrivate *p;
    ClientPrivate *cli_mess;
    LocalPrivate *local_mess;

    bool ifWaitDownLoadSource = false; // 标志是否要等待下载资源
    bool ifForceSetMusic = true; // 有些行为需要强制修改音频，有些不需要
};

#endif // MUSICCLIENT_H
