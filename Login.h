#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include <ReigsterWidget.h>

#include "common.h"


namespace Ui {
class Login;
}


struct LoginPrivate;
struct LoginMessage;


class Login : public QWidget
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    bool eventFilter(QObject *watched, QEvent *event)override;
    bool getMessage(QCStrRef account); // 在本地寻找用户信息
    void updateAccount();

    void doAutoLog();

signals:
    void doLogin(QCStrRef ID, QCStrRef userName, QCStrRef pwd,
                 bool ifAutolog, bool ifRempwd);
    void doRegister(QCStrRef userName, QCStrRef pwd,
                    bool ifAutolog, bool ifRempwd);

public slots:
    void FillCompleterAccountList(); // 显示自动填充的用户名
    void CompleteAccountSlots(); // 自动补全
    void onLogin();

private:
    Ui::Login *ui;
    LoginPrivate *p;
    LoginMessage *message;
    ReigsterWidget *registerPage;
};

#endif // LOGIN_H
