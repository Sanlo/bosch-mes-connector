#ifndef XOPCONWRITER_H
#define XOPCONWRITER_H

#include <QXmlStreamWriter>

class XopconWriter
{
public:
    enum EventType {
        PartRecevied,
        PartProcessed,
    };

    explicit XopconWriter(const QString partId, XopconWriter::EventType event = EventType::PartRecevied);
    bool writeXmlData(QIODevice *device);
    // Getter
    inline QString lineNo() const { return this->xmlLineNo; }
    inline QString statNo() const { return this->xmlStatNo; }
    inline QString statIdx() const { return this->xmlStatIdx; }
    inline QString fuNo() const { return this->xmlFuNo; }
    inline QString workPos() const { return this->xmlWorkPos; }
    inline QString toolPos() const { return this->xmlToolPos; }
    inline QString processNo() const { return this->xmlProcessNo; }
    inline QString processName() const { return this->xmlProcessName; }
    inline QString application() const { return this->xmlApplication; }
    // Setter
    inline void setLineNo(const QString &str) { xmlLineNo = str; }
    inline void setStatNo(const QString &str) { xmlStatNo = str; }
    inline void setStatIdx(const QString &str) { xmlStatIdx = str; }
    inline void setFuNo(const QString &str) { xmlFuNo = str; }
    inline void setWorkPos(const QString &str) { xmlWorkPos = str; }
    inline void setToolPos(const QString &str) { xmlToolPos = str; }
    inline void setProcessNo(const QString &str) { xmlProcessNo = str; }
    inline void setProcessName(const QString &str) { xmlProcessName = str; }
    inline void setApplication(const QString &str) { xmlApplication = str; }

    inline void setMeasureData(QStringList objName, QStringList controlName, QList<double> measureValue)
    {
        m_objname = objName;
        m_controlName = controlName;
        m_measureValue = measureValue;
    }

private:
    void writeItem();
    void writeHeader();
    void writeEvent();
    void writeBody();

    QXmlStreamWriter xml;

    QString partId;
    EventType eventType;

    // xml header.location
    QString xmlLineNo;
    QString xmlStatNo;
    QString xmlStatIdx;
    QString xmlFuNo;
    QString xmlWorkPos;
    QString xmlToolPos;
    QString xmlProcessNo;
    QString xmlProcessName;
    QString xmlApplication;

    // measurement items
    QStringList m_objname;
    QStringList m_controlName;
    QList<double> m_measureValue;
};

#endif // XOPCONWRITER_H
