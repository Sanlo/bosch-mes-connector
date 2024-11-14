#include <QtCore>

#include "xopconwriter.h"

using namespace Qt::StringLiterals;

XopconWriter::XopconWriter() {
    xml.setAutoFormatting(true);

    // FOR TESTING
    eventName = "partReceived";
    partId = "0050204241304335";
}

bool XopconWriter::writeXmlData(QIODevice *device) {
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

void XopconWriter::writeHeader() {

    xml.writeStartElement("header"_L1);
    xml.writeAttribute("version"_L1, "V2.x"_L1);
    xml.writeAttribute("eventId"_L1, "5238"_L1);
    xml.writeAttribute("eventName"_L1, eventName);

    // write location element
    xml.writeStartElement("location"_L1);
    QSettings clientSettings("MesConnector", "Client");
    xml.writeAttribute("lineNo"_L1, clientSettings.value("connection/mes/lineNo").toString());
    xml.writeAttribute("statNo"_L1, clientSettings.value("connection/mes/statNo").toString());
    xml.writeAttribute("statIdx"_L1, clientSettings.value("connection/mes/statIdx").toString());
    xml.writeAttribute("fuNo"_L1, clientSettings.value("connection/mes/fuNo").toString());
    xml.writeAttribute("workPos"_L1, clientSettings.value("connection/mes/workPos").toString());
    xml.writeAttribute("toolPos"_L1, clientSettings.value("connection/mes/toolPos").toString());
    xml.writeAttribute("processNo"_L1, clientSettings.value("connection/mes/processNo").toString());
    xml.writeAttribute("processName"_L1, clientSettings.value("connection/mes/processName").toString());
    xml.writeAttribute("application"_L1, clientSettings.value("connection/mes/application").toString());
    xml.writeEndElement();

    xml.writeEndElement();
}

void XopconWriter::writeEvent() {

    xml.writeStartElement("event"_L1);

    xml.writeStartElement(eventName);
    xml.writeAttribute("identifier"_L1, partId);
    xml.writeAttribute("typeNo"_L1, ""_L1);
    xml.writeAttribute("typeVar"_L1, ""_L1);
    xml.writeEndElement();

    xml.writeEndElement();
}

void XopconWriter::writeBody() {

    xml.writeStartElement("body"_L1);
    xml.writeEndElement();
}
