#ifndef XOPCONREADER_H
#define XOPCONREADER_H

#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE

struct Norminal
{
    Norminal(quint32 objIdx_, double_t lTol_, double_t uTol_)
        : objIdx(objIdx_)
        , lTol(lTol_)
        , uTol(uTol_)
    {}

    quint32 objIdx;
    double_t lTol;
    double_t uTol;
};

class XopconReader
{
public:
    XopconReader();

    bool read(QIODevice *device);
    QString errorString() const;

    bool isValidProcess(const QString &process) const;

    // Getter
    inline QString eventId() const { return this->xmlEventId; };
    inline QString eventName() const { return this->xmlEventName; };
    inline QString lineNo() const { return this->xmlLineNo; };
    inline QString statNo() const { return this->xmlStatNo; };
    inline QString statIdx() const { return this->xmlStatIdx; };
    inline QString processNo() const { return this->xmlProcessNo; };
    inline QString application() const { return this->xmlApplication; };
    inline QString timeStamp() const { return this->xmlTimeStamp; };
    inline QString partIdentifier() const { return this->xmlPartIdentifier; };
    inline quint32 returnCode() const { return this->xmlReturnCode; };
    inline QString nextProcessNo() const { return this->xmlNextProcessNo; };
    inline QString partForStation() const { return this->xmlPartForStation; };
    inline QString typeNo() const { return this->xmlTypeNo; };
    inline QString typeVar() const { return this->xmlTypeVar; };
    inline QList<Norminal> norminalArray() const { return this->xmlNorminal; };

private:
    void readXOPCON();
    void readHeader();
    void readEvent();
    void readBody();
    void readBodyItems();

    QXmlStreamReader xml;
    const QStringList availableProcesses{"38213620", "38213680", "38213685"};

    // xml header
    QString xmlEventId;
    QString xmlEventName;
    // xml header.location
    QString xmlLineNo;
    QString xmlStatNo;
    QString xmlStatIdx;
    QString xmlProcessNo;
    QString xmlApplication;
    QString xmlTimeStamp;
    // xml event.partReceived
    QString xmlPartIdentifier;
    quint32 xmlReturnCode = {0};
    // xml body.structs.workPart
    QString xmlNextProcessNo;
    QString xmlPartForStation;
    QString xmlTypeNo;
    QString xmlTypeVar;
    QList<Norminal> xmlNorminal;
};

#endif // XOPCONREADER_H
