#ifndef DATALOOP_H
#define DATALOOP_H

#include <QObject>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;
QT_END_NAMESPACE

class DataLoop : public QObject
{
    Q_OBJECT
public:
    explicit DataLoop(QObject *parent = nullptr, const QString &entry = "", const QString &token = "");

    QNetworkReply *testConnection();
    QNetworkReply *reqPieceIDs(const QString &partIdentifier);
    QNetworkReply *reqMeasureObjectByPieceID(const QString &pieceID);

    inline void setEntry(const QString &entry) { m_entry = entry; }
    inline void setToken(const QString &token) { m_token = token; }
    inline bool isConnect() { return m_bConnect; }

public slots:
    void onRequestFinished(QNetworkReply *reply);

signals:
    void dlConnection(bool connected);
    void dlFinished(QNetworkReply *reply);

private:
    bool initialize();

private:
    QString m_entry;
    QString m_token;
    bool m_bConnect = false;

    QNetworkAccessManager *m_pAccessManger = nullptr;
    QNetworkRequest *m_pRequest = nullptr;
};

#endif // DATALOOP_H
