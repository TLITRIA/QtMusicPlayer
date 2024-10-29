#include "SettingPage.h"
#include "ui_SettingPage.h"

#include <QFileDialog>
#include <QMessageBox>

struct SettingPagePrivate
{
    QString userId;
    QString name;
    QString icon;
    QString downPath;
    QString styleFile;
    QStringList styles;
};
SettingPage::SettingPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingPage)
{
    MYLOG;
    ui->setupUi(this);

    this->setFixedSize(this->width(), this->height());
    qApp->installEventFilter(this);

    ui->label_iconStatus->clear();
    ui->label_nameStatus->clear();
    ui->label_styleStatus->clear();
    ui->label_id->clear();

    setLabelBold(ui->label_iconStatus, true);
    setLabelBold(ui->label_nameStatus, true);
    setLabelBold(ui->label_styleStatus, true);

    connect(ui->lineEdit_name, &QLineEdit::textEdited, [this](){
        if(ui->lineEdit_name->text() != p->name)
            ui->label_nameStatus->setText("已修改");
        else
            ui->label_nameStatus->setText("");
    });

    connect(ui->lineEdit_name, &QLineEdit::editingFinished, [this](){
        if(!ui->lineEdit_name->text().isEmpty())
            return;
        QMessageBox::information(this, "提示", "用户名不能为空");
        ui->lineEdit_name->setText(p->name);
        ui->label_nameStatus->setText("");
    });

    connect(ui->pushButton_setIcon, &QPushButton::clicked, this, &SettingPage::changeIcon);
    // connect(ui->pushButton_setDown, &QPushButton::clicked, this, &SettingPage::changeDown);
    connect(ui->comboBox_style, &QComboBox::currentTextChanged, this, &SettingPage::changeStyle);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &SettingPage::clickCancel);
    connect(ui->pushButton_save, &QPushButton::clicked, this, &SettingPage::clickSave);
}

SettingPage::~SettingPage()
{
    MYLOG;
    delete ui;
}

void SettingPage::setLabelBold(QLabel* label, bool val)
{
    QFont font = label->font();
    font.setBold(val);
    label->setFont(font);
}

void SettingPage::clearALL()
{
    p = new SettingPagePrivate;

    isUserAction = false;
    // ui->label_downStatus->setText("");
    ui->label_iconStatus->setText("");
    ui->label_nameStatus->setText("");
    ui->label_styleStatus->setText("");
    isUserAction = true;

    ui->label_id->clear();
    ui->comboBox_style->clear();
}

void SettingPage::inputUser(bool ifLogin, QCStrRef id, QCStrRef name, QCStrRef icon)
{
    if(!ifLogin)
    {
        ui->lineEdit_name->setEnabled(false);
        ui->pushButton_setIcon->setEnabled(false);
        return;
    }

    ui->lineEdit_name->setEnabled(true);
    ui->pushButton_setIcon->setEnabled(true);

    ui->label_id->setText(id);
    ui->lineEdit_name->setText(name);
    ui->lineEdit_icon->setText(icon);

    p->userId = id;
    p->icon = icon;
    p->name = name;
}

void SettingPage::inputLocalConfig(QCStrRef downPath, QCStrRef styleFileName, QStringList styleFileList)
{
    // ui->lineEdit_downFold->setText(downPath);

    isUserAction = false;
    ui->comboBox_style->addItems(styleFileList);
    ui->comboBox_style->setCurrentText(styleFileName);
    isUserAction = true;

    p->styleFile = styleFileName;
    p->styles = styleFileList;
    p->downPath = downPath;
}


void SettingPage::changeIcon()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles); // 单个存在的文件
    fileDialog.setNameFilter("ALL Icon (*.png *.jfif *.jpg)"); // 设置过滤器
    fileDialog.setViewMode(QFileDialog::List);

    if (fileDialog.exec()) {
        QStringList fileNames = fileDialog.selectedFiles();
        if(fileNames.size()>1)
        {
            MYLOG<<"请选单独的图片";
            return;
        }
        if(fileNames.size()==0)
        {
            MYLOG<<"请不要空选图片";
            return;
        }
        QFile sourceFile(fileNames[0]);
        QFileInfo Finfo(fileNames[0]);
        QString destPath = C::ClientSetting::getInstance().getCurDirPath()
            + C::Path[C::PathType::HeadIcon] + "/" + Finfo.fileName();
        sourceFile.copy(destPath);
        ui->lineEdit_icon->setText(Finfo.fileName());
        ui->label_iconStatus->setText(ui->lineEdit_icon->text() != p->icon ? "已修改" : "");
    }
}

void SettingPage::changeDown()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::Directory); // 单个存在的文件
    fileDialog.setViewMode(QFileDialog::List);

    if (fileDialog.exec()) {
        QStringList dirNames = fileDialog.selectedFiles();
        if(dirNames.size()>1)
        {
            MYLOG<<"请选单独的文件夹";
            return;
        }
        if(dirNames.size()==0)
        {
            MYLOG<<"请不要空选文件夹";
            return;
        }
        // ui->lineEdit_downFold->setText(dirNames[0]);
        // ui->label_downStatus->setText(ui->lineEdit_downFold->text() != p->downPath ? "已修改" : "");
    }
}

void SettingPage::changeStyle(QCStrRef style)
{
    if(!isUserAction)
        return;
    ui->label_styleStatus->setText(ui->comboBox_style->currentText() != p->styleFile ? "已修改" : "");
}

void SettingPage::clickCancel()
{
    // ui->label_downStatus->setText("");
    ui->label_iconStatus->setText("");
    ui->label_nameStatus->setText("");
    ui->label_styleStatus->setText("");

    isUserAction = false;
    // ui->lineEdit_downFold->setText(p->downPath);
    ui->lineEdit_name->setText(p->name);
    ui->lineEdit_icon->setText(p->icon);
    ui->comboBox_style->setCurrentText(p->styleFile);
    isUserAction = true;
}

void SettingPage::clickSave()
{
    if(ui->lineEdit_name->text() != p->name)
    {
        emit UserChangeName(p->userId, ui->lineEdit_name->text());
    }

    if(ui->lineEdit_icon->text() != p->icon)
    {
        emit transFile_filename(ui->lineEdit_icon->text(), C::PathType::HeadIcon);
        emit UserChangeIcon(p->userId, ui->lineEdit_icon->text());
    }
    if(ui->comboBox_style->currentText() != p->styleFile)
    {
        QString filePath = C::ClientSetting::getInstance().getCurDirPath()
        + C::Path[PathType::Style] + "/" + ui->comboBox_style->currentText();
        QFile qssfile(filePath);
        if(qssfile.open(QIODevice::ReadOnly) == true)
        {
            MYLOG<<"成功加载样式表:"<<qssfile.fileName();
            qApp->setStyleSheet(qssfile.readAll());
            qssfile.close();
        }

        setStyle(ui->comboBox_style->currentText());
    }

    emit QUIT();
}
