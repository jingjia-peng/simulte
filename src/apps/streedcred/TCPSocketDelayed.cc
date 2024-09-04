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

#include "TCPSocketDelayed.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"

void TCPSocketDelayed::send(cMessage *msg, simtime_t delay)
{
    if (sockstate != CONNECTED && sockstate != CONNECTING && sockstate != PEER_CLOSED)
        throw cRuntimeError("TCPSocket::send(): socket not connected or connecting, state is %s", stateName(sockstate));

    msg->setKind(inet::TCP_C_SEND);
    inet::TCPSendCommand *cmd = new inet::TCPSendCommand();
    cmd->setConnId(connId);
    msg->setControlInfo(cmd);

    if (!gateToTcp)
        throw cRuntimeError("TCPSocket: setOutputGate() must be invoked before socket can be used");

    check_and_cast<cSimpleModule *>(gateToTcp->getOwnerModule())->sendDelayed(msg, delay, gateToTcp);
}

