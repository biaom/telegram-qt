/*
   Copyright (C) 2018 

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

#ifndef PHONE_OPERATION_FACTORY_HPP
#define PHONE_OPERATION_FACTORY_HPP

#include "RpcOperationFactory.hpp"
#include "ServerRpcOperation.hpp"

#include <QObject>

namespace Telegram {

namespace Server {

class PhoneRpcOperation : public RpcOperation
{
    Q_OBJECT
    Q_DISABLE_COPY(PhoneRpcOperation)
public:
    explicit PhoneRpcOperation(RpcLayer *rpcLayer) : RpcOperation(rpcLayer) { }

    // Generated process methods
    bool processAcceptCall(RpcProcessingContext &context);
    bool processConfirmCall(RpcProcessingContext &context);
    bool processDiscardCall(RpcProcessingContext &context);
    bool processGetCallConfig(RpcProcessingContext &context);
    bool processReceivedCall(RpcProcessingContext &context);
    bool processRequestCall(RpcProcessingContext &context);
    bool processSaveCallDebug(RpcProcessingContext &context);
    bool processSetCallRating(RpcProcessingContext &context);
    // End of generated process methods

    // Generated run methods
    void runAcceptCall();
    void runConfirmCall();
    void runDiscardCall();
    void runGetCallConfig();
    void runReceivedCall();
    void runRequestCall();
    void runSaveCallDebug();
    void runSetCallRating();
    // End of generated run methods

    void start() override { callMember<>(this, m_runMethod); }

    using RunMethod = void (PhoneRpcOperation::*)();
    using ProcessingMethod = bool (PhoneRpcOperation::*)(RpcProcessingContext &);
    static ProcessingMethod getMethodForRpcFunction(TLValue function);
protected:
    void setRunMethod(RunMethod method);

    RunMethod m_runMethod = nullptr;

    // Generated RPC members
    TLFunctions::TLPhoneAcceptCall m_acceptCall;
    TLFunctions::TLPhoneConfirmCall m_confirmCall;
    TLFunctions::TLPhoneDiscardCall m_discardCall;
    TLFunctions::TLPhoneGetCallConfig m_getCallConfig;
    TLFunctions::TLPhoneReceivedCall m_receivedCall;
    TLFunctions::TLPhoneRequestCall m_requestCall;
    TLFunctions::TLPhoneSaveCallDebug m_saveCallDebug;
    TLFunctions::TLPhoneSetCallRating m_setCallRating;
    // End of generated RPC members
};

class PhoneOperationFactory : public RpcOperationFactory
{
public:
    RpcOperation *processRpcCall(RpcLayer *layer, RpcProcessingContext &context) override;
};

} // Server

} // Telegram

#endif // PHONE_OPERATION_FACTORY_HPP