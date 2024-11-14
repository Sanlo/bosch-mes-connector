#ifndef XOPCONREADER_H
#define XOPCONREADER_H

#include <QXmlStreamReader>

QT_BEGIN_NAMESPACE

QT_END_NAMESPACE


class XopconReader
{
public:
    XopconReader();

    bool read(QIODevice *device);
    QString errorString() const;

private:
    void readXOPCON();
    void readHeader();
    void readEvent();
    void readBody();

    QXmlStreamReader xml;

};

#endif // XOPCONREADER_H
