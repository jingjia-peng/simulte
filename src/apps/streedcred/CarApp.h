//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef APPS_TCPSTREEDCRED_TCPCARAPP_H_
#define APPS_TCPSTREEDCRED_TCPCARAPP_H_

#include <omnetpp.h>
#include "common.h"
#include "CpuModel.h"
#include "corenetwork/binder/LteBinder.h"
#include "inet/common/INETDefs.h"
#include "inet/applications/tcpapp/TCPAppBase.h"

class CarApp : public TCPAppBase, public cListener {
public:
    void initialize(int stage) override;
    virtual ~CarApp();

protected:
    CoinAssignmentStage coinAssignmentStage;
    CoinDepositStage coinDepositStage;
    double coinAssignmentLastTry;
    double coinDepositLastTry;
    double RSU_POSITION_X;
    double RSU_POSITION_Y;
    double lastDistToRSU;

    MacNodeId nodeId_;
    LteBinder* binder_;

    /* TCPSocket::CallbackInterface callback methods */
    virtual void handleTimer(cMessage *msg) override {};

    void receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details) override;
    void handlePositionUpdate(cObject* obj);
    void socketEstablished(int connId, void *ptr);

    /* CarApp parameters */
    CpuModel cpuModel;
    int COIN_REQUEST_BYTE_SIZE;
    int COIN_DEPOSIT_BYTE_SIZE;
    int COIN_DEPOSIT_SIGNATURE_RESPONSE_BYTE_SIZE;
    double COIN_REQUEST_LATENCY_MEAN;
    double COIN_REQUEST_LATENCY_STDDEV;
    double COIN_DEPOSIT_LATENCY_MEAN;
    double COIN_DEPOSIT_LATENCY_STDDEV;
    double COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_MEAN;
    double COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_STDDEV;
};

#endif /* APPS_TCPSTREEDCRED_TCPCARAPP_H_ */
