#ifndef XOPCONWRITER_H
#define XOPCONWRITER_H

#include <QXmlStreamWriter>

class XopconWriter
{
public:
    explicit XopconWriter();
    bool writeXmlData(QIODevice *device);

private:
    void writeItem();
    void writeHeader();
    void writeEvent();
    void writeBody();

    QXmlStreamWriter xml;

    QString eventName;
    QString partId;

};

#endif // XOPCONWRITER_H
