#include "SelectContain.h"
#include "ui_SelectContain.h"

SelectContain::SelectContain(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SelectContain)
{
    ui->setupUi(this);

    ui->tableWidget_source->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置表格数据区内的所有单元格都不允许编辑
    ui->tableWidget_source->setSelectionMode(QAbstractItemView::NoSelection); //设置不可选中
    ui->tableWidget_source->verticalHeader()->hide();

    connect(ui->pushButton_exit, &QPushButton::clicked, this, &SelectContain::ClickExit);
    connect(ui->pushButton_save, &QPushButton::clicked, this, &SelectContain::ClickSave);
}

SelectContain::~SelectContain()
{
    delete ui;
}

void SelectContain::input(QMap<QString, MusicInfo> source, const QStringList& ids)
{
    ClearMessage();
    this->source = source;

    ui->tableWidget_source->setColumnCount(4);
    ui->tableWidget_source->setRowCount(source.size());
    ui->tableWidget_source->setHorizontalHeaderLabels({"复选", "编号","歌曲名","作者"});

    int tmpI = 0;
    for(auto ite = source.begin();
         ite != source.end(); ite++)
    {
        // checkbox
        QCheckBox *newBox = new QCheckBox();
        if(ids.contains(ite.key()))
            newBox->setCheckState(Qt::Checked);
        connect(newBox, &QCheckBox::checkStateChanged, this, &SelectContain::ChangeCheckBox);
        ui->tableWidget_source->setCellWidget(tmpI, 0, newBox);
        // id
        QTableWidgetItem* newItem = new QTableWidgetItem(ite.value().id);
        ui->tableWidget_source->setItem(tmpI, 1, newItem);
        // name
        QTableWidgetItem* newName = new QTableWidgetItem(ite.value().name);
        ui->tableWidget_source->setItem(tmpI, 2, newName);
        // author
        QTableWidgetItem* newAuthor = new QTableWidgetItem(ite.value().author);
        ui->tableWidget_source->setItem(tmpI, 3, newAuthor);

        tmpI++;
    }

    // 设置列宽尽可能小
    ui->tableWidget_source->resizeColumnsToContents();
}

void SelectContain::ClickExit()
{
    this->hide();
}

void SelectContain::ClickSave()
{
    QStringList ret;

    for(int i = 0; i < ui->tableWidget_source->rowCount(); i++)
    {
        QCheckBox *tmpbox = (QCheckBox*)ui->tableWidget_source->cellWidget(i,0);//获取控件
        if(tmpbox->checkState() == Qt::CheckState::Checked)
            ret.append(ui->tableWidget_source->item(i,1)->text());
    }

    emit UPDATE_COLLECTION(ret);
    this->hide();
}

void SelectContain::ChangeCheckBox(int type)
{
    int sum = 0;
    for(int i = 0; i < ui->tableWidget_source->rowCount(); i++)
    {
        QCheckBox *tmpbox = (QCheckBox*)ui->tableWidget_source->cellWidget(i,0);//获取控件
        if(tmpbox->checkState() == Qt::CheckState::Checked)
            sum++;
    }

    ui->label_summary->setText(QString("共%1首，已选%2首").arg(ui->tableWidget_source->rowCount()).arg(sum));
}

void SelectContain::ClearMessage()
{
    ui->tableWidget_source->clear();
    source.clear();
}
