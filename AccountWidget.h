#ifndef ACCOUNTWIDGET_H
#define ACCOUNTWIDGET_H

#include <QWidget>
#include <QDomNode>
namespace Ui {
class AccountWidget;
}

class AccountWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AccountWidget(QDomNode userNode, QWidget *parent = nullptr);
    ~AccountWidget();
    void setUserName(const QString& name);
    void setUserID(const QString& ID);
    void setPicture(const QString& picturePath);
    QString GetId();
    QString GetUsername();
private:
    Ui::AccountWidget *ui;
};

#endif // ACCOUNTWIDGET_H
