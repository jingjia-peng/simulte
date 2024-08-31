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

package lte.apps.streedcred;
import inet.applications.contract.ITCPApp;
//
// TODO auto-generated type
//
simple RSUApp like ITCPApp
{
    parameters:
		string localAddress = default("");
		int localPort = default(1000);
		string serverThreadClass = default("TCPRSUThread");
		string dataTransferMode @enum("bytecount", "object", "bytestream") = default("bytecount");
		
		int numCpuCores;
    	int COIN_ASSIGNMENT_BYTE_SIZE;
    	int COIN_DEPOSIT_SIGNATURE_REQUEST_BYTE_SIZE;
    	int COIN_SUBMISSION_BYTE_SIZE;
    	double COIN_ASSIGNMENT_LATENCY_MEAN;
    	double COIN_ASSIGNMENT_LATENCY_STDDEV;
    	double COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_MEAN;
    	double COIN_DEPOSIT_SIGNATURE_REQUEST_LATENCY_STDDEV;
    	double COIN_SUBMISSION_LATENCY_MEAN;
    	double COIN_SUBMISSION_LATENCY_STDDEV;
    gates:
        input tcpIn @labels(TCPCommand/up);
        output tcpOut @labels(TCPCommand/up);
}
