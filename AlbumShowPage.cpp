#include "AlbumShowPage.h"
#include "ui_AlbumShowPage.h"
#include "AlbumInfo.h"

AlbumShowPage::AlbumShowPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AlbumShowPage)
{
    ui->setupUi(this);
    content = new QGridLayout();
    ui->scrollArea->widget()->setLayout(content);
    content->setSizeConstraint(QLayout::SetFixedSize); // 布局会被限制为一个固定的大小，这个大小由其内容决定。如果布局中的内容发生变化，布局的大小不会随之调整，通常用于需要固定布局尺寸的场景。
    ColNum = 4;
}

AlbumShowPage::~AlbumShowPage()
{
    delete ui;
}

void AlbumShowPage::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    // todo 如何预估缩放列数后页面的宽度是否适合？
    /* col w1 w2 适合的范围
     * 3 354 384 386~404
     * 4 472 508 516~535
     * 5 590 632 636~768
     * 6 708 756
     */
    if(event->size().width() < 404)
        Rerange(2);
    else if(event->size().width() < 546)
        Rerange(3);
    else if(event->size().width() < 648)
        Rerange(4);
    else if(event->size().width() < 779)
        Rerange(5);
    else if(event->size().width() < 850)
        Rerange(6);
    else
        Rerange(7);
}

void AlbumShowPage::setLogin(bool val)
{
    this->ifLogin = val;
}

void AlbumShowPage::INPUT(const QVector<QPair<CollectionInfo, QStringList>> &album_input, bool if_login)
{
    CLEAN();
    setLogin(if_login);
    for(int i = 0; i < album_input.size(); i++)
    {
        AlbumInfo* w = new AlbumInfo(album_input[i].first, ifLogin);
        connect(w, &AlbumInfo::COLLECT_ALBUM, this, &AlbumShowPage::COLLECT_ALBUM);
        content->addWidget(w,i/ColNum,i%ColNum,1,1);
    }
}

void AlbumShowPage::CLEAN()
{
    QLayoutItem* child;
    while((child = content->takeAt(0)) != nullptr)
    {
        content->removeItem(child);
        child->widget()->deleteLater();
    }
}

void AlbumShowPage::Rerange(int col)
{
    ColNum = col;
    QVector<QLayoutItem*> Childs;
    QLayoutItem* child;
    while((child = content->takeAt(0)) != nullptr)  //
    {
        content->removeItem(child); // takeat()是已经取走了吗？？
        Childs.push_back(child);
    }
    if(Childs.size()<1)
    {
        return;
    }
    if(ColNum < 1)
        ColNum = 1;
    if(ColNum > Childs.size())
        ColNum = Childs.size();
    for(int i = 0; i < Childs.size(); i++)
    {
        content->addWidget(Childs[i]->widget(),i/ColNum,i%ColNum,1,1);
    }
}

int AlbumShowPage::calculateGridLayoutWidth(QGridLayout *layout)
{
    int totalWidth = 0;
    for (int i = 0; i < layout->columnCount(); ++i) {
        int colWidth = 0;
        for (int j = 0; j < layout->rowCount(); ++j) {
            QLayoutItem *item = layout->itemAtPosition(j, i);
            if (item != nullptr)
                colWidth = std::max(colWidth, item->widget()->sizeHint().width());
        }
        totalWidth += colWidth;
    }
    return totalWidth;
}
