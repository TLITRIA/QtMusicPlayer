#include "EditCollect.h"
#include "ui_EditCollect.h"

EditCollect::EditCollect(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditCollect)
{
    ui->setupUi(this);
    ui->lineEdit_post->setEnabled(false);

    selectPage = new SelectContain();
    selectPage->setWindowTitle(QObject::tr("编辑收藏"));
    selectPage->setWindowModality(Qt::ApplicationModal); //设置阻塞类型
    selectPage->setAttribute(Qt::WA_ShowModal, true);    //属性设置 true:模态 false:非模态
    selectPage->setWindowFlags(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint | Qt::Dialog);
    selectPage->hide();

    ui->tableWidget_contain->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置表格数据区内的所有单元格都不允许编辑
    ui->tableWidget_contain->setSelectionMode(QAbstractItemView::NoSelection); //设置不可选中
    // ui->tableWidget_contain->horizontalHeader()->hide();
    ui->tableWidget_contain->verticalHeader()->hide();



    //
    // EventFilter *filter = new EventFilter(&button);
    // button.installEventFilter(filter);


    // changed name√ post√ info(事件检测器) contain√
    connect(ui->lineEdit_name, &QLineEdit::textEdited, this, &EditCollect::SetCurUnsaved);
    connect(ui->textEdit_info, &QTextEdit::textChanged, this, &EditCollect::SetCurUnsaved);

    connect(ui->listWidget_collections, &QListWidget::itemDoubleClicked, this, &EditCollect::ClickCollection);
    connect(ui->pushButton_add, &QPushButton::clicked, this, &EditCollect::button_add);
    connect(ui->pushButton_del, &QPushButton::clicked, this, &EditCollect::button_del);
    connect(ui->pushButton_local_post, &QPushButton::clicked, this, &EditCollect::fillFullPostPath);

    connect(ui->pushButton_edit_contain, &QPushButton::clicked, this, &EditCollect::ClickEditContain);
    connect(selectPage, &SelectContain::UPDATE_COLLECTION, this, &EditCollect::ContainSaveTmp);

    connect(ui->pushButton_undo, &QPushButton::clicked, this, &EditCollect::Click_Undo);
    connect(ui->pushButton_save_tmp, &QPushButton::clicked, this, &EditCollect::ClickSaveTmp);
    connect(ui->pushButton_exit_without_save, &QPushButton::clicked, this, &EditCollect::ClickExit);
    connect(ui->pushButton_save_exit, &QPushButton::clicked, this, &EditCollect::ClickSave);
}

EditCollect::~EditCollect()
{
    if(selectPage!=nullptr)
        delete selectPage;
    delete ui;
}

void EditCollect::inputMusic(const QVector<QPair<CollectionInfo, QStringList>> &collection_input,
                             const QMap<QString, MusicInfo>& source_input)
{
    clearALL();

    this->source = source_input;

    for(int i = 0; i < collection_input.size(); i++)
    {
        if(collection_input[i].first.type == COLLEC_TYPE::custom)
        {
            edit.push_back(QPair<QPair<CollectionInfo, QStringList>,int>(collection_input[i], STATUS::UNCHANGED));
            back.push_back(collection_input[i]);
        }
    }

    updateCollectList();
    if(edit.size()>0)
    {
        ui->listWidget_collections->setCurrentRow(0);
        ClickCollection();
    }
}

void EditCollect::clearALL()
{
    back.clear();
    edit.clear();
    source.clear();
    ui->lineEdit_name->clear();
    ui->lineEdit_post->clear();
    isUserAction = false;
    ui->textEdit_info->clear();
    isUserAction = true;
    ui->listWidget_collections->clear();
    ui->tableWidget_contain->clear();
}

void EditCollect::updateCollectList()
{
    ui->listWidget_collections->clear();

    for(int i = 0; i < edit.size(); i++)
    {
        if(edit[i].second == STATUS::UNSAVED)
        {
            ui->listWidget_collections->addItem(edit[i].first.first.foldname + "(未保存)");
            QFont font = ui->listWidget_collections->item(i)->font();
            font.setBold(true);
            ui->listWidget_collections->item(i)->setFont(font);
        }
        else
            ui->listWidget_collections->addItem(edit[i].first.first.foldname);
    }
}

void EditCollect::SetCurUnsaved() // 设置当前所选的收藏夹为未保存
{
    if(!isUserAction) // 非用户行为不执行
        return;
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0) // 未选中不执行
        return;

    edit[curRow].second = STATUS::UNCHANGED;
    ui->listWidget_collections->item(curRow)->setText(edit[curRow].first.first.foldname + "(未保存)");
    // 加粗字体
    QFont font = ui->listWidget_collections->item(curRow)->font();
    font.setBold(true);
    ui->listWidget_collections->item(curRow)->setFont(font);
}

void EditCollect::ClickCollection()
{
    int curRow = ui->listWidget_collections->currentRow();
    ui->lineEdit_name->setText(edit[curRow].first.first.foldname);
    ui->lineEdit_post->setText(edit[curRow].first.first.coverFileName);
    this->isUserAction = false;
    ui->textEdit_info->setText(edit[curRow].first.first.info);
    this->isUserAction = true;


    fillFullContain(curRow);
}

void EditCollect::button_add() // 在当前行创建一个新文件夹
{
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0)
        curRow = 0;

    QPair<CollectionInfo, QStringList> newCollection;
    newCollection.first.foldname = "未命名";
    QDateTime now = QDateTime::currentDateTime();
    newCollection.first.createTime = now.toSecsSinceEpoch();
    newCollection.first.info = "暂无简介";
    newCollection.first.type = COLLEC_TYPE::custom;

    edit.insert(curRow, QPair<QPair<CollectionInfo, QStringList>,int>(newCollection, STATUS::UNSAVED));
    updateCollectList();

    // 添加后“光标”应该放在：
    ui->listWidget_collections->setCurrentRow(curRow);
    ClickCollection();
}

void EditCollect::button_del() // 将当前行标记为待删除
{
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0) // 未选中不执行删除
        return;

    edit.remove(curRow);
    updateCollectList();

    // 删除后“光标”应该放在：
    if(curRow == ui->listWidget_collections->count())  // 后面不在有了光标需要前移
        curRow--;
    if(ui->listWidget_collections->count() == 0)  // 全部删完了结束本次操作
        return;

    ui->listWidget_collections->setCurrentRow(curRow);
    ClickCollection();
}

void EditCollect::Click_Undo()
{
    edit.clear();
    ui->lineEdit_name->clear();
    ui->lineEdit_post->clear();
    ui->textEdit_info->clear();
    ui->listWidget_collections->clear();


    for(int i = 0; i<back.size(); i++)
    {
        edit.push_back(QPair<QPair<CollectionInfo, QStringList>,int>(back[i], STATUS::UNCHANGED));
        ui->listWidget_collections->addItem(back[i].first.foldname);
    }

    if(edit.size()>0)
    {
        ui->listWidget_collections->setCurrentRow(0);
        ClickCollection();
    }
}

void EditCollect::ClickSave()
{
    QVector<QPair<CollectionInfo, QStringList>> toUpdate; // 需要修改的，就是saved
    QSet<quint64> toDelete; // 需要删除的，就是原本在back，在edit中找不到的
    QSet<quint64> remain;

    for(int i = 0; i < edit.size(); i++)
    {
        if(edit[i].first.first.id != 0)       // 不修改的
            remain.insert(edit[i].first.first.id);
        if(edit[i].second == STATUS::SAVED)   // 需要修改的
            toUpdate.push_back(edit[i].first);
    }

    for(int i = 0; i < back.size(); i++)
        if(!remain.contains(back[i].first.id)) // 需要删除的
            toDelete.insert(back[i].first.id);

    emit UPDATE_COLLECTIONS(toUpdate, toDelete);
    this->hide();
}

void EditCollect::ClickExit()
{
    this->hide();
}

void EditCollect::fillFullPostPath()
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFiles); // 单个存在的文件
    fileDialog.setNameFilter("ALL Post (*.png *.jfif *.jpg)"); // 设置过滤器
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
                            + C::Path[C::PathType::Post] + "/" + Finfo.fileName();

        sourceFile.copy(destPath);
        ui->lineEdit_post->setText(Finfo.fileName());
    }
    SetCurUnsaved();
}

void EditCollect::fillFullContain(int curRow)
{
    ui->tableWidget_contain->clear();
    ui->tableWidget_contain->setColumnCount(3);
    ui->tableWidget_contain->setRowCount(edit[curRow].first.second.size()); // id, name, author
    ui->tableWidget_contain->setHorizontalHeaderLabels({"编号","名称","作者"});
    for(int i = 0; i < edit[curRow].first.second.size(); i++)
    {
        QString tmpId = edit[curRow].first.second[i];
        // id
        QTableWidgetItem* newItem = new QTableWidgetItem(tmpId);
        ui->tableWidget_contain->setItem(i, 0, newItem);
        // name
        QTableWidgetItem* newName = new QTableWidgetItem(source[tmpId].name);
        ui->tableWidget_contain->setItem(i, 1, newName);
        // author
        QTableWidgetItem* newAuthor = new QTableWidgetItem(source[tmpId].author);
        ui->tableWidget_contain->setItem(i, 2, newAuthor);
    }

    // 设置列宽尽可能小
    ui->tableWidget_contain->resizeColumnsToContents();
}

void EditCollect::ClickSaveTmp()
{
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0) // 未选中不操作
        return;

    edit[curRow].second = STATUS::SAVED;
    ui->listWidget_collections->item(curRow)->setText(ui->lineEdit_name->text());
    edit[curRow].first.first.coverFileName = ui->lineEdit_post->text();
    edit[curRow].first.first.foldname = ui->lineEdit_name->text();
    edit[curRow].first.first.info = ui->textEdit_info->toPlainText();

    QFont font = ui->listWidget_collections->item(curRow)->font();
    font.setBold(false);
    ui->listWidget_collections->item(curRow)->setFont(font);
}

void EditCollect::ContainSaveTmp(const QStringList& ids)
{
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0) // 未选中不操作
        return;
    edit[curRow].first.second = ids;
    fillFullContain(curRow);
}

void EditCollect::ClickEditContain()
{
    int curRow = ui->listWidget_collections->currentRow();
    if(curRow < 0) // 未选中不操作
        return;

    selectPage->input(source, edit[curRow].first.second);
    selectPage->show();
    SetCurUnsaved();
}
