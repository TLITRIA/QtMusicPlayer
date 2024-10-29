#ifndef CONFIGXML_H
#define CONFIGXML_H
#include "common.h"

#include <QObject>
#include <QtXml>
/*
专门处理客户端xml
*/
class ConfigXml : public QObject
{
    Q_OBJECT
public:
    static ConfigXml& getInstance() {
        static ConfigXml instance;
        return instance;
    }

    bool LoadXMLPath(const QString& Path);
    QDomElement ReadFromXML();
    bool WriteToXML();

    // task
    bool updateAccount(QCStrRef ID, QCStrRef userName, QCStrRef HeadIconFilePath,
                        bool ifAuto, bool ifRempwd, int* errorCode);
    bool getAccountMessage(QCStrRef ID, QString* userName, QString* iconFileName);
    bool setAccountLike(QCStrRef m_id, QCStrRef u_id, bool val);
    bool getAutoLogAccount(QString* id);

    static bool WriteToXML(const QDomDocument& doc, const QString& filePath);
    static bool createFile(QCStrRef filePath);
    static bool createNullLoginFile();
    static bool createNullSourceFile();
    static bool createNullLocalConifg();
private:
    ConfigXml() {
        hasPath = false;
    } // 私有构造函数，确保只能通过 getInstance() 获取对象
    ConfigXml(const ConfigXml&) = delete; // 禁止拷贝构造
    void operator=(const ConfigXml&) = delete; // 禁止赋值操作

    bool hasPath;
    QString filePath;
    QDomDocument doc;
};

#endif // CONFIGXML_H
