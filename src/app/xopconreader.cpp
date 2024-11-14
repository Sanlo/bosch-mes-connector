#include <QDebug>

#include "xopconreader.h"

using namespace Qt::StringLiterals;

XopconReader::XopconReader() {}

bool XopconReader::read(QIODevice *device) {
    xml.setDevice(device);

    if(xml.readNextStartElement()) {
        if(xml.name() == "root"_L1){
            qDebug() << xml.name().toString();
        }

        Q_ASSERT(xml.isStartElement() && xml.name() == "root"_L1);

        while (xml.readNextStartElement()) {
            if (xml.name() == "header"_L1) {
                qDebug() << xml.name().toString();
                readHeader();
            }
            else if (xml.name() == "event"_L1) {
                qDebug() << xml.name().toString();
                readEvent();
            }
            else if (xml.name() == "body"_L1) {
                qDebug() << xml.name().toString();
                readBody();
            }
            else{
                qDebug() << xml.name().toString();
                xml.skipCurrentElement();
            }
        }
    }

    return !xml.error();
}

QString XopconReader::errorString() const {
    return QObject::tr("%1\nLine %2, column %3")
                .arg(xml.errorString())
                .arg(xml.lineNumber())
                .arg(xml.columnNumber());
}

void XopconReader::readHeader() {

    Q_ASSERT(xml.isStartElement() && xml.name() == "header"_L1);
    xml.skipCurrentElement();
}

void XopconReader::readEvent() {
    Q_ASSERT(xml.isStartElement() && xml.name() == "event"_L1);
    xml.skipCurrentElement();

}

void XopconReader::readBody() {
    Q_ASSERT(xml.isStartElement() && xml.name() == "body"_L1);

    while(xml.readNextStartElement()) {
        if(xml.name() == "items"_L1){
            // TODO: Read Items
            xml.skipCurrentElement();

        } else if(xml.name() == "structs"_L1) {
            // TODO: Read structs
            xml.readNextStartElement();
            if(xml.name() == "workPart"_L1) {
                QString changeOver = xml.attributes().value("changeOver"_L1).toString();
                QString identifier = xml.attributes().value("identifier"_L1).toString();
                QString nextProcessNo = xml.attributes().value("nextProcessNo"_L1).toString();
                QString partForStation = xml.attributes().value("partForStation"_L1).toString();
                QString typeNo = xml.attributes().value("typeNo"_L1).toString();
                QString typeVar = xml.attributes().value("typeVar"_L1).toString();
                QString workingCode = xml.attributes().value("workingCode"_L1).toString();
                QString batch = xml.attributes().value("batch"_L1).toString();

                qDebug() <<"Server Reply for Work Part:\n"<<changeOver<<identifier<<nextProcessNo<<partForStation<<typeNo<<typeVar<<workingCode<<batch;
        } else {
            xml.skipCurrentElement();
        }
    }
    }

}

