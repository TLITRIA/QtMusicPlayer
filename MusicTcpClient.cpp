#include "MusicTcpClient.h"

#include <QTextCodec>
#include <QThread>
#include <QFile>
#include <QFileInfo>

struct RecvPrivate
{
    QFile recvFile;
    qint64 fileSize;
    qint64 recvSize;
    // QTextCodec *codec;
};


MusicTcpClient::MusicTcpClient(QObject *parent)
    : QObject{parent}
    , trans(new RecvPrivate)
    , xml (&ConfigXml::getInstance())
{
    xml->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                           + C::Path[C::PathType::Config]
                           + C::File[C::FileType::LoginAccount]); // 写入文件路径
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QTcpSocket::connected, this, &MusicTcpClient::connected);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MusicTcpClient::disconnected);
    connect(clientSocket, &QTcpSocket::readyRead, this, &MusicTcpClient::readyread);
}

MusicTcpClient::~MusicTcpClient()
{
    delete trans;
    delete clientSocket;
}

void MusicTcpClient::readByteArray(const QByteArray &recv)
{
    static QByteArray frame_head_compare = QByteArray(frameHead,4);
    recvByteArray+=recv;
    while(recvByteArray.length()>0)
    {
        while(!recvByteArray.startsWith(frame_head_compare) && recvByteArray.size() > 4)
            recvByteArray.remove(0,1);
        if(recvByteArray.size() < 7+2)
            return;
        const int content_length = uchar(recvByteArray[4])*0x100 + uchar(recvByteArray[5]);
        if(recvByteArray.size() < 7+2 + content_length)
            return;
        if(memcmp(recvByteArray.constData()+7+content_length,frameTail,2) != 0)
        {
            recvByteArray.clear();
            MYLOG;
            return;
        }
        // MYLOG<<"recvByteArray:\n"<<recvByteArray.mid(7, content_length)<<QString("代码是%1").arg((char)recvByteArray[6]);
        switch((char)recvByteArray[6])
        {
        case CODE::TEST:
        {
            MYLOG<<recvByteArray.sliced(7, content_length);
            break;
        }
        case CODE::UP_HEAD:
        {
            sendData(CODE::END, "");
            break;
        }
        case CODE::UP_BODY:
        {
            sendData(CODE::END, "");
            break;
        }
        case CODE::DOWN_HEAD:
        {
            QString fileName;
            int type;
            UnpackFileMsg(QJsonDocument::fromJson(recvByteArray.mid(7, content_length)).object(),
                          &fileName, &type, &trans->fileSize);


            trans->recvFile.setFileName(C::ClientSetting::getInstance().getCurDirPath()
                +C::Path[type] + "/" + fileName);
            if(trans->recvFile.exists())
                trans->recvFile.remove();
            if(!trans->recvFile.open(QIODevice::NewOnly | QIODevice::WriteOnly))
                sendData(CODE::DOWN_HEAD, QByteArray("文件创建出错"));
            trans->recvSize = 0;
            trans->recvFile.resize(trans->fileSize);
            // MYLOG<<"准备好传输："<<trans->recvFile.fileName();
            break;
        }
        case CODE::DOWN_BODY:
        {
            trans->recvFile.seek(trans->recvSize);
            int realReadSize = trans->recvFile.write(recvByteArray.sliced(7+3, content_length-3));
            if(realReadSize != content_length-3)
                sendData(CODE::DOWN_BODY, QByteArray("数据写入出错"));
            trans->recvSize+=realReadSize;
            if(trans->recvSize == trans->fileSize)
                sendData(CODE::DOWN_BODY, QByteArray("传输完成"));
            break;
        }
        case CODE::END:
        {
            trans->recvFile.close();
            trans->fileSize = 0;
            // trans->recvFile.open(QIODevice::ReadOnly);
            // MYLOG<<"英文中文歌词解析情况不同"<<GetCorrectUnicode(trans->recvFile.readAll());
            // trans->recvFile.close();

            int resType;
            int pathType;
            QString message;
            if(!UnpackTransEnd(QJsonDocument::fromJson(recvByteArray.sliced(7, content_length)).object()
                                ,&resType ,&pathType ,&message))
                break;

            if(resType!=TRANS::RES::SUCCESS)
            {
                MYLOG<<"传输失败"<<message;
                break;
            }
            QFileInfo info = QFileInfo(trans->recvFile);
            // MYLOG<<"文件类型:"<<pathType;
            switch(pathType)
            {
            case C::PathType::Account:
            {
                emit UPDATE_PRIVATE(info.baseName());
                break;
            }
            case C::PathType::Config:
            {
                if(info.fileName() == C::File[C::FileType::MusicSource].split("/")[1]) // source.xml
                    emit UPDATE_SOURCEXML();
                break;
            }
            case C::PathType::HeadIcon:
            {
                emit UPDATE_HRADICON(info.fileName());
                break;
            }
            case C::PathType::Post:
            {
                break;
            }
            case C::PathType::Lcr:
            {
                break;
            }
            case C::PathType::Audio:
            {
                break;
            }
            default:
            {
                MYLOG<<"未添加的指令，下载的文件路径类型"<<pathType;
                break;
            }
            }
            break;
        }
        case CODE::LOGIN:
        {MYLOG<<"LOGIN";
            QString ans;
            QString ID;
            QString username;
            QString HeadIconName;
            bool ifRempwd;
            bool ifAutolog;

            if(!UnpackUserMsg(QJsonDocument::fromJson(recvByteArray.sliced(7, content_length)).object()
                            ,&ans ,&ID ,&username,&HeadIconName, &ifRempwd, &ifAutolog))
                break;

            if(ans == "success")
            {
                int errorcode = 0;
                xml->updateAccount(ID, username, HeadIconName, ifAutolog, ifRempwd, &errorcode);
                emit LOGIN_SUCCESS(QString("成功登陆 [%1:%2]")
                                       .arg(clientSocket->peerAddress().toString())
                                       .arg(clientSocket->peerPort()), ID);
            }
            else
                emit LOGIN_FAIL(ans);

            break;
        }
        case CODE::REGISTER:
        {
            // 解析
            QString ans;
            QString ID;
            QString username;
            QString headfileName;
            bool ifRempwd;
            bool ifAutolog;
            if(!UnpackUserMsg(QJsonDocument::fromJson(recvByteArray.sliced(7, content_length)).object()
                               ,&ans ,&ID ,&username, &headfileName, &ifRempwd, &ifAutolog))
                break;

            int errorcode = 0;

            if(ans == "success")
                xml->updateAccount(ID, username, "", ifRempwd, false, &errorcode);
            MYLOG<<QString("%1").arg(errorcode);
        }
        case CODE::UPDATE:
        {

            QJsonDocument jsonDoc(QJsonDocument::fromJson(recvByteArray.sliced(7, content_length)).object());
            // MYLOG<<"命令:"<<jsonDoc["type"].toInt();
            switch(jsonDoc["type"].toInt())
            {
            case S::UPDATE::MusiScource:
            {
                MYLOG<<"音乐资源下载完成，更新";
                emit UPDATE_MUSICSURCE();
                break;
            }
            case S::UPDATE::_UserName:
            {
                emit UPDATE_NAME(jsonDoc["name"].toString());
                break;
            }
            case S::UPDATE::_UserIcon:
            {
                emit UPDATE_ICON(jsonDoc["icon"].toString());
                break;
            }
            default :
            {
                break;
            }
            }

            break;
        }
        default:
        {
            MYLOG<<"未知添加的命令："<<recvByteArray[6];break;
        }
        }
        recvByteArray.remove(0, 7+2+content_length);
    }
}

void MusicTcpClient::sendData(char type, const QByteArray &content)
{
    frameHead[6] = type;
    const quint64 data_size=content.count();
    frameHead[5]=data_size%0x100;
    frameHead[4]=data_size/0x100;
    clientSocket->write(frameHead, 7);
    clientSocket->write(content);
    clientSocket->write(frameTail, 2);
}

qint64 MusicTcpClient::sendPage(qint64 page, const QByteArray &content)
{
    frameHead[6] = CODE::UP_BODY;
    const quint64 data_size=content.count() + 3;
    frameHead[5]=data_size%0x100;
    frameHead[4]=data_size/0x100;

    framePage[0]=page/0x10000%0x100;
    framePage[1]=page/0x100%0x100;
    framePage[2]=page%0x100;
    clientSocket->write(frameHead, 7);
    clientSocket->write(framePage, 3);
    clientSocket->write(content);
    clientSocket->write(frameTail, 2);
    return content.count();
}

bool MusicTcpClient::doCheckConnecting()
{
    return clientSocket->state() == QAbstractSocket::ConnectedState;
}

void MusicTcpClient::transFile_path(QCStrRef filePath, int type)
{
    // MYLOG<<"传输的文件位于"<<filePath;
    QFileInfo fileInfo(filePath);
    if(!fileInfo.exists())
    {
        MYLOG<<filePath<<"\t!fileInfo.exists()";
        return;
    }

    QFile file(fileInfo.filePath());
    if(!file.open(QIODevice::ReadOnly))
        return;
    // MYLOG<<fileInfo.fileName();
    sendData(CODE::UP_HEAD, fileMsg(fileInfo.fileName(), type, fileInfo.size()));
    qint64 pages = 0;
    while(!file.atEnd())
        sendPage(pages++, file.read(1024));

    file.close();
    sendData(CODE::END, transEnd(TRANS::RES::SUCCESS, type, "传输成功"));
}

void MusicTcpClient::transFile_filename(QCStrRef fileName, int type)
{
    QString filePath = C::ClientSetting::getInstance().getCurDirPath()
                       + C::Path[type] + "/" + fileName;
    transFile_path(filePath, type);
}


void MusicTcpClient::Login(QCStrRef ID, QCStrRef userName, QCStrRef pwd, bool ifAuto, bool ifRempwd)
{
    if(Connect("127.0.0.1", 8081) == QTcpSocket::SocketState::ConnectedState)
        sendData(CODE::LOGIN, LoginMsg(ID, userName, pwd, ifRempwd, ifAuto));
}

void MusicTcpClient::Register(QCStrRef userName, QCStrRef pwd, bool ifAuto, bool ifRempwd)
{
    if(Connect("127.0.0.1", 8081) == QTcpSocket::SocketState::ConnectedState)
        sendData(CODE::REGISTER, LoginMsg("", userName, pwd, ifRempwd, ifAuto));
}


void MusicTcpClient::MusicLike(QCStrRef m_id, bool value)
{
    if(doCheckConnecting())
        sendData(CODE::REQUEST, UploadLike(m_id, value));
}

void MusicTcpClient::MusicDown(QCStrRef m_id)
{
    sendData(CODE::REQUEST, DownloadSingleMusic(m_id));
}

void MusicTcpClient::UserChangeName(QCStrRef u_id, QCStrRef name)
{
    sendData(CODE::REQUEST, ChangeName(u_id, name));
}

void MusicTcpClient::UserChangeIcon(QCStrRef u_id, QCStrRef icon)
{
    sendData(CODE::REQUEST, ChangeIcon(u_id, icon));
}

int MusicTcpClient::Connect(QCStrRef server_address, quint16 server_port)
{
    clientSocket->connectToHost(QHostAddress(server_address), server_port);
    clientSocket->waitForConnected();
    return clientSocket->state();
}

int MusicTcpClient::DisConnect()
{
    clientSocket->disconnectFromHost();
    return clientSocket->state();
}

void MusicTcpClient::disconnected()
{
    MYLOG<< QString("断开 [%1:%2]")
        .arg(clientSocket->peerAddress().toString())
        .arg(clientSocket->peerPort());
    emit DISCONNECT();
    // MYLOG<<clientSocket->state();
}

void MusicTcpClient::connected()
{
    MYLOG<<"已连接服务器";
}

void MusicTcpClient::readyread()
{
    if(clientSocket->bytesAvailable()<=0)
        return;
    while(clientSocket->bytesAvailable())
        readByteArray(clientSocket->readAll());
}

void MusicTcpClient::UPDATE_COLLECTIONS(QVector<QPair<CollectionInfo, QStringList>> toUpdate, QSet<quint64> toDelete)
{
    if(!doCheckConnecting())
        return;

    // 更新
    for(int i = 0; i<toUpdate.size(); i++)
    {
        QJsonObject obj;
        obj["type"] = REQUEST::UpdateWholeCollection;        // 上传收藏夹信息
        obj["name"] = toUpdate[i].first.foldname;
        obj["post"] = toUpdate[i].first.coverFileName;
        obj["creatTime"] = QString::number(toUpdate[i].first.createTime);
        obj["info"] = toUpdate[i].first.info;
        obj["c_type"] = QString::number(toUpdate[i].first.type);
        obj["id"] = QString::number(toUpdate[i].first.id);
        QJsonArray contain;
        for(auto& ite:toUpdate[i].second)
            contain.push_back(ite);
        obj["containArray"] = contain;

        sendData(CODE::REQUEST, QJsonDocument(obj).toJson());

        // 上传图片
        MYLOG<<"更新："<<toUpdate[i].first.foldname;
        transFile_filename(toUpdate[i].first.coverFileName, PathType::Post);
    }

    // MYLOG<<toDelete.size();
    for (auto ite = toDelete.begin();
         ite != toDelete.end(); ite++)
    {
        QJsonObject obj;
        obj["type"] = REQUEST::DeleteCollection; // 删除收藏夹
        obj["c_id"] = QString::number(*ite);
        sendData(CODE::REQUEST, QJsonDocument(obj).toJson());
        // MYLOG<<"删除"<<*ite;
    }

    // 传输结束,发送信号获取更新后的数据
    QJsonObject obj;
    obj["type"] = REQUEST::GetUpdateUserData;  // 发送信号表示结束
    sendData(CODE::REQUEST, QJsonDocument(obj).toJson());

    // QJsonDocument(obj).toJson() === QBytearray()
}

void MusicTcpClient::COLLECT_ALBUM(const quint64& a_id, bool val)
{
    if(!doCheckConnecting())
        return;
    QJsonObject obj;
    obj["type"] = REQUEST::UpdateAlbum;
    obj["a_id"] = QString::number(a_id);
    obj["val"] = val;
    sendData(CODE::REQUEST, QJsonDocument(obj).toJson());
}

QString GetCorrectUnicode(const QByteArray &ba)
{
    QTextCodec::ConverterState state;
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QString text = codec->toUnicode( ba.constData(), ba.size(), &state);
    if (state.invalidChars > 0)
    {
        text = QTextCodec::codecForName( "GBK" )->toUnicode(ba);
        return QString("GBK");
    }
    else
    {
        text = ba;
        return QString("UTF8");
    }
}
