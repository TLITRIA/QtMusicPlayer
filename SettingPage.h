#ifndef SETTINGPAGE_H
#define SETTINGPAGE_H

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QEvent>

#include "common.h"

using namespace C;

namespace Ui {
class SettingPage;
}
struct SettingPagePrivate;
class SettingPage : public QWidget
{
    Q_OBJECT

public:
    explicit SettingPage(QWidget *parent = nullptr);
    ~SettingPage();

    void setLabelBold(QLabel* label, bool val);
    void clearALL();
    void inputUser(bool ifLogin, QCStrRef id, QCStrRef name, QCStrRef icon);
    void inputLocalConfig(QCStrRef downPath, QCStrRef styleFileName, QStringList styleFileList);

signals:
    void UserChangeName(QCStrRef u_id, QCStrRef name);
    void UserChangeIcon(QCStrRef u_id, QCStrRef icon);
    void transFile_filename(QCStrRef fileName, int type);

    void setStyle(QCStrRef style);
    void QUIT();

private slots:
    void changeIcon();
    void changeDown();
    void changeStyle(QCStrRef style);
    void clickCancel();
    void clickSave();
private:
    void SetCurUnsaved();
    Ui::SettingPage *ui;
    SettingPagePrivate* p;

    bool isUserAction = true; // 判断是否是用户行为
};

#endif // SETTINGPAGE_H
