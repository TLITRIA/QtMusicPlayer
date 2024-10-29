#ifndef MUSICTCPCLIENT_H
#define MUSICTCPCLIENT_H
#include "common.h"
#include "ConfigXml.h"
#include <QObject>
#include <QTimer>
#include <QTcpSocket>

using namespace C;

struct RecvPrivate;

class MusicTcpClient : public QObject
{
    Q_OBJECT
public:
    explicit MusicTcpClient(QObject *parent = nullptr);
    ~MusicTcpClient();

    bool doCheckConnecting(); // 是否已连接
    int Connect(const QString &server_address, quint16 server_port);
    int DisConnect();

    void readByteArray(const QByteArray& recv); // 接收
    void sendData(char type, const QByteArray &content = QByteArray()); // 发送
    qint64 sendPage(qint64 page, const QByteArray &content);
    void transFile_path(QCStrRef filePath, int type); // 传出文件
    void transFile_filename(QCStrRef fileName, int type);
    /*Task*/
    //
    void Login(QCStrRef ID, QCStrRef userName, QCStrRef pwd, bool ifAuto, bool ifRempwd); // 登陆
    void Register(QCStrRef userName, QCStrRef pwd, bool ifAuto, bool ifRempwd); // 注册
    // MusicSource
    void MusicLike(QCStrRef m_id, bool value); // 喜欢
    void MusicDown(QCStrRef m_id); // 下载
    // user change 用户修改信息
    void UserChangeName(QCStrRef u_id, QCStrRef name);
    void UserChangeIcon(QCStrRef u_id, QCStrRef icon);

signals:
    void LOGIN_SUCCESS(QCStrRef message, QCStrRef ID); //
    void LOGIN_FAIL(QCStrRef ans); //
    void DISCONNECT();

    // XX要更新
    void UPDATE_SOURCEXML(); // source.xml
    void UPDATE_PRIVATE(QCStrRef ID); // ID.xml
    void UPDATE_HRADICON(QCStrRef IconName); // 头像
    void UPDATE_MUSICSURCE(); // 音乐资源文件
    void UPDATE_ICON(QCStrRef icon);
    void UPDATE_NAME(QCStrRef name);

public slots:
    void disconnected();
    void connected();
    void readyread();
    void UPDATE_COLLECTIONS(QVector<QPair<CollectionInfo, QStringList>> toUpdate,
                            QSet<quint64> toDelete); // 上传修改的文件夹信息
    void COLLECT_ALBUM(const quint64& a_id, bool val);
private:
    QTcpSocket* clientSocket;
    QByteArray recvByteArray;
    ConfigXml* xml;
    RecvPrivate *trans;
    char frameHead[7] = {0x0f, (char)0xF0, 0x00, (char)0xFF, 0x00, 0x00, 0x00};
    char frameTail[2] = {0x0D, 0x0A}; // /r/n
    char framePage[3] = {0x00, 0x00, 0x00};
};

QString GetCorrectUnicode(const QByteArray& ba);
#endif // MUSICTCPCLIENT_H
