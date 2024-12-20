#ifndef XOPCONWRITER_H
#define XOPCONWRITER_H

#include <QXmlStreamWriter>
#include "xopconreader.h"

struct TotalHeightStack
{
    TotalHeightStack(const QString &name_, double_t value_, size_t dataType_ = 4)
        : name(name_)
        , value(value_)
        , dataType(dataType_)
    {}
    QString name;
    double_t value;
    size_t dataType;
};

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
    inline QString typeNo() const { return this->xmlPartTypeNo; }
    inline QString typeVar() const { return this->xmlPartTypeVar; }
    inline QList<Norminal> norminalArray() const { return this->xmlNorminal; }
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
    inline void setTypeNo(const QString &str) { xmlPartTypeNo = str; }
    inline void setTypeVar(const QString &str) { xmlPartTypeVar = str; }
    inline void setNorminalArray(const QList<Norminal> &list) { xmlNorminal = list; }

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
    void writeBodyStructs();
    void writeBodyStructArray();
    void writeBodyItems();

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
    // xml body.event
    QString xmlPartTypeNo;
    QString xmlPartTypeVar;

    // xml body.structArray
    QList<Norminal> xmlNorminal;

    // measurement items
    QStringList m_objname;
    QStringList m_controlName;
    QList<double> m_measureValue;

private:
    // xml body.structs for partProcessed
    quint32 xmlNoiBits{0};
    quint32 xmlResult{1};
    // measurement total height stack
    QList<TotalHeightStack *> m_TotalHeightStacks;
};

#endif // XOPCONWRITER_H
