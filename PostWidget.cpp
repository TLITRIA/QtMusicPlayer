#include "PostWidget.h"
#include "common.h"

#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

struct PostWidgetPivate
{
    int rotateDegree;
    QTimer timer;
    QPixmap picture;
};

PostWidget::PostWidget(QWidget *parent)
    : QWidget{parent}
    , p(new PostWidgetPivate)
{
    connect(&p->timer,&QTimer::timeout,this,&PostWidget::UpdateScene);
}

PostWidget::~PostWidget()
{
    delete p;
}

void PostWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if(event->size().height() < 300)
        this->resize(300, 300);
    else
        this->resize(event->size().height(), event->size().height());
}

void PostWidget::SetPixMap(const QPixmap &picture)
{
    p->picture = picture;
}

void PostWidget::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    // painter.eraseRect(rect());
    painter.setRenderHint(QPainter::Antialiasing);

    QPoint center = QPoint(this->width()/2, this->height()/2);
    int r = std::min(this->height(), this->width()) / 2;

    QPainterPath painterPath;
    painterPath.addEllipse(center, r, r);

    QTransform transform;
    transform.translate(center.x(), center.y());
    transform.rotate(p->rotateDegree);

    painter.setClipPath(painterPath);
    painter.setTransform(transform);

    // todo 这里有异常输出scall判断为null
    if(!p->picture.isNull())
    {
        p->picture.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(-this->width()/2, -this->height()/2, this->width(), this->height(), p->picture);
    }
}

void PostWidget::Start()
{
    p->timer.start(50);
}

void PostWidget::Pause()
{
    p->timer.stop();
}

void PostWidget::UpdateScene()
{
    p->rotateDegree = (p->rotateDegree + 2) % 360;
    update();
}
