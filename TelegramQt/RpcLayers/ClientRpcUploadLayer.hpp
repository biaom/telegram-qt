/*
   Copyright (C) 2018 Alexander Akulich <akulichalexander@gmail.com>

   This file is a part of TelegramQt library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

 */

#ifndef TELEGRAM_CLIENT_RPC_UPLOAD_LAYER_HPP
#define TELEGRAM_CLIENT_RPC_UPLOAD_LAYER_HPP

#include "ClientRpcLayerExtension.hpp"
#include "TLTypes.hpp"

namespace Telegram {

class PendingRpcOperation;

namespace Client {

class UploadRpcLayer : public BaseRpcLayerExtension
{
    Q_OBJECT
public:
    explicit UploadRpcLayer(QObject *parent = nullptr);

    // Generated Telegram API declarations
    PendingRpcOperation *getCdnFile(const QByteArray &fileToken, quint32 offset, quint32 limit);
    PendingRpcOperation *getCdnFileHashes(const QByteArray &fileToken, quint32 offset);
    PendingRpcOperation *getFile(const TLInputFileLocation &location, quint32 offset, quint32 limit);
    PendingRpcOperation *getWebFile(const TLInputWebFileLocation &location, quint32 offset, quint32 limit);
    PendingRpcOperation *reuploadCdnFile(const QByteArray &fileToken, const QByteArray &requestToken);
    PendingRpcOperation *saveBigFilePart(quint64 fileId, quint32 filePart, quint32 fileTotalParts, const QByteArray &bytes);
    PendingRpcOperation *saveFilePart(quint64 fileId, quint32 filePart, const QByteArray &bytes);
    // End of generated Telegram API declarations
};

} // Client namespace

} // Telegram namespace

#endif // TELEGRAM_CLIENT_RPC_UPLOAD_LAYER_HPP