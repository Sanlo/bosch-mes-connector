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

    const QString format = "yyyy-MM-ddTHH:mm:ss.zzzzttt";
    const QString dateTime = QDateTime::currentDateTime().toString(format);
    xml.writeAttribute("timeStamp"_L1, dateTime);

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

    xml.writeStartElement(EventType::PartRecevied == eventType ? "partReceived"_L1
                                                               : "partProcessed"_L1);

    xml.writeAttribute("identifier"_L1, partId);

    if (eventType == EventType::PartRecevied) {
        xml.writeAttribute("typeNo"_L1, xmlPartTypeNo);
        xml.writeAttribute("typeVar"_L1, xmlPartTypeVar);
    }

    xml.writeEndElement();

    xml.writeEndElement();
}

void XopconWriter::writeBody()
{
    xml.writeStartElement("body"_L1);

    if (EventType::PartProcessed == eventType) {
        writeBodyStructArray();
        writeBodyStructs();
        writeBodyItems();
    }

    xml.writeEndElement();
}

void XopconWriter::writeBodyStructs()
{
    xml.writeStartElement("structs"_L1);
    xml.writeStartElement("resHead"_L1);

    xml.writeAttribute("result"_L1, QString::number(xmlResult));
    xml.writeAttribute("typeNo"_L1, xmlPartTypeNo);
    xml.writeAttribute("typeVar"_L1, xmlPartTypeVar);
    xml.writeAttribute("nioBits"_L1, QString::number(xmlNoiBits));

    xml.writeEndElement();
    xml.writeEndElement();
}

void XopconWriter::writeBodyStructArray()
{
    // write array header
    xml.writeStartElement("structArrays");
    xml.writeStartElement("array");
    xml.writeAttribute("name", "processRealData");

    xml.writeStartElement("structDef");
    xml.writeStartElement("item");
    xml.writeAttribute("name", "name");
    xml.writeAttribute("dataType", "8");
    xml.writeEndElement();

    xml.writeStartElement("item");
    xml.writeAttribute("name", "value");
    xml.writeAttribute("dataType", "4");
    xml.writeEndElement();

    xml.writeStartElement("item");
    xml.writeAttribute("name", "result");
    xml.writeAttribute("dataType", "2");
    xml.writeEndElement();

    xml.writeStartElement("item");
    xml.writeAttribute("name", "loLim");
    xml.writeAttribute("dataType", "4");
    xml.writeEndElement();

    xml.writeStartElement("item");
    xml.writeAttribute("name", "upLim");
    xml.writeAttribute("dataType", "4");
    xml.writeEndElement();

    xml.writeStartElement("item");
    xml.writeAttribute("name", "checkType");
    xml.writeAttribute("dataType", "2");
    xml.writeEndElement();
    xml.writeEndElement();

    xml.writeStartElement("values");

    // sort norminal list

    std::sort(xmlNorminal.begin(), xmlNorminal.end(), [](const Norminal a, const Norminal b) {
        return a.objIdx < b.objIdx;
    });

    quint32 res;
    quint32 base = 1;
    QString itemName;
    double averageAzuXMessung1{0.0f};
    double averageAzuXMessung2{0.0f};
    for (int i = 0; i < m_objname.size(); ++i) {
        xml.writeStartElement("item");

        // validate every measured items
        double measured = m_measureValue.at(i);
        if (xmlNorminal.at(i).lTol < measured && xmlNorminal.at(i).uTol > measured) {
            res = 1;
        } else {
            xmlNoiBits = xmlNoiBits + (base << i);
            res = 2;
        }
        // average distance compute
        if (i < 4) {
            averageAzuXMessung1 += measured;
        } else if (i >= 4 && i < 8) {
            averageAzuXMessung2 += measured;
        }

        // store TotalHeightStack to List
        itemName = QString("Result.HeightMeasurement.Distance.%1")
                       .arg(m_objname.at(i).split(u' ').join(""));
        if (i > 10) {
            m_TotalHeightStacks.append(new TotalHeightStack(itemName, measured));
        }

        xml.writeAttribute("name", itemName);
        xml.writeAttribute("value", QString::number(measured));
        xml.writeAttribute("result", QString::number(res));
        xml.writeAttribute("loLim", QString::number(xmlNorminal.at(i).lTol));
        xml.writeAttribute("upLim", QString::number(xmlNorminal.at(i).uTol));
        xml.writeAttribute("checkType", "5");

        xml.writeEndElement();
    }

    //
    xml.writeStartElement("item");
    xml.writeAttribute("name", QString("Result.HeightMeasurement.Distance.AToXMeasurement"));
    averageAzuXMessung1 = averageAzuXMessung1 / 4.0;
    xml.writeAttribute("value", QString::number(averageAzuXMessung1));
    res = (xmlNorminal.at(13).lTol < averageAzuXMessung1 && averageAzuXMessung1 < xmlNorminal.at(13).uTol) ? 1 : 2;
    xml.writeAttribute("result", QString::number(res));
    xml.writeAttribute("loLim", QString::number(xmlNorminal.at(13).lTol));
    xml.writeAttribute("upLim", QString::number(xmlNorminal.at(13).uTol));
    xml.writeAttribute("checkType", "5");

    xml.writeEndElement();
    xml.writeStartElement("item");
    xml.writeAttribute("name", QString("Result.HeightMeasurement.Distance.AToBMeasurement"));
    averageAzuXMessung2 = averageAzuXMessung2 / 4.0;
    xml.writeAttribute("value", QString::number(averageAzuXMessung2));
    res = (xmlNorminal.at(14).lTol < averageAzuXMessung2 && averageAzuXMessung2 < xmlNorminal.at(14).uTol) ? 1 : 2;
    xml.writeAttribute("result", QString::number(res));
    xml.writeAttribute("loLim", QString::number(xmlNorminal.at(14).lTol));
    xml.writeAttribute("upLim", QString::number(xmlNorminal.at(14).uTol));
    xml.writeAttribute("checkType", "5");

    xml.writeEndElement();

    // close values tag
    xml.writeEndElement();
    // close array tag
    xml.writeEndElement();
    // close structArrays tag
    xml.writeEndElement();

    if (0 != xmlNoiBits) {
        xmlResult = 2;
    }
}

void XopconWriter::writeBodyItems()
{
    Q_ASSERT(!m_TotalHeightStacks.isEmpty());
    xml.writeStartElement("items");
    foreach (const auto i, m_TotalHeightStacks) {
        xml.writeStartElement("item");
        xml.writeAttribute("name", i->name);
        xml.writeAttribute("value", QString::number(i->value));
        xml.writeAttribute("dataType", QString::number(i->dataType));
        xml.writeEndElement();
    }
    // close body items tag
    xml.writeEndElement();
}
