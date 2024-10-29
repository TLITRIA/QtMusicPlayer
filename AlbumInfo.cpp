#include "AlbumInfo.h"
#include "ui_AlbumInfo.h"

AlbumInfo::AlbumInfo(const CollectionInfo &input, bool ifLogin, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AlbumInfo)
{
    ui->setupUi(this);

    this->ifLogin = ifLogin;
    info = input;

    ui->label_name->setText(info.foldname);
    ui->label_name->setMaximumWidth(65);
    ui->label_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);  // 左对齐
    ui->label_name->setToolTip(ui->label_name->text());  // 鼠标悬停时提示全部文字


    connect(ui->pushButton_collect, &QPushButton::clicked, this, &AlbumInfo::ClickCollect);
    setPost();

    // 设置收藏属性，未登录、不是专辑、等情况下不可操作
    if(!this->ifLogin || info.type != COLLEC_TYPE::album)
        ui->pushButton_collect->setEnabled(false);

    // 是否收藏
    setCollect();
}

AlbumInfo::~AlbumInfo()
{
    delete ui;
}

void AlbumInfo::setCollect()
{
    if(info.ifCollect && ifLogin)
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_2.png"));
    else
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_1.png"));
    ui->pushButton_collect->setIconSize(ui->pushButton_collect->iconSize());
}

void AlbumInfo::setPost()
{
    ui->label_post->clear();
    QString postPath = ClientSetting::getInstance().getCurDirPath()
                       + Path[PathType::Post] + "/" + info.coverFileName;
    QPixmap pixmap(postPath);
    if(pixmap.isNull())
        pixmap.load(":/Img/icon/localFold.png");
    ui->label_post->setPixmap(pixmap.scaled(ui->label_post->sizeHint(),Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void AlbumInfo::ClickCollect()
{
    info.ifCollect = ! info.ifCollect;
    setCollect();
    emit COLLECT_ALBUM(info.id, info.ifCollect);
}
