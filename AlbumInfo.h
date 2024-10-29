#ifndef ALBUMINFO_H
#define ALBUMINFO_H

#include <QWidget>
#include "common.h"

using namespace C;

namespace Ui {
class AlbumInfo;
}

class AlbumInfo : public QWidget
{
    Q_OBJECT

public:
    explicit AlbumInfo(const CollectionInfo& input, bool ifLogin, QWidget *parent = nullptr);
    ~AlbumInfo();

    void setCollect();
    void setPost();

private slots:
    void ClickCollect();

signals:
    void COLLECT_ALBUM(const quint64& a_id, bool val);
private:
    Ui::AlbumInfo *ui;
    CollectionInfo info;
    bool ifLogin = false;
};

#endif // ALBUMINFO_H
