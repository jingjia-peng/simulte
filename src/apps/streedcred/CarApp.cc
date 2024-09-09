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

#include "CarApp.h"
#include <omnetpp.h>
#include "inet/common/FindModule.h"
#include "veins_inet/VeinsInetMobility.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "message/CoinRequest_m.h"
#include "message/CoinDeposit_m.h"
#include "message/CoinDepositSignatureResponse_m.h"

Define_Module(CarApp);

void CarApp::initialize(int stage)
{
    TCPAppBase::initialize(stage);
    if (stage==inet::INITSTAGE_LOCAL){
        FindModule<>::findHost(this)->subscribe(veins::VeinsInetMobility::mobilityStateChangedSignal, this);

        coinAssignmentStage = CoinAssignmentStage::INIT;
        coinDepositStage = CoinDepositStage::INIT;
        RSU_POSITION_X = par("RSU_POSITION_X");
        RSU_POSITION_Y = par("RSU_POSITION_Y");
        lastDistToRSU = 10000;

        cpuModel.init(1);
        COIN_REQUEST_BYTE_SIZE = par("COIN_REQUEST_BYTE_SIZE");
        COIN_DEPOSIT_BYTE_SIZE = par("COIN_DEPOSIT_BYTE_SIZE");
        COIN_DEPOSIT_SIGNATURE_RESPONSE_BYTE_SIZE = par("COIN_DEPOSIT_SIGNATURE_RESPONSE_BYTE_SIZE");
        COIN_REQUEST_LATENCY_MEAN = par("COIN_REQUEST_LATENCY_MEAN");
        COIN_REQUEST_LATENCY_STDDEV = par("COIN_REQUEST_LATENCY_STDDEV");
        COIN_DEPOSIT_LATENCY_MEAN = par("COIN_DEPOSIT_LATENCY_MEAN");
        COIN_DEPOSIT_LATENCY_STDDEV = par("COIN_DEPOSIT_LATENCY_STDDEV");
        COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_MEAN = par("COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_MEAN");
        COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_STDDEV = par("COIN_DEPOSIT_SIGNATURE_RESPONSE_LATENCY_STDDEV");
    } else if (stage==inet::INITSTAGE_APPLICATION_LAYER) {
        // Register the node with the binder
        // Issue primarily is how do we set the link layer address
        // Get the binder
        binder_ = getBinder();
        // Get our UE
        cModule *ue = getParentModule();
        //Register with the binder
        nodeId_ = binder_->registerNode(ue, UE, 0);

        // Get my IP address
        L3Address ip;
        L3AddressResolver().tryResolve(par("localAddress"), ip);
        // Register the nodeId_ with the binder.
        binder_->setMacNodeId(ip.toIPv4(), nodeId_);
        EV_WARN << "[Vehicle " << nodeId_ << "] MAC address: " << nodeId_ << " IP address: " << ip << endl;
    }
}

void CarApp::handlePositionUpdate(cObject* obj){
        veins::VeinsInetMobility* const mobility = check_and_cast<veins::VeinsInetMobility*>(obj);
        inet::Coord curPos = mobility->getCurrentPosition();
        double distToRSU = sqrt(pow(curPos.x - RSU_POSITION_X, 2) + pow(curPos.y - RSU_POSITION_Y, 2));
        double currentTime = simTime().dbl();

        // When approaching the intersection, trigger coin deposit.
        // before first message sending, connect to RSU
        if (distToRSU < 150 && distToRSU < lastDistToRSU && coinDepositStage == CoinDepositStage::INIT
                && (socket.getState() == inet::TCPSocket::BOUND || socket.getState() == inet::TCPSocket::NOT_BOUND)) {
            connect();
        }

        // When leaving the intersection, trigger coin assignment.
        if (distToRSU < 150 && distToRSU > lastDistToRSU && coinAssignmentStage == CoinAssignmentStage::INIT) {
            coinAssignmentLastTry = currentTime;
            std::pair<double,double> latency = cpuModel.getLatency(currentTime, COIN_REQUEST_LATENCY_MEAN, COIN_REQUEST_LATENCY_STDDEV);

            CoinRequest* packet = new CoinRequest();
            packet->setByteLength(COIN_REQUEST_BYTE_SIZE);
            packet->setVid(nodeId_);

            // TODO: add delay before sending
            connect();
            sendPacket(packet);
            coinAssignmentStage = CoinAssignmentStage::REQUESTED;
            EV_WARN << "[Vehicle " << nodeId_ << "]: I sent a message of CoinRequest. Queue time " << latency.first
                    << " Computation time " << latency.second << endl;
        }

        // When leaving the intersection, claim failure when the vehicle is too far from RSU
        if (distToRSU > 150) {
            if (coinAssignmentStage != CoinAssignmentStage::INIT && coinAssignmentStage != CoinAssignmentStage::FINISHED && coinAssignmentStage != CoinAssignmentStage::FAILED) {
                coinAssignmentStage = CoinAssignmentStage::FAILED;
                EV_WARN << "[Vehicle " << nodeId_ << "]: Coin assignment failed." << endl;
            }
            if (coinDepositStage != CoinDepositStage::INIT && coinDepositStage != CoinDepositStage::SIGNATURE_SENT && coinDepositStage != CoinDepositStage::FAILED) {
                coinDepositStage = CoinDepositStage::FAILED;
                EV_WARN << "[Vehicle " << nodeId_ << "]: Coin deposit failed." << endl;
            }
        }
        lastDistToRSU = distToRSU;
}

void CarApp::socketEstablished(int connId, void *ptr)
{
    // *redefine* to perform or schedule first sending
    TCPAppBase::socketEstablished(connId, ptr);
    double currentTime = simTime().dbl();

    if (coinDepositStage == CoinDepositStage::INIT) {
        std::pair<double,double> latency = cpuModel.getLatency(currentTime, COIN_DEPOSIT_LATENCY_MEAN, COIN_DEPOSIT_LATENCY_STDDEV);

        CoinDeposit* packet = new CoinDeposit();
        packet->setByteLength(COIN_DEPOSIT_BYTE_SIZE);
        packet->setVid(nodeId_);

//      TODO: add delay before sending
        sendPacket(packet);
        coinDepositStage = CoinDepositStage::REQUESTED;
        EV_WARN << "[Vehicle " << nodeId_ << "]: I sent a message of CoinDeposit. Queue time " << latency.first
                << " Computation time " << latency.second << endl;
    }
}

void CarApp::receiveSignal(cComponent* source, simsignal_t signalID, cObject* obj, cObject* details){
    Enter_Method_Silent();
    if (signalID == veins::VeinsInetMobility::mobilityStateChangedSignal) {
        handlePositionUpdate(obj);
    }
}

CarApp::~CarApp() {
    binder_->unregisterNode(nodeId_);
}

