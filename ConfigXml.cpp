#include "ConfigXml.h"
#include "common.h"
#include <QFile>
#include <QDebug>


bool ConfigXml::LoadXMLPath(const QString &Path)
{
    this->filePath = Path;
    hasPath = true;

    return true;
}

QDomElement ConfigXml::ReadFromXML()
{
    QDomElement root;
    if(!hasPath)
    {
        MYLOG<< "未加载文件路径";
        return root;
    }
    QFile file(filePath);
    if(file.open(QIODevice::ReadOnly) == false)
    {
        MYLOG << "xml 文件打开失败："<< filePath ;
        MYLOG << "错误信息" << file.errorString();
        return root;
    }

    auto Content = file.readAll();
    file.close();
    QString errorMsg;
    int row = 0, column = 0;
    // 解析xml文件内容
    if(doc.setContent(Content, &errorMsg, &row, &column) == false)
    {
        MYLOG<<filePath<<"xml 文件解析失败:"<<errorMsg;
        MYLOG<<"row:"<<row<<"column:"<<column;
        return root;
    }
    // MYLOG<<"xml 文件解析成功!";
    root = std::move(doc.documentElement());

    return root;
}

bool ConfigXml::WriteToXML()
{
    QFile file(filePath);
    if(file.open(QIODevice::WriteOnly) == false)
    {
        MYLOG<< "xml 文件打开失败："<< filePath;
        return false;
    }

    if (file.write(doc.toByteArray()) <= 0)
    {
        MYLOG << "Debug message at" << __FILE__ << ":" << __LINE__ <<"xml 文件写入失败!";
        file.close();
        return false;
    }
    file.close();
    return true;
}

bool ConfigXml::updateAccount(QCStrRef ID, QCStrRef userName, QCStrRef HeadIconFilePath, bool ifAuto, bool ifRempwd, int *errorCode)
{
    *errorCode = 0;
    this->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                      + C::Path[C::PathType::Config]
                      + C::File[C::FileType::LoginAccount]);
    QDomElement root = this->ReadFromXML();
    if(root.isNull())
        return false;
    // 用户
    QDomElement Accounts = root.firstChildElement("Accounts");

    if(!Accounts.isNull())
    {
        for(auto userNode = Accounts.firstChild();
             !userNode.isNull(); userNode = userNode.nextSibling())
        {
            if(userNode.toElement().attribute("ID") == ID || userNode.toElement().attribute("ID") == "") // 顺带清理错误数据
            {
                Accounts.removeChild(userNode);
            }
        }

    }
    QDomElement userNode =doc.createElement("account");
    userNode.setAttribute("ID", ID);
    userNode.setAttribute("name", userName);
    userNode.setAttribute("ICON", HeadIconFilePath);
    userNode.setAttribute("ifRempwd", ifRempwd?"true":"false");
    Accounts.appendChild(userNode);

    // 设置自动登陆
    QDomElement AutoLogin = root.firstChildElement("AutoLogin");
    QString AutoID = AutoLogin.attribute("ID");
    if(ifAuto) // 自动登陆
    {
        AutoLogin.setAttribute("ID", ID);
    }
    else
    {
        if(AutoID == ID)
            AutoLogin.setAttribute("ID", "");
    }

    this->WriteToXML();
    return true;
}

bool ConfigXml::getAccountMessage(QCStrRef ID, QString *userName, QString *iconFileName)
{
    if(userName == nullptr || iconFileName == nullptr)
    {
        MYLOG<<"NULPTR";
        return false;
    }
    this->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                      + C::Path[C::PathType::Config]
                      + C::File[C::FileType::LoginAccount]);
    QDomElement root = this->ReadFromXML();
    QDomNodeList list = root.elementsByTagName("account");
    for(int i=0; i<list.size(); i++)
    {
        if(list.at(i).toElement().attribute("ID") == ID)
        {
            *userName = list.at(i).toElement().attribute("name");
            *iconFileName = list.at(i).toElement().attribute("ICON");
            return true;
        }
    }
    return false;
}

bool ConfigXml::setAccountLike(QCStrRef m_id, QCStrRef u_id, bool val)
{
    if(u_id.isEmpty())
        return false;
    this->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                      + C::Path[C::PathType::Account]
                      + "/" + u_id + ".xml");
    QDomElement root = this->ReadFromXML();
    QDomNodeList tmpList =  root.firstChildElement("MusicSource").elementsByTagName("source");
    for(int i = 0; i < tmpList.size(); i++)
    {
        if(tmpList.at(i).toElement().text() == m_id)
        {
            if(!val)
            {
                root.firstChildElement("MusicSource").removeChild(tmpList.at(i));
                this->WriteToXML();
                return true;
            }
            else
            {
                return false;
            }
        }

    }
    if(val)
    {
        QDomText tmpText = doc.createTextNode(m_id);
        QDomNode tmpNode = doc.createElement("source");
        tmpNode.appendChild(tmpText);
        root.firstChildElement("MusicSource").appendChild(tmpNode);
        this->WriteToXML();
        return true;
    }
    return false; // 返回有无改动
}

bool ConfigXml::getAutoLogAccount(QString *id)
{
    if(id == nullptr)
        return false;
    this->LoadXMLPath(C::ClientSetting::getInstance().getCurDirPath()
                      + C::Path[C::PathType::Config]
                      + C::File[C::FileType::LoginAccount]);
    QDomElement root = this->ReadFromXML();
    QDomNodeList tmplist = root.elementsByTagName("AutoLogin");
    for (int i = 0; i < tmplist.count(); ++i) {
        if(tmplist.at(i).toElement().attribute("ID").isEmpty())
            return false;
        else
        {
            *id = tmplist.at(i).toElement().attribute("ID");
            return true;
        }
    }
    return false;
}

// STATIC
bool ConfigXml::WriteToXML(const QDomDocument& doc, const QString& filePath)
{
    bool ret = false;
    QFile file(filePath);
    file.open(QIODevice::WriteOnly);
    if (file.isOpen()) {
        QTextStream stream(&file);
        doc.save(stream, 4);
        ret = !stream.status(); // 检查stream的状态
        file.close();
    }
    /*
    if (ret) {
        MYLOG << "写入成功";
    } else {
        MYLOG << "写入失败";
    }*/
    return ret;
}

bool ConfigXml::createFile(QCStrRef filePath)
{
    // MYLOG<<filePath;
    QFile tmpFile;
    tmpFile.setFileName(filePath);
    if(tmpFile.exists())
        tmpFile.remove();
    bool ret = tmpFile.open(QIODevice::NewOnly);
    tmpFile.close();
    return ret;
}

bool ConfigXml::createNullLoginFile()
{
    QString tmpFilePath = C::ClientSetting::getInstance().getCurDirPath()
                       + C::Path[C::PathType::Config]
                       + C::File[C::FileType::LoginAccount];
    MYLOG<<"创建空文件："<<tmpFilePath;
    createFile(tmpFilePath);

    QFile tmpFile(tmpFilePath);

    QDomDocument doc;
    // xml头部格式
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    // xml根节点
    QDomElement root = doc.createElement("LoginAccount");
    doc.appendChild(root);

    // 用户
    QDomElement Accounts = doc.createElement("Accounts"); // 收藏的音乐资源
    root.appendChild(Accounts);


    // 自动登陆的用户
    QDomElement AutoLogin = doc.createElement("AutoLogin"); // 收藏的音乐资源
    root.appendChild(AutoLogin);
    bool ret = WriteToXML(doc, tmpFilePath);
    return ret;
}

bool ConfigXml::createNullSourceFile()
{
    QString tmpFilePath = C::ClientSetting::getInstance().getCurDirPath()
                          + C::Path[C::PathType::Config]
                          + C::File[C::FileType::LoginAccount];
    MYLOG<<"创建空文件："<<tmpFilePath;
    createFile(tmpFilePath);

    QFile tmpFile(tmpFilePath);

    QDomDocument doc;
    // xml头部格式
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    // // xml根节点
    // QDomElement root = doc.createElement("LoginAccount");
    // doc.appendChild(root);

    // // 用户
    // QDomElement Accounts = doc.createElement("Accounts"); // 收藏的音乐资源
    // root.appendChild(Accounts);

    // QDomElement AutoLogin = doc.createElement("AutoLogin"); // 收藏的音乐资源
    // root.appendChild(AutoLogin);

    return WriteToXML(doc, tmpFilePath);
}

bool ConfigXml::createNullLocalConifg()
{
    QString tmpFilePath = C::ClientSetting::getInstance().getCurDirPath()
                          + C::Path[C::PathType::Config]
                          + C::File[C::FileType::LocalConfig];
    MYLOG<<"创建空文件："<<tmpFilePath;
    QDomDocument doc;
    QDomProcessingInstruction instruction;
    instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
    doc.appendChild(instruction);

    QDomElement root = doc.createElement("Local");
    doc.appendChild(root);

    QDomElement Download = doc.createElement("Download");
    root.appendChild(Download);
    Download.setAttribute("PATH", "");

    QDomElement Style = doc.createElement("Style");
    root.appendChild(Style);
    Style.setAttribute("FILENAME", "");

    QDomElement Play = doc.createElement("Play");
    root.appendChild(Play);
    Play.setAttribute("VOLUMN", "50");

    return WriteToXML(doc, tmpFilePath);
}
