#include "MusicListPage.h"
#include "ui_MusicListPage.h"

MusicListPage::MusicListPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicListPage)
{
    ui->setupUi(this);

    connect(ui->listWidget_collection, &QListWidget::itemDoubleClicked, this, &MusicListPage::CollectionDoubleClicked);
    connect(ui->listWidget_music, &QListWidget::itemDoubleClicked, this, &MusicListPage::MusicDoubleClicked);
    connect(ui->pushButton_edit, &QPushButton::clicked, this, &MusicListPage::EDITCOLLECTION);
    connect(ui->pushButton_down, &QPushButton::clicked, this, &MusicListPage::clickDown);
    connect(ui->pushButton_collect, &QPushButton::clicked, this, &MusicListPage::clickCollect);

    setDown();
    setCollect();
}

MusicListPage::~MusicListPage()
{
    delete ui;
}

int MusicListPage::currentRow_collection()
{
    return ui->listWidget_collection->currentRow();
}

int MusicListPage::currentRow_music()
{
    return ui->listWidget_music->currentRow();
}

int MusicListPage::count_music()
{
    return ui->listWidget_music->count();
}

int MusicListPage::musicScrollBar_position()
{
    return ui->listWidget_music->verticalScrollBar()->value();
}

void MusicListPage::AddMusicList(InfoBar *newBar)
{
    QListWidgetItem *newItem = new QListWidgetItem(ui->listWidget_music);
    newItem->setSizeHint(QSize(newBar->width(), newBar->height()));
    ui->listWidget_music->setItemWidget(newItem, newBar);
}

void MusicListPage::AddCollectionList(QListWidgetItem *newItem)
{
    ui->listWidget_collection->addItem(newItem);
}

void MusicListPage::RemoveMusicList()
{
    ui->listWidget_music->clear();
}

void MusicListPage::RemoveCollectionList()
{
    ui->listWidget_collection->clear();
}

void MusicListPage::setCurrentMusicRow(int row)
{
    ui->listWidget_music->setCurrentRow(row); // 设置当前选择的行数
}

void MusicListPage::setCurrentCollectionRow(int row)
{
    ui->listWidget_collection->setCurrentRow(row);
}


void MusicListPage::moveMusicScoll(int val)
{
    ui->listWidget_music->verticalScrollBar()->setValue(val);
}

void MusicListPage::setLogin(bool val)
{
    ifLogin = val;
    if(!ifLogin)
    {
        ui->pushButton_down->setEnabled(false);
        ui->pushButton_collect->setEnabled(false);
    }
    else
    {
        ui->pushButton_down->setEnabled(true);
        ui->pushButton_collect->setEnabled(true);
    }
}

void MusicListPage::setCollectable(bool val)
{
    ui->pushButton_collect->setEnabled(val);
}

void MusicListPage::setIfCollect(bool val)
{
    ifCollect = val;
    setCollect();
}

void MusicListPage::setDown()
{
    if(ifDown)
        ui->pushButton_down->setIcon(QIcon(":/Img/icon/yes.png"));
    else
        ui->pushButton_down->setIcon(QIcon(":/Img/icon/download.png"));
    ui->pushButton_down->setIconSize(ui->pushButton_down->iconSize());

    ui->pushButton_down->setEnabled(!ifDown); // 已下载的不用下载
}

void MusicListPage::setCollect()
{
    if(ifCollect)
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_2.png"));
    else
        ui->pushButton_collect->setIcon(QIcon(":/Img/icon/collect_1.png"));
    ui->pushButton_collect->setIconSize(ui->pushButton_collect->iconSize());
}

void MusicListPage::clickDown()
{
    emit DOWN();
}

void MusicListPage::clickCollect()
{
    ifCollect = !ifCollect;
    setCollect();
    emit COLLECT(ifCollect);
}


