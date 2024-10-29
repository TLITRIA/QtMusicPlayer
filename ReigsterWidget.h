#ifndef REIGSTERWIDGET_H
#define REIGSTERWIDGET_H

#include <QWidget>
#include "common.h"
namespace Ui {
class ReigsterWidget;
}

class ReigsterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ReigsterWidget(QWidget *parent = nullptr);
    ~ReigsterWidget();
signals:
    void RETURN_LOGIN();
    void REGISTER(QCStrRef username, QCStrRef pwd, bool ifAutoLog, bool ifRemPwd);
private:
    Ui::ReigsterWidget *ui;
};

#endif // REIGSTERWIDGET_H
