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


#include "RSUThread.h"
#include "message/CoinRequest_m.h"
#include "message/CoinAssignment_m.h"
#include "message/CoinDeposit_m.h"
#include "message/CoinDepositSignatureRequest_m.h"
#include "message/CoinDepositSignatureResponse_m.h"


void RSUThread::dataArrived(cMessage *msg, bool)
{
    EV_WARN << "[RSUThread]: I received a data message of class " << msg->getClassName()
            << ", name " << msg->getName() << ", byte " << PK(msg)->getByteLength() << endl;

    if (CoinRequest* req = dynamic_cast<CoinRequest*>(msg)) {
        int vid = req->getVid();
        EV_WARN << "[RSU]: I received a message of CoinRequest from " << vid << endl;

        CoinAssignment* packet = new CoinAssignment();
        packet->setVid(vid);
        packet->setByteLength(10); // TODO: get customized bytelen from .ned file
        // TODO: get CPU latency

        getSocket()->send(packet);
        EV_WARN << "[RSU]: I sent a message of CoinAssignment to " << vid << endl;
    }
    else if (CoinDeposit* req = dynamic_cast<CoinDeposit*>(msg)) {
        int vid = req->getVid();
        EV_WARN << "[RSU] I received a message of CoinDeposit from " << vid << endl;

        CoinDepositSignatureRequest* packet = new CoinDepositSignatureRequest();
        packet->setVid(vid);
        packet->setByteLength(10); // TODO: get customized bytelen from .ned file
        // TODO: get CPU latency

        getSocket()->send(packet);
        EV_WARN << "[RSU]: I sent a message of CoinAssignment to " << vid << endl;
    }
    else if (CoinDepositSignatureResponse* req = dynamic_cast<CoinDepositSignatureResponse*>(msg)) {
        int vid = req->getVid();
        EV_WARN << "[RSU] I received a message of CoinDepositSignatureResponse from " << vid << endl;

        // TODO: check RSU stage map to see if deposit succeed
        EV_WARN << "[Vehicle " << vid << "]: Coin deposit succeed." << endl;
    }
}
