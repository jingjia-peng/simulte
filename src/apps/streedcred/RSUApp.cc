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

#include "RSUApp.h"
#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/lifecycle/NodeStatus.h"
#include "inet/networklayer/common/L3AddressResolver.h"

#include "message/CoinAssignment_m.h"
#include "message/CoinDepositSignatureRequest_m.h"
#include "message/CoinSubmission_m.h"

Define_Module(RSUApp);

using std::string;

void RSUApp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage==inet::INITSTAGE_LOCAL){
        int numCpuCores = par("numCpuCores");
        cpuModel.init(numCpuCores);
        COIN_ASSIGNMENT_BYTE_SIZE = par("COIN_ASSIGNMENT_BYTE_SIZE");
        COIN_DEPOSIT_SIGNATURE_REQUEST_BYTE_SIZE = par("COIN_DEPOSIT_SIGNATURE_REQUEST_BYTE_SIZE");
        COIN_SUBMISSION_BYTE_SIZE = par("COIN_SUBMISSION_BYTE_SIZE");
        COIN_ASSIGNMENT_LATENCY_MEAN = par("COIN_ASSIGNMENT_LATENCY_MEAN");
        COIN_ASSIGNMENT_LATENCY_STDDEV = par("COIN_ASSIGNMENT_LATENCY_STDDEV");
        COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_MEAN = par("COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_MEAN");
        COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_STDDEV = par("COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_STDDEV");
        COIN_SUBMISSION_LATENCY_MEAN = par("COIN_SUBMISSION_LATENCY_MEAN");
        COIN_SUBMISSION_LATENCY_STDDEV = par("COIN_SUBMISSION_LATENCY_STDDEV");
    }
    else if (stage==inet::INITSTAGE_APPLICATION_LAYER) {
        // Register the node with the binder
        // Issue primarily is how do we set the link layer address
        // Get the binder
        binder_ = getBinder();
        // Get our UE
        cModule *ue = getParentModule();
        // Register with the binder
        nodeId_ = binder_->registerNode(ue, UE, 0);

        // Get my IP address and set the socket
        IPv4Address ip = L3AddressResolver().resolve(string("rsu[0]").c_str()).toIPv4();
        int localPort = par("localPort");
        serverSocket.setOutputGate(gate("tcpOut"));
        serverSocket.readDataTransferModePar(*this);
        serverSocket.bind(ip, localPort);
        serverSocket.listen();

        // Register the nodeId_ with the binder.
        binder_->setMacNodeId(ip, nodeId_);
        EV_WARN << "[RSU] MAC address: " << nodeId_ << " IP address: " << ip << endl;

        bool isOperational;
        NodeStatus *nodeStatus = dynamic_cast<NodeStatus *>(findContainingNode(this)->getSubmodule("status"));
        isOperational = (!nodeStatus) || nodeStatus->getState() == NodeStatus::UP;
        if (!isOperational)
            throw cRuntimeError("This module doesn't support starting in node DOWN state");
    }
}

void RSUApp::handleMessage(cMessage *msg){
    TCPSrvHostApp::handleMessage(msg);
    EV_WARN << "[RSU]: receive a message" << endl;
}

RSUApp::~RSUApp() {
    binder_->unregisterNode(nodeId_);
}

