#include <QtCore>

#include "xopconwriter.h"

using namespace Qt::StringLiterals;

XopconWriter::XopconWriter(const QString partId, XopconWriter::EventType event)
    : partId(partId)
    , eventType(event)
{
    xml.setAutoFormatting(true);
}

bool XopconWriter::writeXmlData(QIODevice *device)
{
    xml.setDevice(device);

    xml.writeStartDocument();
    xml.writeStartElement("root"_L1);

    writeHeader();

    writeEvent();

    writeBody();

    xml.writeEndElement();

    xml.writeEndDocument();

    return true;
}

void XopconWriter::writeItem() {}

void XopconWriter::writeHeader()
{
    // write header and attribute
    xml.writeStartElement("header"_L1);
    xml.writeAttribute("version"_L1, "V2.x"_L1);

    xml.writeAttribute("eventId"_L1, "5238"_L1);
    if (eventType == EventType::PartRecevied) {
        xml.writeAttribute("eventName"_L1, "partReceived"_L1);
    } else if (eventType == EventType::PartProcessed) {
        xml.writeAttribute("eventName"_L1, "partProcessed"_L1);
    }
    xml.writeAttribute("contentType"_L1, "3"_L1);
    xml.writeAttribute("timeStamp"_L1, QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss.zzz+08:00"));

    // write location and attribue
    xml.writeStartElement("location"_L1);
    QSettings clientSettings("MesConnector", "Client");
    xml.writeAttribute("lineNo"_L1, lineNo());
    xml.writeAttribute("statNo"_L1, statNo());
    xml.writeAttribute("statIdx"_L1, statIdx());
    xml.writeAttribute("fuNo"_L1, fuNo());
    xml.writeAttribute("workPos"_L1, workPos());
    xml.writeAttribute("toolPos"_L1, toolPos());
    xml.writeAttribute("processNo"_L1, processNo());
    xml.writeAttribute("processName"_L1, processName());
    xml.writeAttribute("application"_L1, application());
    xml.writeEndElement();

    xml.writeEndElement();
}

void XopconWriter::writeEvent()
{
    xml.writeStartElement("event"_L1);

    if (eventType == EventType::PartRecevied) {
        xml.writeStartElement("partReceived"_L1);
    } else if (eventType == EventType::PartProcessed) {
        xml.writeStartElement("partProcessed"_L1);
    }

    xml.writeAttribute("identifier"_L1, partId);
    xml.writeAttribute("typeNo"_L1, ""_L1);
    xml.writeAttribute("typeVar"_L1, ""_L1);
    xml.writeEndElement();

    xml.writeEndElement();
}

void XopconWriter::writeBody()
{
    xml.writeStartElement("body"_L1);

    if (m_objname.size()) {
        xml.writeStartElement("items"_L1);
        for (int i = 0; i < m_objname.size(); ++i) {
            xml.writeStartElement("item"_L1);
            xml.writeAttribute("name"_L1, m_objname.at(i));
            xml.writeAttribute("control"_L1, m_controlName.at(i));
            xml.writeAttribute("value"_L1, QString("%1").arg(m_measureValue.at(i)));
            xml.writeEndElement();
        }
        xml.writeEndElement();
    }

    xml.writeEndElement();
}
