#ifndef XOPCONREADER_H
#define XOPCONREADER_H

#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

class XopconReader
{
public:
    enum EventType {
        PartRecevied,
        PartProcessed,
    };

    explicit XopconReader();

    bool read(QIODevice *device);
    QString errorString() const;

    // Getter
    inline QString eventId() const { return this->xmlEventId; };
    inline QString eventName() const { return this->xmlEventName; };
    inline QString lineNo() const { return this->xmlLineNo; };
    inline QString statNo() const { return this->xmlStatNo; };
    inline QString statIdx() const { return this->xmlStatIdx; };
    inline QString processNo() const { return this->xmlProcessNo; };
    inline QString application() const { return this->xmlApplication; };
    inline QString partIdentifier() const { return this->xmlPartIdentifier; };
    inline quint32 returnCode() const { return this->xmlReturnCode; };
    inline QString partForStation() const { return this->xmlPartForStation; };
    inline QString typeNo() const { return this->xmlTypeNo; };
    inline QString typeVar() const { return this->xmlTypeVar; };

private:
    void readXOPCON();
    void readHeader();
    void readEvent();
    void readBody();

    QXmlStreamReader xml;

    // xml header
    QString xmlEventId;
    QString xmlEventName;
    // xml header.location
    QString xmlLineNo;
    QString xmlStatNo;
    QString xmlStatIdx;
    QString xmlProcessNo;
    QString xmlApplication;
    // xml event.partReceived
    QString xmlPartIdentifier;
    quint32 xmlReturnCode = {0};
    // xml body.structs.workPart
    QString xmlPartForStation;
    QString xmlTypeNo;
    QString xmlTypeVar;
};

#endif // XOPCONREADER_H
