#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QWidget>
struct PostWidgetPivate;

class PostWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PostWidget(QWidget *parent = nullptr);
    ~PostWidget();
    void resizeEvent(QResizeEvent *event) override;

    void SetPixMap(const QPixmap &picture);
    void paintEvent(QPaintEvent *e);

    void Start();
    void Pause();
signals:
public slots:
    void UpdateScene();
private:
    PostWidgetPivate *p;
};

#endif // POSTWIDGET_H
