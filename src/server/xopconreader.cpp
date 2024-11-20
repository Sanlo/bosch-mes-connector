#include "xopconreader.h"

#include <QDebug>

using namespace Qt::StringLiterals;

XopconReader::XopconReader() {}

bool XopconReader::read(QIODevice *device)
{
    xml.setDevice(device);

    if (xml.readNextStartElement()) {
        if (xml.name() != "root"_L1) {
            qDebug() << xml.name().toString();
            return false;
        }

        Q_ASSERT(xml.isStartElement() && xml.name() == "root"_L1);

        while (xml.readNextStartElement()) {
            if (xml.name() == "header"_L1) {
                qDebug().noquote() << xml.name().toString() << QString().fill('=', 40);
                readHeader();

            } else if (xml.name() == "event"_L1) {
                qDebug().noquote() << xml.name().toString() << QString().fill('=', 40);
                readEvent();
            } else if (xml.name() == "body"_L1) {
                qDebug().noquote() << xml.name().toString() << QString().fill('=', 40);
                readBody();
            } else {
                qDebug().noquote() << xml.name().toString() << QString().fill('=', 40);
                xml.skipCurrentElement();
            }
        }
    }

    return !xml.error();
}

QString XopconReader::errorString() const {
    return QObject::tr("%1\nLine %2, column %3").arg(xml.errorString()).arg(xml.lineNumber()).arg(xml.columnNumber());
}

void XopconReader::readHeader() {
    Q_ASSERT(xml.isStartElement() && xml.name() == "header"_L1);

    // Read Header Attribute
    xmlEventId = xml.attributes().value("eventId").toString();
    xmlEventName = xml.attributes().value("eventName").toString();

    qDebug() << "eventId: " << xmlEventId << "eventName: " << xmlEventName;

    // Read Location Element
    while (xml.readNextStartElement()) {
        if (xml.name() == "location") {
            qDebug().noquote() << xml.name() << QString().fill('=', 40);
            xmlLineNo = xml.attributes().value("lineNo").toString();
            xmlStatNo = xml.attributes().value("statNo").toString();
            xmlStatIdx = xml.attributes().value("statIdx").toString();
            QString fuNo = xml.attributes().value("fuNo").toString();
            QString workPos = xml.attributes().value("workPos").toString();
            QString toolPos = xml.attributes().value("toolPos").toString();
            xmlProcessNo = xml.attributes().value("processNo").toString();
            QString processName = xml.attributes().value("processName").toString();
            xmlApplication = xml.attributes().value("application").toString();

            qDebug() << "lineNo: " << xmlLineNo << "statNo: " << xmlStatNo << "statIdx: " << xmlStatIdx
                     << "fuNo: " << fuNo << "workPos: " << workPos << "toolPos: " << toolPos
                     << "processNo: " << xmlProcessNo << "application: " << xmlApplication;
        } else {
            xml.skipCurrentElement();
        }
    }
    // Skip from header element
    xml.skipCurrentElement();
}

void XopconReader::readEvent() {
    Q_ASSERT(xml.isStartElement() && xml.name() == "event"_L1);
    while (xml.readNextStartElement()) {
        if (xml.name() == "result") {
            xmlReturnCode = xml.attributes().value("returnCode").toInt();
            qDebug() << "returnCode: " << xmlReturnCode;
        } else if (xml.name() == "partRecevied") {
            xmlPartIdentifier = xml.attributes().value("identifier").toString();
            qDebug() << "part identifier: " << xmlPartIdentifier;
        } else {
            xml.skipCurrentElement();
        }
    }
    // Skip from event element
    xml.skipCurrentElement();
}

void XopconReader::readBody() {
    Q_ASSERT(xml.isStartElement() && xml.name() == "body"_L1);
    while (xml.readNextStartElement()) {
        if (xml.name() == "items"_L1) {
            // TODO: Read Items
            xml.skipCurrentElement();
        } else if (xml.name() == "structs"_L1) {
            xml.readNextStartElement();
            if (xml.name() == "workPart"_L1) {
                QString changeOver = xml.attributes().value("changeOver"_L1).toString();
                QString identifier = xml.attributes().value("identifier"_L1).toString();
                QString nextProcessNo = xml.attributes().value("nextProcessNo"_L1).toString();
                xmlPartForStation = xml.attributes().value("partForStation"_L1).toString();
                xmlTypeNo = xml.attributes().value("typeNo"_L1).toString();
                xmlTypeVar = xml.attributes().value("typeVar"_L1).toString();
                QString workingCode = xml.attributes().value("workingCode"_L1).toString();
                QString batch = xml.attributes().value("batch"_L1).toString();

                qDebug().noquote() << "Server Reply for Work Part:\n"
                                   << changeOver << identifier << nextProcessNo << xmlPartForStation << xmlTypeNo
                                   << xmlTypeVar << workingCode << batch;
            } else {
                xml.skipCurrentElement();
            }
        }
    }
}
