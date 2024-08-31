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

#ifndef APPS_TCPSTREEDCRED_TCPRSUTHREAD_H_
#define APPS_TCPSTREEDCRED_TCPRSUTHREAD_H_

#include "inet/applications/tcpapp/TCPSrvHostApp.h"

class RSUThread : public inet::TCPServerThreadBase {
public:
    RSUThread();
    virtual ~RSUThread();

    virtual void established() override {};
    virtual void dataArrived(cMessage *msg, bool urgent) override {};
    virtual void timerExpired(cMessage *timer) override {};
};

#endif /* APPS_TCPSTREEDCRED_TCPRSUTHREAD_H_ */
