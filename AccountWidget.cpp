#include "AccountWidget.h"
#include "ui_AccountWidget.h"
#include "common.h"
#include <QPixMap>

AccountWidget::AccountWidget(QDomNode userNode, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AccountWidget)
{
    ui->setupUi(this);

    setUserID(userNode.toElement().attribute("ID"));
    setUserName(userNode.toElement().attribute("name"));
    setPicture(userNode.toElement().attribute("ICON"));
}

AccountWidget::~AccountWidget()
{
    delete ui;
}

void AccountWidget::setUserName(const QString &name)
{
    ui->label_username->setText(name);
}

void AccountWidget::setUserID(const QString &ID)
{
    ui->label_account->setText(ID);
}

void AccountWidget::setPicture(const QString &picturePath)
{
    QString tmpDirPath = C::ClientSetting::getInstance().getCurDirPath();
    QPixmap headIcon(tmpDirPath + C::Path[C::PathType::HeadIcon] + "/" + picturePath);

    if(headIcon.isNull())
    {
        headIcon.load(":/Img/icon/default_User.png");
    }

    ui->label_icon->setPixmap(headIcon.scaled(ui->label_icon->sizeHint()));
}

QString AccountWidget::GetId()
{
    return ui->label_account->text();
}

QString AccountWidget::GetUsername()
{
    return ui->label_username->text();
}
