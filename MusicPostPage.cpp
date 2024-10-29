#include "MusicPostPage.h"
#include "PostWidget.h"

#include "ui_MusicPostPage.h"

#include <map>
#include <QPalette>

#include <QFile>

#include <QTextCodec>

struct MusicPostPrivate
{
    PostWidget* postWidget;
    std::vector<std::pair<qint64,QString>> LRCVector;  // 歌词列表
    QPixmap post;
};

MusicPostPage::MusicPostPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MusicPostPage)
    , p(new MusicPostPrivate)
{
    ui->setupUi(this);
    p->postWidget = new PostWidget();
    ui->gridLayout_post->addWidget(p->postWidget);
    p->postWidget->setMinimumHeight(300);
}
MusicPostPage::~MusicPostPage()
{
    delete ui;
    delete p;
}

void MusicPostPage::setLCR(QCStrRef path)
{
    QFile file(path);
    if(file.open(QIODevice::ReadOnly) == false )
    {
        MYLOG<< file.fileName() <<"文件打开失败";
        return;
    }

    QByteArray context = file.readAll();
    QTextStream in(&file);
    file.close();
    QTextCodec *codec;


    if(GetCorrectUnicode(context) == "UTF8")
    {
        codec=QTextCodec::codecForName("utf-8");
        QString tmp = codec->toUnicode(context);

        QStringList text = tmp.split("\n");
        for(int i = 0; i < text.size()-1; i++)
        {
            QStringList lineList = text[i].split(']');
            if (lineList.size() != 2)
                continue;
            QString LRC =lineList[1];
            long position = perserLRCTime(lineList[0]);
            p->LRCVector.push_back({position, LRC});
        }
    }
    else
    {
        codec=QTextCodec::codecForLocale();
        QString tmp = codec->toUnicode(context);
        QStringList text = tmp.split("\r\n");
        for(int i = 0; i < text.size()-1; i++)
        {
            QStringList lineList = text[i].split(']');
            if (lineList.size() != 2)
                continue;
            QString LRC =lineList[1];
            long position = perserLRCTime(lineList[0]);
            p->LRCVector.push_back({position, LRC});
        }
    }
}

void MusicPostPage::setNoneLCR()
{
    p->LRCVector.push_back({0, "暂无歌词"});
}

void MusicPostPage::play()
{
    p->postWidget->Start();
}

void MusicPostPage::pause()
{
    p->postWidget->Pause();
}

long MusicPostPage::perserLRCTime(QString &timeStr)
{
    timeStr.remove(0, 1);
    int minute = timeStr.split(":")[0].toInt();
    int second = timeStr.split(":")[1].split(".")[0].toInt();
    int milleSecond = timeStr.split(".")[1].toInt();
    return minute * 1000*60 + second * 1000 + milleSecond;
}

QString MusicPostPage::GetCorrectUnicode(const QByteArray &ba)
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

void MusicPostPage::setMusicInfo(QCStrRef lcr, QCStrRef post)
{
    QString tmpDirPath = C::ClientSetting::getInstance().getCurDirPath();

    p->LRCVector.clear();
    ui->label_lrc->clear();

    if(lcr == QString())
        setNoneLCR();
    else
        setLCR(tmpDirPath + C::Path[C::PathType::Lcr]  + "/" + lcr);

    if(post == QString())
        p->post = QPixmap(":/Img/icon/post_default.png");
    else
        p->post = QPixmap(tmpDirPath + C::Path[C::PathType::Post] + "/" + post);

    p->postWidget->SetPixMap(p->post);
}


void MusicPostPage::changeLCR(qint64 position)
{
    if(p->LRCVector.size() ==  0)
    {
        MYLOG<<"未知错误";
        return;
    }


    if(p->LRCVector.front().first > position)
    {
        ui->label_lrc->setText(p->LRCVector.front().second);
        return;
    }

    if(p->LRCVector.back().first <= position)
    {
        ui->label_lrc->setText(p->LRCVector.back().second);
        return;
    }

    for(int i = 0; i < p->LRCVector.size()-1; i++)
    {
        if(p->LRCVector[i].first <= position && p->LRCVector[i+1].first > position)
        {
            ui->label_lrc->setText(p->LRCVector[i].second);
            return;
        }
    }

    ui->label_lrc->setText("暂无歌词");
}

