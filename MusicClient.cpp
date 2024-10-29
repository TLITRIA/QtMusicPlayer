#include "MusicClient.h"
#include "ui_MusicClient.h"
#include "MusicListPage.h"
#include "MusicPostPage.h"
#include "MusicTcpClient.h"
#include "InfoBar.h"
#include "ConfigXml.h"
#include "EditCollect.h"
#include "Login.h"
#include "AlbumShowPage.h"
#include "PlayController.h"
#include "SettingPage.h"

#include <QDomDocument>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QListWidgetItem>
#include <QPainter>
#include <QScrollArea>

#include <random>


struct MusicClientPrivate
{
    // widget 具体界面
    AlbumShowPage* albumPage;
    MusicListPage *playlistPage;
    MusicPostPage *postPage;
    SettingPage *settingPage;
    QScrollArea* area;
    PlayController *player;
    EditCollect* editCollect;
    Login* loginPage;

    // func
    MusicTcpClient tcpclient; // 用套接字是否连接判断是否登陆
    ConfigXml* xml; // 操作xml文件

    // data
    QMap<QString, MusicInfo> musicSource;                       // 音乐库资源 ID：具体信息
    QVector<QPair<CollectionInfo, QStringList>> collections;    // 专辑、播放列表汇总
    QStringList playlist;           // 播放列表 [当前行]：ID
    std::vector<int> randomlist;    // 播放列表的随机索引
    int switchShow; // 界面编号
    int playmode; // 音乐播放模式
    int style; // 界面样式编号
};

struct ClientPrivate // 用户信息
{
    QString ID;
    QString headIconFileName;
    QString UserName;
    bool ifLogin;
    QTimer LoginTimeout;
};


struct LocalPrivate
{
    QString styleFile;
    QString downloadPath;
    QStringList styles;
};

MusicClient::MusicClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicClient)
    , p(new MusicClientPrivate)
    , cli_mess(new ClientPrivate)
    , local_mess(new LocalPrivate)
{
    MYLOG;
    ui->setupUi(this);
/* 初始化 */
    p->albumPage = new AlbumShowPage();
    p->playlistPage = new MusicListPage();
    p->postPage = new MusicPostPage();
    p->settingPage = new SettingPage;
    p->loginPage = new Login();
    p->player = new PlayController;
    p->editCollect = new EditCollect();
    p->area = new QScrollArea;

    p->xml = &ConfigXml::getInstance();

    p->editCollect->setWindowTitle(QObject::tr("编辑收藏夹信息"));
    p->editCollect->setWindowModality(Qt::ApplicationModal); //设置阻塞类型
    p->editCollect->setAttribute(Qt::WA_ShowModal, true);    //属性设置 true:模态 false:非模态
    p->editCollect->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);

    p->loginPage->setWindowTitle(QObject::tr("登陆"));
    p->loginPage->setWindowModality(Qt::ApplicationModal); //设置阻塞类型
    p->loginPage->setAttribute(Qt::WA_ShowModal, true);    //属性设置 true:模态 false:非模态
    p->loginPage->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);

    // 主界面
    ui->horizontalSlider_volumn->setSliderPosition(0);
    ui->horizontalSlider_playtime->setSliderPosition(0);
    setLoginButtonIcon(":/Img/icon/default_User.png");

    // 登陆 定时器，一段时间后自动取消登陆行为
    cli_mess->ifLogin = false;
    cli_mess->LoginTimeout.setSingleShot(true); // 单次触发
    cli_mess->LoginTimeout.setInterval(5000);

    // 按钮
    QTransform tmpRot;
    tmpRot.rotate(180);
    ui->pushButton_show->setIcon(QIcon(QPixmap(":/Img/icon/show.png").transformed(tmpRot)));
    ui->pushButton_play_pause->setIcon(QIcon(":/Img/icon/play.png"));
    ui->pushButton_play_pause->setIconSize(ui->pushButton_play_pause->size());
    ui->pushButton_playmode->setIcon(QIcon(":/Img/icon/order_circle.png"));
    ui->pushButton_playmode->setIconSize(ui->pushButton_playmode->size());

    // 播放模式
    p->playmode = PMODE::circle;

    // 页面
    p->switchShow = SHOW_TYPE::MUSICLIST;
    ui->gridLayout_main->addWidget(p->playlistPage);
    p->playlistPage->show();

    // 设置界面
    p->area->setWidget(p->settingPage);
    p->area->setFixedWidth(p->settingPage->width()+20);


/*  初始化-信号 */
    // 播放控制
    connect(ui->horizontalSlider_playtime, &QSlider::sliderMoved, p->player, &PlayController::setPosition);    // 播放滑条
    connect(ui->horizontalSlider_volumn, &QSlider::valueChanged, p->player, &PlayController::setVolumn); // 音量滑条
    connect(p->player, &PlayController::updatePosition, this, &MusicClient::updatePosition); // 播放位置反馈
    connect(p->player, &PlayController::updateDuration, this, &MusicClient::updateDuration);  // 播放总时长反馈
    connect(p->player, &PlayController::updatePosition, p->postPage, &MusicPostPage::changeLCR); // 切换歌词
    connect(p->player, &PlayController::EndOfMedia, this, &MusicClient::player_next); // 播完音乐
    connect(p->player, &PlayController::playbackStateChanged, this, &MusicClient::setPlayButton);
    connect(ui->pushButton_play_pause, &QPushButton::clicked, this, &MusicClient::clickPlayButton);     // 播放/暂停
    connect(ui->pushButton_next, &QPushButton::clicked, this, &MusicClient::player_next);               // 当前列表下一首
    connect(ui->pushButton_prev, &QPushButton::clicked, this, &MusicClient::player_prev);               // 当前列表上一首
    connect(ui->pushButton_playmode, &QPushButton::clicked, this, &MusicClient::player_changemode);     // 切换播放模式

    // 专辑页
    connect(p->albumPage, &AlbumShowPage::COLLECT_ALBUM, &p->tcpclient, &MusicTcpClient::COLLECT_ALBUM); // 操作专辑收藏

    // 编辑自定义收藏夹
    connect(p->editCollect, &EditCollect::UPDATE_COLLECTIONS, &p->tcpclient, &MusicTcpClient::UPDATE_COLLECTIONS);

    // 播放列表
    connect(p->playlistPage, &MusicListPage::CollectionDoubleClicked, this, &MusicClient::updateMusicList);
    connect(p->playlistPage, &MusicListPage::MusicDoubleClicked, this, &MusicClient::setMusicPlay);
    connect(p->playlistPage, &MusicListPage::EDITCOLLECTION, this, &MusicClient::editCollection);
    connect(p->playlistPage, &MusicListPage::COLLECT, this, &MusicClient::setAlbumCollection);
    connect(p->playlistPage, &MusicListPage::DOWN, this, &MusicClient::downloadAllPlaylist);

    // 登陆
    connect(ui->pushButton_login, &QPushButton::clicked, [this]()
    {
        if(p->tcpclient.doCheckConnecting()) // 已连接服务器
        {
            p->tcpclient.DisConnect(); // 断开连接
            setLoginButtonIcon(":/Img/icon/default_User.png");
            DisplayLogoutMessageBox();
        }
        else
        {
            cli_mess->ifLogin = false;
            p->loginPage->show();
        }
    });
    connect(p->loginPage, &Login::doLogin, this, &MusicClient::doLogin);
    connect(p->loginPage, &Login::doRegister, this, &MusicClient::doRegister);

    // tcp
    connect(&p->tcpclient, &MusicTcpClient::LOGIN_SUCCESS, this, &MusicClient::LoginSuccess);
    connect(&p->tcpclient, &MusicTcpClient::LOGIN_FAIL, this, &MusicClient::LoginFail);
    connect(&p->tcpclient, &MusicTcpClient::DISCONNECT, this, &MusicClient::Disconnect);

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_SOURCEXML, [this](){
        LoadSourceXML();LoadCollectionXML(cli_mess->ID);});

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_HRADICON, [this](QCStrRef IconName){
        if(IconName == cli_mess->headIconFileName)
            setLoginButtonIcon(C::ClientSetting::getInstance().getCurDirPath() + C::Path[C::PathType::HeadIcon] + "/" + IconName);});

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_ICON, [this](QCStrRef icon){ MYLOG<<icon;
        cli_mess->headIconFileName = icon;
        setLoginButtonIcon(C::ClientSetting::getInstance().getCurDirPath() + C::Path[C::PathType::HeadIcon] + "/" + icon);
    });

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_NAME, [this](QCStrRef name){
        cli_mess->UserName = name;});

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_PRIVATE, this, &MusicClient::LoadCollectionXML);

    connect(&p->tcpclient, &MusicTcpClient::UPDATE_MUSICSURCE, [this](){
        LoadSourceXML(); LoadCollectionXML(cli_mess->ID);
        if(ifWaitDownLoadSource)
            setMusicPlay();
    });

    connect(&cli_mess->LoginTimeout, &QTimer::timeout, this, &MusicClient::LoginTimeout);

    // 切换显示
    connect(ui->pushButton_show, &QPushButton::clicked, this, &MusicClient::SwitchShow);
    connect(ui->pushButton_test1, &QPushButton::clicked, this, &MusicClient::SwitchAlbum);
    connect(ui->pushButton_test2, &QPushButton::clicked, this, &MusicClient::SwitchSetting);

    connect(p->settingPage, &SettingPage::QUIT, [this](){ui->pushButton_test2->click();});
    connect(p->settingPage, &SettingPage::setStyle, [this](QCStrRef style){local_mess->styleFile = style;});
    connect(p->settingPage, &SettingPage::UserChangeIcon, &p->tcpclient, &MusicTcpClient::UserChangeIcon);
    connect(p->settingPage, &SettingPage::UserChangeName, &p->tcpclient, &MusicTcpClient::UserChangeName);
    connect(p->settingPage, &SettingPage::transFile_filename, &p->tcpclient, &MusicTcpClient::transFile_filename);
/* 客户端启动后自动执行的任务 */
    LoadSourceXML(); // 载入音乐库资源
    LoadCollectionXML();
    // todo 向服务器请求样式文件、加载本地配置

    local_mess->downloadPath = "";
    local_mess->styleFile = "Ubuntu.qss";
    local_mess->styles = {"Aqua.qss","ConsoleStyle.qss",
        "MacOS.qss","ManjaroMix.qss","Ubuntu.qss"};

    ui->horizontalSlider_volumn->setSliderPosition(10);
    p->loginPage->doAutoLog();
}

MusicClient::~MusicClient()
{
    delete ui;
    if(p->loginPage != nullptr)
        delete p->loginPage;
    if(p->editCollect != nullptr)
        delete p->editCollect;
    delete p;
    delete cli_mess;
    delete local_mess;
}

bool MusicClient::LoadSourceXML() //  音乐库资源musicSource
{
    MYLOG<<"音乐库资源musicSource";
    p->playlistPage->setLogin(cli_mess->ifLogin);
    p->musicSource.clear();
    QString filePath = C::ClientSetting::getInstance().getCurDirPath()
                       + C::Path[C::PathType::Config]
                       + C::File[C::FileType::MusicSource];
    p->xml->LoadXMLPath(filePath);
    auto root = p->xml->ReadFromXML();
    QDomNodeList list = root.elementsByTagName("source");
    for (int i = 0; i < list.count(); i++) {
        // 获取链表中的值
        QDomElement element = list.at(i).toElement();
        QString m_key(element.attribute("ID"));
        MusicInfo m_val;
        m_val.id = element.attribute("ID");
        m_val.author = element.attribute("AUTHOR");
        m_val.name = element.attribute("NAME");
        m_val.audio = element.attribute("AUDIO");
        m_val.post = element.attribute("POST");
        m_val.lcr = element.attribute("LCR");
        m_val.music_type = element.attribute("TYPE").toInt();
        m_val.collect_times = element.attribute("COLLECT").toULongLong();
        // 判断本地资源是否完整
        QString tmpFileName;
        bool isChecked1 = true; // 是否符合本地文件：source没有数据或source有数据且本地有文件为true;
        bool isChecked2 = true;
        bool isChecked3 = true;
        tmpFileName = element.attribute("LCR");
        if(!tmpFileName.isEmpty())
            isChecked1 = C::CheckFile(C::ClientSetting::getInstance().getCurDirPath() + C::Path[C::PathType::Lcr] + "/" + tmpFileName);
        tmpFileName = element.attribute("POST");
        if(!tmpFileName.isEmpty())
            isChecked2 = C::CheckFile(C::ClientSetting::getInstance().getCurDirPath() + C::Path[C::PathType::Post] + "/" + tmpFileName);
        tmpFileName = element.attribute("AUDIO");
        if(!tmpFileName.isEmpty())
            isChecked3 = C::CheckFile(C::ClientSetting::getInstance().getCurDirPath() + C::Path[C::PathType::Audio] + "/" + tmpFileName);
        if(isChecked1 && isChecked2 && isChecked3)
            m_val.ifDown = true;
        p->musicSource[m_key] = m_val;
    }
    return true;
}

bool MusicClient::LoadCollectionXML(QCStrRef ID)
{
    MYLOG<<"LoadCollectionXML";
    p->collections.clear();
    // 1. 音乐库和本地资源
    QPair<CollectionInfo, QStringList> all_source; // 所有播放资源
    QPair<CollectionInfo, QStringList> local_source;
    all_source.first.foldname = "音乐库";
    all_source.first.info = "所有音乐资源";
    all_source.first.type = COLLEC_TYPE::all;

    local_source.first.foldname = "本地音乐";
    local_source.first.info = "本地所有音乐资源";
    local_source.first.type = C::COLLEC_TYPE::local;
    // 读取musicSource
    for(auto ite = p->musicSource.begin();
         ite!= p->musicSource.end(); ite++)
    {
        all_source.second.push_back(ite.key());
        if(ite.value().ifDown)
            local_source.second.push_back(ite.key());
    }
    p->collections.append(local_source);
    p->collections.append(all_source);
    // 至此音乐库、本地资源添加完成

    // 2.官方的播放列表
    QString filePath = C::ClientSetting::getInstance().getCurDirPath()
                       + C::Path[C::PathType::Config]
                       + C::File[C::FileType::MusicSource];
    p->xml->LoadXMLPath(filePath);
    auto root = p->xml->ReadFromXML();
    QDomNodeList nodeLists = root.elementsByTagName("pl");

    for(int i = 0; i < nodeLists.size(); i++)
    {
        QPair<CollectionInfo, QStringList> pl;
        pl.first.foldname = nodeLists.at(i).toElement().attribute("NAME");
        pl.first.coverFileName = nodeLists.at(i).toElement().attribute("COVER");
        pl.first.info = nodeLists.at(i).toElement().attribute("INFO");
        pl.first.type = COLLEC_TYPE::album;
        pl.first.id = nodeLists.at(i).toElement().attribute("ID").toULongLong();
        pl.first.collect = nodeLists.at(i).toElement().attribute("COLLECT").toULongLong();

        QDomNodeList idList = nodeLists.at(i).toElement().elementsByTagName("S_ID");
        for(int j = 0; j < idList.size(); j++)
            pl.second.append(idList.at(j).toElement().text());
        p->collections.append(pl);
    }

    // 如果登陆
    if(cli_mess->ifLogin && !ID.isEmpty())
    {
        QString filePath = C::ClientSetting::getInstance().getCurDirPath()
                           + C::Path[C::PathType::Account]
                           + "/" + ID + ".xml";
        p->xml->LoadXMLPath(filePath);
        root = p->xml->ReadFromXML();
        // 用户集合
        QPair<CollectionInfo, QStringList> favourite;
        favourite.first.foldname = "默认收藏夹";
        favourite.first.info = "喜欢的歌曲";
        favourite.first.type = COLLEC_TYPE::favourite;

        QDomNodeList favouriteList = root.elementsByTagName("source");
        for(int i = 0; i < favouriteList.size(); i++)
        {
            p->musicSource[favouriteList.at(i).toElement().text()].ifLike = true;
            favourite.second.append(favouriteList.at(i).toElement().text());
        }
        p->collections.append(favourite);

        //
        QDomNodeList playlist_like = root.elementsByTagName("pl");
        for(int i = 0; i < playlist_like.size(); i++)
            for(int j = 0; j < p->collections.size(); j++)
                if(p->collections[j].first.type == COLLEC_TYPE::album
                    && p->collections[j].first.id == playlist_like.at(i).toElement().text().toULongLong())
                    p->collections[j].first.ifCollect = true;

        // 自定义收藏夹
        QDomNodeList customList = root.elementsByTagName("cl");
        for(int i = 0; i < customList.size(); i++)
        {
            QPair<CollectionInfo, QStringList> cl;
            cl.first.foldname = customList.at(i).toElement().attribute("NAME");
            cl.first.coverFileName = customList.at(i).toElement().attribute("POST");
            cl.first.createTime = customList.at(i).toElement().attribute("CREATETIME").toULongLong();
            cl.first.info = customList.at(i).toElement().attribute("INFO");
            cl.first.type = COLLEC_TYPE::custom;
            cl.first.id = customList.at(i).toElement().attribute("ID").toULongLong();
            QDomNodeList S_ID = customList.at(i).toElement().elementsByTagName("S_ID");
            for(int j = 0; j < S_ID.size(); j++)
            {
                p->musicSource[S_ID.at(j).toElement().text()].ifCollect = true;
                cl.second.append(S_ID.at(j).toElement().text());
            }

            p->collections.append(cl);
        }
    }

    // 排序  显示顺序
    std::sort(p->collections.begin(), p->collections.end(),
        [](const QPair<CollectionInfo, QStringList>& a,
        const QPair<CollectionInfo, QStringList>& b)
    {
        int a_type = a.first.type;
        int b_type = b.first.type;
        return a_type < b_type;
    });

    updateCollectionList();
    return true;
}

void MusicClient::randomListGenerate()
{
    p->randomlist.clear();
    int count = p->playlistPage->count_music();
    for (int i = 0; i < count; i++)
        p->randomlist.push_back(i);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(p->randomlist.begin(), p->randomlist.end(), rng);
}

int MusicClient::DisplayLogoutMessageBox()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(""); // todo
    msgBox.setText("你已退出登陆");
    msgBox.exec();
    return 0;
}

QPixmap MusicClient::createCircularPixmap(const QPixmap &source)
{
    int size = qMin(source.width(), source.height());
    // 创建一个圆形裁剪的 QPixmap
    QPixmap roundedPixmap(size, size);
    roundedPixmap.fill(Qt::transparent);

    QPainter painter(&roundedPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制圆形裁剪
    painter.setBrush(QBrush(source.scaled(size, size, Qt::KeepAspectRatioByExpanding)));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);
    return roundedPixmap;
}

void MusicClient::setUIMode(int mode)
{
    QTransform reverse;
    reverse.rotate(180);
    ui->pushButton_show->setIcon(QIcon(QPixmap(":/Img/icon/show.png").transformed(reverse)));
    ui->pushButton_test1->setText("打开专辑界面");
    ui->pushButton_test2->setText("打开设置界面");
    switch (mode) {
    case SHOW_TYPE::ALBUM:
        ui->pushButton_test1->setText("返回");
        break;
    case SHOW_TYPE::MUSICLIST:
        break;
    case SHOW_TYPE::POST:
        ui->pushButton_show->setIcon(QIcon(QPixmap(":/Img/icon/show.png")));
        break;
    case SHOW_TYPE::SETTING:
        ui->pushButton_test2->setText("返回");
        break;
    default:
        break;
    }
}

void MusicClient::SwitchShow()
{
    QLayoutItem* child;
    while((child = ui->gridLayout_main->takeAt(0)) != nullptr)
    {
        child->widget()->hide();
        ui->gridLayout_main->removeItem(child);
    }
    if(p->switchShow != SHOW_TYPE::POST)
    {
        ui->gridLayout_main->addWidget(p->postPage);
        p->postPage->show();
        p->switchShow = SHOW_TYPE::POST;
    }
    else
    {
        ui->gridLayout_main->addWidget(p->playlistPage);
        p->playlistPage->show();
        p->switchShow = SHOW_TYPE::MUSICLIST;
    }
    setUIMode(p->switchShow);
}

void MusicClient::SwitchAlbum()
{
    QLayoutItem* child;
    while((child = ui->gridLayout_main->takeAt(0)) != nullptr)
    {
        child->widget()->hide();
        ui->gridLayout_main->removeItem(child); // 删除
    }
    if(p->switchShow !=SHOW_TYPE::ALBUM)
    {
        p->albumPage->INPUT(p->collections, cli_mess->ifLogin);
        ui->gridLayout_main->addWidget(p->albumPage);
        p->albumPage->show();
        p->switchShow = SHOW_TYPE::ALBUM;
    }
    else
    {
        ui->gridLayout_main->addWidget(p->playlistPage);
        p->playlistPage->show();
        p->switchShow = SHOW_TYPE::MUSICLIST;
    }
    setUIMode(p->switchShow);
}

void MusicClient::SwitchSetting()
{
    QLayoutItem* child;
    while((child = ui->gridLayout_main->takeAt(0)) != nullptr)
    {
        child->widget()->hide();
        ui->gridLayout_main->removeItem(child);
    }
    if(p->switchShow !=SHOW_TYPE::SETTING)
    {
        ui->gridLayout_main->addWidget(p->area);
        p->settingPage->clearALL();
        p->settingPage->inputLocalConfig(local_mess->downloadPath,
                                         local_mess->styleFile, local_mess->styles);
        p->settingPage->inputUser(cli_mess->ifLogin, cli_mess->ID,
                                  cli_mess->UserName, cli_mess->headIconFileName);
        p->area->show();
        p->switchShow = SHOW_TYPE::SETTING;
    }
    else
    {
        ui->gridLayout_main->addWidget(p->playlistPage);
        p->playlistPage->show();
        p->switchShow = SHOW_TYPE::MUSICLIST;
    }
    setUIMode(p->switchShow);
}

void MusicClient::updatePosition(quint64 position)
{
    ui->label_position->setText(QString("%1:%2")//4代表宽度，10表示10进制，空位补零
        .arg(position / 1000 / 60, 2, 10, QLatin1Char('0'))
        .arg(position / 1000 % 60, 2, 10, QLatin1Char('0')));
    ui->horizontalSlider_playtime->setValue(position);
}

void MusicClient::updateDuration(quint64 duration)
{
    ui->label_duration->setText(QString("%1:%2")//4代表宽度，10表示10进制，空位补零
        .arg(duration / 1000 / 60, 2, 10, QLatin1Char('0'))
        .arg(duration / 1000 % 60, 2, 10, QLatin1Char('0')));
    ui->horizontalSlider_playtime->setRange(0, duration);
}

void MusicClient::clickPlayButton()
{
    if(p->player->getState() == QMediaPlayer::PlaybackState::PlayingState)
    {
        Pause();
    }
    else
    {
        Play();
    }
}

void MusicClient::setPlayButton(QMediaPlayer::PlaybackState newState)
{
    if(newState ==  QMediaPlayer::PlayingState)
        ui->pushButton_play_pause->setIcon(QIcon(":/Img/icon/pause.png"));
    else
        ui->pushButton_play_pause->setIcon(QIcon(":/Img/icon/play.png"));
    ui->pushButton_play_pause->setIconSize(ui->pushButton_play_pause->size());
}

void MusicClient::player_next()
{
    int currentRow = p->playlistPage->currentRow_music();
    int nextRow = 0;
    int count  = p->playlistPage->count_music();

    if(count <= 0)
        return;
    if(p->playmode == PMODE::one)
    {
        nextRow = currentRow;
    }
    else if(p->playmode == PMODE::list)
    {
        nextRow = currentRow+1;
        if(nextRow == count)
        {
            Pause();
            return;
        }
    }
    else if(p->playmode == PMODE::circle)
    {
        nextRow = (currentRow + 1) % count;
    }
    else if(p->playmode == PMODE::random)
    {
        std::vector<int>::iterator it = std::find(p->randomlist.begin(),p->randomlist.end(), currentRow);
        int index=&*it-&p->randomlist[0];
        nextRow = p->randomlist[(index+1) % count];
    }
    p->playlistPage->setCurrentMusicRow(nextRow);
    setMusicPlay();
}

void MusicClient::player_prev()
{
    int currentRow = p->playlistPage->currentRow_music();
    int nextRow = 0;
    int count = p->playlistPage->count_music();

    if(count <= 0)
        return;

    if(p->playmode == PMODE::one)
    {
        nextRow = currentRow;
    }
    else if(p->playmode == PMODE::list)
    {
        nextRow = currentRow - 1;
        if(nextRow == -1)
        {
            Pause();
            return;
        }
    }
    else if(p->playmode == PMODE::circle)
    {
        nextRow = (currentRow-1+count) % count;
    }
    else if(p->playmode == PMODE::random)
    {
        std::vector<int>::iterator it = std::find(p->randomlist.begin(),p->randomlist.end(), currentRow);
        int index=&*it-&p->randomlist[0];
        nextRow = p->randomlist[(index-1+count) % count];
    }
    p->playlistPage->setCurrentMusicRow(nextRow);
    setMusicPlay();
}

void MusicClient::player_changemode()
{
    p->playmode = (p->playmode+1)%4;
    if (p->playmode == PMODE::list)
        ui->pushButton_playmode->setIcon(QIcon(":/Img/icon/order_list.png"));
    else if(p->playmode == PMODE::circle)
        ui->pushButton_playmode->setIcon(QIcon(":/Img/icon/order_circle.png"));
    else if(p->playmode == PMODE::one)
        ui->pushButton_playmode->setIcon(QIcon(":/Img/icon/order_one.png"));
    else if(p->playmode == PMODE::random)
    {
        randomListGenerate();
        ui->pushButton_playmode->setIcon(QIcon(":/Img/icon/order_random.png"));
    }
    ui->pushButton_playmode->setIconSize(ui->pushButton_playmode->size());
}

void MusicClient::Play()
{
    p->player->play();
    p->postPage->play();
}

void MusicClient::Pause()
{
    p->player->pause();
    p->postPage->pause();
}

void MusicClient::updateMusicList()
{
    int cur_c = p->playlistPage->currentRow_collection();
    int cur_m = p->playlistPage->currentRow_music();
    int cur_val_m = p->playlistPage->musicScrollBar_position();

    p->playlistPage->RemoveMusicList();
    p->playlist.clear();

    //  设置是否可收藏、是否已收藏
    if(p->collections[cur_c].first.type == COLLEC_TYPE::album)
    {
        p->playlistPage->setCollectable(true);
        p->playlistPage->setIfCollect(p->collections[cur_c].first.ifCollect);
    }
    else
    {
        p->playlistPage->setCollectable(false);
        p->playlistPage->setIfCollect(false);
    }

    QStringList::Iterator itor;
    for (itor = p->collections[cur_c].second.begin();
         itor != p->collections[cur_c].second.end(); ++itor)
    {
        p->playlist.append(*itor);
        InfoBar *newBar = new InfoBar(p->musicSource[*itor], cli_mess->ifLogin);
        connect(newBar, &InfoBar::DOWN, &p->tcpclient, &MusicTcpClient::MusicDown);
        connect(newBar, &InfoBar::COLLECT, this, &MusicClient::editCollection);
        connect(newBar, &InfoBar::LIKE, this, &MusicClient::Like);
        newBar->setFixedHeight(50);
        p->playlistPage->AddMusicList(newBar);
    }
    randomListGenerate();

    if(p->playlistPage->count_music()>0)
        p->playlistPage->setCurrentMusicRow(cur_m > 0 ? cur_m : 0);

    p->playlistPage->moveMusicScoll(cur_val_m > 0 ? cur_val_m : 0);
    setMusicPlay();
}

void MusicClient::setMusicPlay()
{
    if(!ifForceSetMusic)
    {
        ifForceSetMusic = true;
        return;
    }


    if(p->player->getState() == QMediaPlayer::PlaybackState::PlayingState)
        Pause();

    if(p->playlistPage->count_music()<=0)
        return;

    int cur_m = p->playlistPage->currentRow_music();
    QString ID = p->playlist[cur_m > 0 ? cur_m : 0];
    if(cli_mess->ifLogin && !p->musicSource[ID].ifDown)
    {
        p->tcpclient.MusicDown(ID);
        ifWaitDownLoadSource = true;
        return;
    }

    p->postPage->setMusicInfo(p->musicSource[ID].lcr, p->musicSource[ID].post);
    p->player->setMusicInfo(p->musicSource[ID].audio);
    Play();
}

void MusicClient::editCollection()
{
    if(cli_mess->ifLogin)
    {
        p->editCollect->inputMusic(p->collections, p->musicSource);
        p->editCollect->show();
    }
}

void MusicClient::setAlbumCollection(bool val)
{
    int cur = p->playlistPage->currentRow_collection();
    p->collections[cur].first.ifCollect = val;
    p->tcpclient.COLLECT_ALBUM(p->collections[cur].first.id, val);
}

void MusicClient::downloadAllPlaylist()
{
    int cur = p->playlistPage->currentRow_collection();
    QStringList downloadList  = p->collections[cur].second;
    for(int i = 0; i < downloadList.size(); i++)
        if(!p->musicSource[downloadList[i]].ifDown)
            p->tcpclient.MusicDown(downloadList[i]);
}

void MusicClient::updateCollectionList()
{
    MYLOG;
    int cur_c = p->playlistPage->currentRow_collection();
    p->playlistPage->RemoveCollectionList();

    for (auto itor = p->collections.begin();
         itor != p->collections.end(); itor++)
    {
        int type = itor->first.type;
        QString title;
        if(type == COLLEC_TYPE::all)
            title =  itor->first.foldname + "【全部资源】";
        else if (type == COLLEC_TYPE::local)
            title =  itor->first.foldname + "【本地资源】";
        else if (type == COLLEC_TYPE::album)
            title =  itor->first.foldname + "【专辑】";
        else if (type == COLLEC_TYPE::favourite)
            title =  itor->first.foldname + "【喜欢的乐曲】";
        else if (type == COLLEC_TYPE::custom)
            title =  itor->first.foldname + "【自定义收藏夹】";
        else if (type == COLLEC_TYPE::history)
            title =  itor->first.foldname + "【历史播放】";
        else
            title = itor->first.foldname;

        QListWidgetItem *tmpItem = new QListWidgetItem(title);
        p->playlistPage->AddCollectionList(tmpItem);
    }


    p->playlistPage->setCurrentCollectionRow(cur_c > 0 ? cur_c : 0);
    updateMusicList();

}

void MusicClient::setLoginButtonIcon(QCStrRef headIconFilePath)
{
    // 载入图像
    QPixmap originalPixmap;
    if(headIconFilePath.isEmpty())
        originalPixmap.load(":/Img/icon/default_User.png");
    else
        originalPixmap.load(headIconFilePath);

    // 裁剪图像为圆形
    QPixmap circularPixmap = createCircularPixmap(originalPixmap);

    // 设置按钮图标
    ui->pushButton_login->setIcon(QIcon(circularPixmap));
    ui->pushButton_login->setIconSize(QSize(100, 100)); // todo
}

void MusicClient::doLogin(QCStrRef ID, QCStrRef userName, QCStrRef pwd, bool ifAutolog, bool ifRempwd)
{
    ui->pushButton_login->setEnabled(false); // 确认登陆结果前不能打开login
    cli_mess->UserName = userName;
    cli_mess->LoginTimeout.start();
    p->tcpclient.Login(ID, userName, pwd, ifAutolog, ifRempwd); // 连接！并登陆！
}

void MusicClient::doRegister(QCStrRef userName, QCStrRef pwd, bool ifAutolog, bool ifRempwd)
{
    ui->pushButton_login->setEnabled(false); // 确认登陆结果前不能打开login
    cli_mess->UserName = userName;
    setLoginButtonIcon(":/Img/icon/default_User.png");
    cli_mess->LoginTimeout.start();
    p->tcpclient.Register(userName, pwd, ifAutolog, ifRempwd);
}

void MusicClient::LoginSuccess(QCStrRef message, QCStrRef ID)
{
    cli_mess->LoginTimeout.stop();
    if(cli_mess->ifLogin)
    {
        MYLOG<<"已登录却收到允许登陆";
        return;
    }

    // 登陆更新
    MYLOG<<message << cli_mess->UserName;
    cli_mess->ifLogin = true;
    cli_mess->ID = ID;
    p->xml->getAccountMessage(ID, &cli_mess->UserName, &cli_mess->headIconFileName);
    setLoginButtonIcon(C::ClientSetting::getInstance().getCurDirPath() +
                       C::Path[C::PathType::HeadIcon] + "/" + cli_mess->headIconFileName);

    ui->pushButton_login->setEnabled(true);
    setWindowTitle("登陆成功");

    LoadCollectionXML(cli_mess->ID);
}

void MusicClient::LoginFail(QCStrRef ans)
{
    MYLOG<<ans;
    cli_mess->LoginTimeout.stop();
    cli_mess->ifLogin = false;
    ui->pushButton_login->setEnabled(true);
    setWindowTitle("登陆失败");
    setLoginButtonIcon(":/Img/icon/default_User.png");
}

void MusicClient::Disconnect()
{
    setLoginButtonIcon(":/Img/icon/default_User.png");
    setWindowTitle("断开连接");

    cli_mess->ifLogin = false;
    cli_mess->ID = "";
    cli_mess->UserName = "";
    cli_mess->headIconFileName = "";

    LoadSourceXML();
    LoadCollectionXML(cli_mess->ID);
}

void MusicClient::LoginTimeout()
{
    ui->pushButton_login->setEnabled(true);
    setWindowTitle("登陆超时");
    setLoginButtonIcon(":/Img/icon/default_User.png");
}

void MusicClient::Like(QCStrRef m_id, bool value)
{
    ifForceSetMusic = false;
    p->xml->setAccountLike(m_id, cli_mess->ID, value);
    LoadSourceXML();
    LoadCollectionXML(cli_mess->ID);
    p->tcpclient.MusicLike(m_id, value);
}



