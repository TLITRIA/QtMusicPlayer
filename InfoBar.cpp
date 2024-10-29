#include "InfoBar.h"
#include "ui_InfoBar.h"

struct InfoBarPrivate
{
    MusicInfo info;
};

InfoBar::InfoBar(const MusicInfo& info, bool ifLogin, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::InfoBar)
    , p(new InfoBarPrivate)
{
    ui->setupUi(this);
    p->info = info;

    QString tmpCurPath = ClientSetting::getInstance().getCurDirPath();

    ui->label_ID->setText(p->info.id);
    ui->label_duration->setText("");
    ui->label_singer->setText(p->info.author);
    ui->label_title->setText(p->info.name);

    connect(ui->pushButton_like, &QPushButton::clicked, this, &InfoBar::clickLike);
    connect(ui->pushButton_down, &QPushButton::clicked, this, &InfoBar::clickDown);
    connect(ui->pushButton_collect, &QPushButton::clicked, this, &InfoBar::clickCollect);

    setDown();
    setCollect();
    setLike();
    if(!ifLogin)
    {
        ui->pushButton_down->setEnabled(false);
        ui->pushButton_collect->setEnabled(false);
        ui->pushButton_like->setEnabled(false);
    }
}

InfoBar::~InfoBar()
{
    delete ui;
    delete p;
}

void InfoBar::setDown()
{
    if(p->info.ifDown)
        ui->pushButton_down->setIcon(QIcon(":/Img/icon/yes.png"));
    else
        ui->pushButton_down->setIcon(QIcon(":/Img/icon/download.png"));
    ui->pushButton_down->setIconSize(ui->pushButton_down->iconSize());

    ui->pushButton_down->setEnabled(!p->info.ifDown); // 已下载的不用下载
}

void InfoBar::setLike()
{
    if(p->info.ifLike)
        ui->pushButton_like->setIcon(QIcon(":/Img/icon/like_1.png"));
    else
        ui->pushButton_like->setIcon(QIcon(":/Img/icon/like_2.png"));
    ui->pushButton_like->setIconSize(ui->pushButton_like->iconSize());
}

void InfoBar::setCollect()
{
    if(p->info.ifCollect)
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_2.png"));
    else
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_1.png"));
    ui->pushButton_collect->setIconSize(ui->pushButton_collect->iconSize());
}

void InfoBar::clickDown()
{
    emit DOWN(p->info.id); // 下载处理
}

void InfoBar::clickLike()
{
    p->info.ifLike = !p->info.ifLike;
    setLike();
    emit LIKE(p->info.id, p->info.ifLike); // 发送信号
}

void InfoBar::clickCollect()
{
    emit COLLECT();
}
