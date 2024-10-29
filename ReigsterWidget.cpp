#include "ReigsterWidget.h"
#include "ui_ReigsterWidget.h"

#include <QTimer>
#include <QMessageBox>

ReigsterWidget::ReigsterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReigsterWidget)
{
    ui->setupUi(this);
    setWindowTitle("Register");

    QAction *qq = new QAction;
    qq->setIcon(QIcon(":/Img/icon/qq.png"));
    ui->lineEdit_username->addAction(qq, QLineEdit::LeadingPosition);

    QAction *lock = new QAction;
    lock->setIcon(QIcon(":/Img/icon/lock.png"));
    ui->lineEdit_password->addAction(lock, QLineEdit::LeadingPosition);

    QAction *pwd_eye = new QAction;
    pwd_eye->setIcon(QIcon(":/Img/icon/pwd_eye.png"));
    ui->lineEdit_password->addAction(pwd_eye, QLineEdit::TrailingPosition);


    ui->lineEdit_username->setPlaceholderText("请输入用户名...");
    ui->lineEdit_password->setPlaceholderText("请输入密码...");
    ui->checkBox_autolog->setCheckable(false);

    connect(ui->pushButton_return_login, &QPushButton::clicked, [this](){
        emit RETURN_LOGIN();
        QTimer::singleShot(100, this, &QWidget::hide);
    });
    connect(ui->pushButton_register, &QPushButton::clicked, [this](){
        if(ui->lineEdit_username->text().isEmpty() ||
            ui->lineEdit_password->text().isEmpty())
        {
            QMessageBox::information(NULL, "提示", "请补全信息");
            return;
        }

        QRegularExpression regex("^\\d{9}$");
        QRegularExpressionMatch match = regex.match(ui->lineEdit_username->text());
        if(match.hasMatch())
        {
            QMessageBox::information(NULL, "提示", "用户名格式不能与密码相同");
            return;
        }

        REGISTER(ui->lineEdit_username->text(),
                 ui->lineEdit_password->text(),
                 ui->checkBox_autolog->isChecked(),
                 ui->checkBox_remPwd->isChecked());
    });
}

ReigsterWidget::~ReigsterWidget()
{
    delete ui;
}
