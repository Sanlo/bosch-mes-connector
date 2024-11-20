
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "dataloop.h"

DataLoop::DataLoop(QObject *parent, const QString &entry, const QString &token)
    : QObject(parent)
    , m_entry(entry)
    , m_token(token)
    , m_pAccessManger(new QNetworkAccessManager(this))
    , m_pRequest(new QNetworkRequest())
{
    initialize();
}

QNetworkReply *DataLoop::testConnection()
{
    m_pRequest->setUrl(QUrl(QString("%1/Workspaces").arg(m_entry)));

    return m_pAccessManger->get(*m_pRequest);
}

QNetworkReply *DataLoop::reqPieceIDs(const QString &projectName)
{
    QUrl url(QString("%1/inspectorProjects?filter=name eq \'%2\'&expand=inspectorPieces").arg(m_entry, projectName));

    m_pRequest->setUrl(url);
    return m_pAccessManger->get(*m_pRequest);
}

QNetworkReply *DataLoop::reqMeasureObjectByPieceID(const QString &pieceID)
{
    if (pieceID.isEmpty())
        return nullptr;

    QUrl url(QString("%1/MeasurementObjects?$filter=inspectorPiece/id eq %2&$expand=controls").arg(m_entry, pieceID));
    m_pRequest->setUrl(url);
    return m_pAccessManger->get(*m_pRequest);
}

void DataLoop::onRequestFinished(QNetworkReply *reply)
{
    m_bConnect = (200 == reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()) ? true : false;

    emit dlConnection(m_bConnect);
}

bool DataLoop::initialize()
{
    m_pRequest->setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toLatin1());
    connect(m_pAccessManger, &QNetworkAccessManager::finished, this, &DataLoop::onRequestFinished);

    return true;
}
