#include "ClientBackend.hpp"
#include "ClientSettings.hpp"
#include "AccountStorage.hpp"
#include "ConnectionApi.hpp"
#include "ConnectionApi_p.hpp"
#include "ContactsApi.hpp"
#include "ContactsApi_p.hpp"
#include "ClientConnection.hpp"
#include "Client.hpp"
#include "ClientRpcLayer.hpp"
#include "DataStorage.hpp"
#include "MessagingApi.hpp"
#include "RpcError.hpp"
#include "Debug_p.hpp"
#include "UpdatesLayer.hpp"

#include "Operations/ClientHelpOperation.hpp"
#include "PendingRpcOperation.hpp"

// Generated low-level layer includes
#include "ClientRpcAccountLayer.hpp"
#include "ClientRpcAuthLayer.hpp"
#include "ClientRpcBotsLayer.hpp"
#include "ClientRpcChannelsLayer.hpp"
#include "ClientRpcContactsLayer.hpp"
#include "ClientRpcHelpLayer.hpp"
#include "ClientRpcLangpackLayer.hpp"
#include "ClientRpcMessagesLayer.hpp"
#include "ClientRpcPaymentsLayer.hpp"
#include "ClientRpcPhoneLayer.hpp"
#include "ClientRpcPhotosLayer.hpp"
#include "ClientRpcStickersLayer.hpp"
#include "ClientRpcUpdatesLayer.hpp"
#include "ClientRpcUploadLayer.hpp"
#include "ClientRpcUsersLayer.hpp"
// End of generated low-level layer includes

#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(c_clientBackendCategory, "telegram.client.backend", QtWarningMsg)

namespace Telegram {

namespace Client {

Backend::Backend(Client *parent) :
    QObject(parent),
    m_client(parent)
{
    Telegram::initialize();

    Backend *b = this;
    BaseRpcLayerExtension::RpcProcessingMethod rpcProcessMethod = [b](PendingRpcOperation *operation) mutable {
        qCDebug(c_clientBackendCategory) << "Default processing for" << operation
                                         << TLValue::firstFromArray(operation->requestData());
        Connection *connection = b->getDefaultConnection();
        if (!connection) {
            qCWarning(c_clientBackendCategory) << "No connection for processMethod";
            return;
        }
        connection->rpcLayer()->sendRpc(operation);
    };

    // Generated low-level layer initialization
    m_accountLayer = new AccountRpcLayer(this);
    m_accountLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_authLayer = new AuthRpcLayer(this);
    m_authLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_botsLayer = new BotsRpcLayer(this);
    m_botsLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_channelsLayer = new ChannelsRpcLayer(this);
    m_channelsLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_contactsLayer = new ContactsRpcLayer(this);
    m_contactsLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_helpLayer = new HelpRpcLayer(this);
    m_helpLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_langpackLayer = new LangpackRpcLayer(this);
    m_langpackLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_messagesLayer = new MessagesRpcLayer(this);
    m_messagesLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_paymentsLayer = new PaymentsRpcLayer(this);
    m_paymentsLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_phoneLayer = new PhoneRpcLayer(this);
    m_phoneLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_photosLayer = new PhotosRpcLayer(this);
    m_photosLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_stickersLayer = new StickersRpcLayer(this);
    m_stickersLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_updatesLayer = new UpdatesRpcLayer(this);
    m_updatesLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_uploadLayer = new UploadRpcLayer(this);
    m_uploadLayer->setRpcProcessingMethod(rpcProcessMethod);
    m_usersLayer = new UsersRpcLayer(this);
    m_usersLayer->setRpcProcessingMethod(rpcProcessMethod);
    // End of generated low-level layer initialization

    m_connectionApi = new ConnectionApi(this);
    ClientApiPrivate::get(m_connectionApi)->setBackend(this);

    m_contactsApi = new ContactsApi(this);
    ClientApiPrivate::get(m_contactsApi)->setBackend(this);

    m_messagingApi = new MessagingApi(this);
    ClientApiPrivate::get(m_messagingApi)->setBackend(this);

    m_updatesApi = new UpdatesInternalApi(this);
    m_updatesApi->setBackend(this);
}

bool Backend::isSignedIn() const
{
    return m_connectionApi->isSignedIn();
}

PendingOperation *Backend::getDcConfig()
{
    if (m_getConfigOperation) {
        if (!m_getConfigOperation->isFailed()) {
            return m_getConfigOperation;
        }
        // Delete the failed operation
        m_getConfigOperation->deleteLater();
        m_getConfigOperation = nullptr;
    }

    HelpOperation *op = new HelpOperation(this);
    op->setObjectName(QLatin1String("Backend/GetDcConfig"));
    op->setBackend(this);
    op->setRunMethod(&HelpOperation::requestDcConfig);
    op->startLater();
    m_getConfigOperation = op;
    connect(op, &PendingOperation::finished, this, &Backend::onGetDcConfigurationFinished);
    return m_getConfigOperation;
}

PendingOperation *Backend::sync()
{
    if (!m_contactsApi->selfContactId()) {
        ContactsApiPrivate *privateApi = ContactsApiPrivate::get(m_contactsApi);
        return privateApi->sync();
    }
    PendingOperation *op = new SucceededPendingOperation(this);
    op->setObjectName(QLatin1String("Backend/Sync"));
    return op;
}

Connection *Backend::getDefaultConnection()
{
    ConnectionApiPrivate *privateApi = ConnectionApiPrivate::get(m_connectionApi);
    return privateApi->getDefaultConnection();
}

Connection *Backend::ensureConnection(const ConnectionSpec &dcSpec)
{
    ConnectionApiPrivate *privateApi = ConnectionApiPrivate::get(m_connectionApi);
    return privateApi->ensureConnection(dcSpec);
}

void Backend::onGetDcConfigurationFinished(PendingOperation *operation)
{
    if (!operation->isSucceeded()) {
        qCDebug(c_clientBackendCategory) << Q_FUNC_INFO << "Unable to get dc configuration" << operation->errorDetails();
        return;
    }
}

bool Backend::syncAccountToStorage()
{
    ConnectionApiPrivate *privateApi = ConnectionApiPrivate::get(m_connectionApi);
    Connection *connection = privateApi->mainConnection();
    if (!connection) {
        return false;
    }
    switch (connection->status()) {
    case Connection::Status::HasDhKey:
    case Connection::Status::Signed:
        break;
    default:
        return false;
    }
    m_accountStorage->setAuthKey(connection->authKey());
    m_accountStorage->setAuthId(connection->authId());
    m_accountStorage->setDcInfo(connection->dcOption());
    m_accountStorage->setDeltaTime(connection->deltaTime());
    m_accountStorage->setSessionData(connection->rpcLayer()->sessionId(),
                                     connection->rpcLayer()->contentRelatedMessagesNumber());
    m_accountStorage->sync();
    return true;
}

} // Client namespace

} // Telegram namespace
