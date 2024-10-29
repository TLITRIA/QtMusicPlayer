#include "Login.h"
#include "ui_Login.h"
#include "ConfigXml.h"
#include "AccountWidget.h"

#include <QListWidget>
#include <QMessageBox>

struct LoginPrivate
{
    QListWidget accountListWidget;
    ConfigXml *xml;
    std::map<QString, QDomNode> accountList;
    QString AutoLogID;
};

struct LoginMessage
{
    QString ID;
    QString userName;
    QString headIconPath;
    bool ifAutolog;
};

Login::Login(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Login)
    , p(new LoginPrivate)
    , message(new LoginMessage)
{
    MYLOG;
    p->xml = &ConfigXml::getInstance();
    ui->setupUi(this);
    setWindowTitle("Login");

    QAction *qq = new QAction;
    qq->setIcon(QIcon(":/Img/icon/qq.png"));
    ui->lineEdit_account->addAction(qq, QLineEdit::LeadingPosition);

    QAction *lock = new QAction;
    lock->setIcon(QIcon(":/Img/icon/lock.png"));
    ui->lineEdit_password->addAction(lock, QLineEdit::LeadingPosition);

    // QAction *pwd_eye = new QAction;
    // pwd_eye->setIcon(QIcon(":/Img/icon/pwd_eye.png"));
    // ui->lineEdit_password->addAction(pwd_eye, QLineEdit::TrailingPosition);

    ui->lineEdit_account->setPlaceholderText("请输入用户名或ID号...");
    ui->lineEdit_password->setPlaceholderText("请输入密码...");

    p->accountListWidget.setParent(this);
    p->accountListWidget.hide();

    qApp->installEventFilter(this); // 把当前界面加到全局过滤器中 什么意思？

    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    QRegularExpression regx("[A-Za-z0-9_]+");
    QRegularExpressionValidator *pwd_validator = new QRegularExpressionValidator(regx, ui->lineEdit_password);
    ui->lineEdit_password->setValidator(pwd_validator);
    ui->lineEdit_password->setMaxLength(18); // 最多输入18位

    // registerpage
    registerPage = new ReigsterWidget;
    registerPage->hide();
    registerPage->resize(this->width(), this->height());

    connect(ui->lineEdit_account, &QLineEdit::textChanged, this, &Login::FillCompleterAccountList); // 输入ID或名称产生下拉列表
    connect(&p->accountListWidget, &QListWidget::clicked, this, &Login::CompleteAccountSlots); // 点击下拉列表，填入信息
    connect(ui->pushButton_login, &QPushButton::clicked, this, &Login::onLogin); // 登陆
    // 注册
    connect(ui->pushButton_register, &QPushButton::clicked, [this](){ // 前往注册
        registerPage->move(this->x(), this->y());
        registerPage->show();
        this->hide();
    });
    connect(registerPage, &ReigsterWidget::RETURN_LOGIN, [this](){ // 返回登陆
        registerPage->hide();
        this->show();
    });
    connect(registerPage, &ReigsterWidget::REGISTER, [this](QCStrRef username, QCStrRef pwd, bool ifAutoLog, bool ifRemPwd){ //注册
        registerPage->hide();
        emit doRegister(username, pwd, ifAutoLog, ifRemPwd);
    });

    updateAccount();
}

Login::~Login()
{
    delete ui;
    delete p;
    delete message;
}

bool Login::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress && watched != ui->lineEdit_account)
    {
        QTimer::singleShot(100, &p->accountListWidget, &QListWidget::hide);
    }
    return QWidget::eventFilter(watched, event);
}

bool Login::getMessage(QCStrRef account)
{
    // 先判断是ID还是用户名
    bool ifIsID = JudgeIsID(account);
    for(auto ite = p->accountList.begin();
         ite != p->accountList.end(); ite++)
    {
        if(ifIsID && ite->first == account)
        {
            message->userName = ite->second.toElement().firstChildElement("NAME").text();
            message->headIconPath = ite->second.toElement().firstChildElement("HEADICON").text();
            message->ID = account;
            message->ifAutolog = (ite->second.toElement().attribute("autologin") == "true");
            return true;
        }
        if(!ifIsID && ite->second.toElement().firstChildElement("NAME").text() == account)
        {
            message->userName = account;
            message->headIconPath = ite->second.toElement().firstChildElement("HEADICON").text();
            message->ID = ite->first;
            message->ifAutolog = (ite->second.toElement().attribute("autologin") == "true");
            return true;
        }
    }
    if(ifIsID)
    {
        message->userName = "";
        message->ID = account;
    }
    else
    {
        message->userName = account;
        message->ID = "";
    }
    message->headIconPath = "";
    message->ifAutolog = ui->checkBox_autolog;
    return false; // 未在本地找到信息
}

void Login::updateAccount()
{
    p->accountList.clear();
    p->xml->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                              + C::Path[C::PathType::Config]
                              + C::File[C::FileType::LoginAccount]);
    QDomElement root = p->xml->ReadFromXML();
    QDomElement AccountsNode = root.firstChildElement("Accounts");
    if(!AccountsNode.isNull())
    {
        for(auto userNode = AccountsNode.firstChild();
             !userNode.isNull();
             userNode = userNode.nextSibling())
        {
            if(userNode.toElement().tagName() == "account")
            {
                QString accountID = userNode.toElement().attribute("ID");
                p->accountList.insert({accountID, userNode});
            }
        }
    }
}

void Login::doAutoLog()
{
    if(p->xml->getAutoLogAccount(&p->AutoLogID))
    {
        MYLOG<<p->AutoLogID;
        QDomNode userNode = p->accountList[p->AutoLogID];

        emit doLogin(p->AutoLogID,
                     userNode.toElement().attribute("name"),
                     "",
                     true,
                     userNode.toElement().attribute("ifRempwd") == "true");
    }
}


void Login::FillCompleterAccountList()
{
    updateAccount();
    int count = 0; // 如果没有相似账户就不显示下拉列表
    p->accountListWidget.clear();
    QString content = ui->lineEdit_account->text();
    for(auto ite = p->accountList.begin();
         ite != p->accountList.end(); ite++)
    {
        QListWidgetItem *item = new QListWidgetItem;
        AccountWidget *w = new AccountWidget(ite->second);
        if(ite->first.contains(content) || w->GetUsername().contains(content))
        {
            count++;
            p->accountListWidget.addItem(item);
            p->accountListWidget.setItemWidget(item, w);
            item->setSizeHint(w->sizeHint());
        }
    }
    p->accountListWidget.move(ui->lineEdit_account->x(), ui->lineEdit_account->y() + ui->lineEdit_account->height());
    p->accountListWidget.setFixedWidth(ui->lineEdit_account->width());
    p->accountListWidget.setFixedHeight(ui->checkBox_autolog->y() - ui->lineEdit_account->y());
    if(count==0)
        p->accountListWidget.hide();
    else
        p->accountListWidget.show();
}

void Login::CompleteAccountSlots()
{
    auto item = p->accountListWidget.currentItem();
    QWidget *w = p->accountListWidget.itemWidget(item);
    AccountWidget *aw = static_cast<AccountWidget*>(w);
    ui->lineEdit_account->setText(aw->GetId());

    QDomNode userNode = p->accountList[aw->GetId()];

    p->xml->getAutoLogAccount(&p->AutoLogID);
    ui->checkBox_autolog->setChecked(aw->GetId() == p->AutoLogID);
    ui->checkBox_remPwd->setChecked(userNode.toElement().attribute("ifRempwd") == "true");

    if(ui->checkBox_remPwd->isChecked())
        ui->lineEdit_password->setText("123456789123456789");
}

void Login::onLogin()
{
    // 提示补全信息
    if(ui->checkBox_remPwd->isChecked())
    {
        if(ui->lineEdit_account->text().isEmpty())
        {
            QMessageBox::information(NULL, "提示", "请补全信息");
            return;
        }
    }
    else
    {
        if(ui->lineEdit_account->text().isEmpty() ||
            ui->lineEdit_password->text().isEmpty())
        {
            QMessageBox::information(NULL, "提示", "请补全信息");
            return;
        }
    }
    // 获取信息
    getMessage(ui->lineEdit_account->text());
    emit doLogin(message->ID,
                 message->userName,
                 ui->lineEdit_password->text(),
                 ui->checkBox_autolog->isChecked(),
                 ui->checkBox_remPwd->isChecked());
    QTimer::singleShot(10, this, &QWidget::hide);
}

