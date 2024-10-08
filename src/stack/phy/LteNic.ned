// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 


package lte.stack.phy;

import inet.linklayer.contract.IWirelessNic;
import lte.stack.mac.LteMac;
import lte.stack.pdcp_rrc.LtePdcpRrc;
import lte.stack.phy.feedback.LteDlFeedbackGenerator;
import lte.stack.rlc.LteRlc;
import lte.corenetwork.lteip.IP2lte;
import lte.x2.LteX2Manager;
import lte.stack.compManager.LteCompManager;
import lte.stack.d2dModeSelection.D2DModeSelection;
import lte.stack.handoverManager.LteHandoverManager;

// 
// Interface for the LTE Stack.
//
moduleinterface ILteNic extends IWirelessNic
{
        string nodeType;
        bool d2dCapable;            // inherit the value from the parent module
    gates:
        inout EUTRAN_RRC_Sap;   // Control of RRC Layer
        inout x2[] @loose;     // optional x2 manager
}

// 
// Base module for the LTE Stack.
//
// TODO: implement IWirelessNic
module LteNicBase like ILteNic
{
    parameters:
        @display("i=block/ifcard;bgb=250,444;bgl=3");
        string LtePdcpRrcType;      // One of: "LtePdcpRrcUe", "LtePdcpRrcEnb", "LtePdcpRrcRelayUe", "LtePdcpRrcRelayEnb"
        string LteMacType;          // One of: "LteMacUe", "LteMacEnb", "LteMacEnbExperimental", "LteMacRelayUe", "LteMacRelayEnb"
        string LtePhyType;
        string nodeType;
        bool d2dCapable;            // inherit the value from the parent module
        
    gates:
        //#
        //# Gates connecting UE/eNB and LTE Stack
        //#
        //# Control Ports
        inout EUTRAN_RRC_Sap;   // Control of RRC Layer
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @loose;  // to receive messages sent using sendDirect()
        inout x2[] @loose;     // optional x2 manager

    submodules:
        // PDCP-RRC Layer
        pdcpRrc: <LtePdcpRrcType> like LtePdcpRrc {
            @display("p=150,142");
            nodeType = nodeType;
        }
        // RLC Layer
        rlc: LteRlc {
            @display("p=150,227");
            d2dCapable = d2dCapable;
        }

        // MAC removed to lower layers as both types no longer require interface table

        // LtePhy submodule
        phy: <LtePhyType> like LtePhy {
            @display("p=151,390");
            @class(LtePhyType);
        }

    connections:
        //#
        //# Internal LTE Stack Connections 
        //# 

        EUTRAN_RRC_Sap <--> pdcpRrc.EUTRAN_RRC_Sap;

        //# PDCP <-> RLC
        pdcpRrc.UM_Sap <--> rlc.UM_Sap;
        pdcpRrc.AM_Sap <--> rlc.AM_Sap;
        pdcpRrc.TM_Sap <--> rlc.TM_Sap;

        //# external: lower connection
        radioIn --> phy.radioIn;
}

// Module for IP Based NICs
module LteNicIpBase extends LteNicBase
{
	   parameters:
        @display("i=block/ifcard;bgb=250,444;bgl=3");
        string interfaceTableModule;
        string routingTableModule;

    submodules:
        // bridge between lte stack and network layer
        ip2lte: IP2lte {
            nodeType = nodeType;
            interfaceTableModule = interfaceTableModule;
            routingTableModule = routingTableModule;
            @display("p=150,68");
        }

        // MAC Layer
        mac: <LteMacType> like LteMac {
            interfaceTableModule = interfaceTableModule;
            @display("p=151,315");
        }

    connections:
        ip2lte.toStackLte --> pdcpRrc.DataIn;
        ip2lte.fromStackLte <-- pdcpRrc.DataOut;

        //# external: lower connection
        ip2lte.upperLayerOut --> upperLayerOut;
        upperLayerIn --> ip2lte.upperLayerIn;

        //# RLC <-> MAC
        rlc.RLC_to_MAC --> mac.RLC_to_MAC;
        rlc.MAC_to_RLC <-- mac.MAC_to_RLC;

        //#
        //# Connections from LTE Stack to radio interface
        //#
        mac.MAC_to_PHY --> phy.upperGateIn;
        mac.PHY_to_MAC <-- phy.upperGateOut;
}

// Module for Non-IP Based NICs
module LteNicNonIpBase extends LteNicBase
{
    parameters:
        @display("i=block/ifcard;bgb=250,444;bgl=3");

    submodules:
        // MAC Layer
        mac: <LteMacType> like LteMac {
            @display("p=151,315");
        }


    connections:
        //# external: lower connection
        pdcpRrc.DataOut --> upperLayerOut;
        upperLayerIn --> pdcpRrc.DataIn;

        //# RLC <-> MAC
        rlc.RLC_to_MAC --> mac.RLC_to_MAC;
        rlc.MAC_to_RLC <-- mac.MAC_to_RLC;

        //#
        //# Connections from LTE Stack to radio interface
        //#
        mac.MAC_to_PHY --> phy.upperGateIn;
        mac.PHY_to_MAC <-- phy.upperGateOut;
}

//
// User Equipment of LTE stack
//
module LteNicUe extends LteNicIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcUe";
        LteMacType = default("LteMacUeRealistic");
        LtePhyType = "LtePhyUe";

    submodules:
        // Feedback generator submodule
        dlFbGen: LteDlFeedbackGenerator {
            @display("p=50,50;is=s");
        }
}

//
// D2D-capable User Equipment of LTE stack
//
module LteNicUeD2D extends LteNicIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcUeD2D";
        LteMacType = default("LteMacUeRealisticD2D");
        LtePhyType = "LtePhyUeD2D";

        string d2dPeerAddresses = default("");  // list of D2D peer UEs, separated by blank spaces

    submodules:
        // Feedback generator submodule
        dlFbGen: LteDlFeedbackGenerator {
            @display("p=50,50;is=s");
        }
}

//
// Non-IP Mode 4 D2D User Equipment of LTE stack
//
module LteNicVUeMode4 extends LteNicNonIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcUeD2D";
        LteMacType = default("LteMacVUeMode4");
        LtePhyType = "LtePhyVUeMode4";
        
        string d2dPeerAddresses = default(""); // list of D2D peer UEs, separated by blank spaces
}

//
// IP-based Mode 4 D2D User Equipment of LTE stack
//
module LteNicUeMode4D2D extends LteNicIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcUeD2D";
        LteMacType = default("LteMacVUeMode4D2D"); // IP-based MAC interface
        LtePhyType = "LtePhyVUeMode4";
        
        string d2dPeerAddresses = default(""); // list of D2D peer UEs, separated by blank spaces
}

//
// eNodeB of LTE stack
//
module LteNicEnb extends LteNicIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcEnb";
        LteMacType = default("LteMacEnbRealistic");
        LtePhyType = "LtePhyEnb";
        bool compEnabled = default(false);
        string LteCompManagerType = default("LteCompManagerProportional");

    submodules:
        //#
        //# X2 manager
        //#       
        x2Manager: LteX2Manager {
            @display("p=60,227,row");
        }

        //#
        //# Modules exploiting X2 interface
        //#
        compManager: <LteCompManagerType> like LteCompManager if compEnabled {
            @display("p=60,315,row");
        }
        handoverManager: LteHandoverManager {
            @display("p=60,142,row");
        }

    connections:
        //# connections between X2 Manager and its users
        compManager.x2ManagerIn <-- x2Manager.dataPort$o++ if compEnabled;
        compManager.x2ManagerOut --> x2Manager.dataPort$i++ if compEnabled;
        handoverManager.x2ManagerIn <-- x2Manager.dataPort$o++;
        handoverManager.x2ManagerOut --> x2Manager.dataPort$i++;

        //# connections between X2 Manager and GTP (for handover)
        x2$i[0] --> x2Manager.x2Gtp$i;
        x2$o[0] <-- x2Manager.x2Gtp$o;

        //# connections between X2 Manager and the outer world
        for i=1..sizeof(x2)-1 {
            x2$i[i] --> x2Manager.x2$i++;
            x2$o[i] <-- x2Manager.x2$o++;
        }
}

//
// eNodeB of LTE stack with support for D2D-capable UEs
//
module LteNicEnbD2D extends LteNicIpBase
{
    parameters:
        LtePdcpRrcType = "LtePdcpRrcEnbD2D";
        LteMacType = default("LteMacEnbRealisticD2D");
        LtePhyType = "LtePhyEnbD2D";
        bool compEnabled = default(false);
        string LteCompManagerType = default("LteCompManagerProportional");
        string d2dModeSelectionType = default("D2DModeSelectionBase");

    submodules:
        //#
        //# X2 manager
        //#       
        x2Manager: LteX2Manager {
            @display("p=60,227,row");
        }

        //#
        //# Modules exploiting X2 interface
        //#
        compManager: <LteCompManagerType> like LteCompManager if compEnabled {
            @display("p=60,315,row");
        }
        handoverManager: LteHandoverManager {
            @display("p=60,142,row");
        }

        d2dModeSelection: <d2dModeSelectionType> like D2DModeSelection {
            @display("p=60,68,row");
        }
    connections:
        //# connections between X2 Manager and its users
        compManager.x2ManagerIn <-- x2Manager.dataPort$o++ if compEnabled;
        compManager.x2ManagerOut --> x2Manager.dataPort$i++ if compEnabled;
        handoverManager.x2ManagerIn <-- x2Manager.dataPort$o++;
        handoverManager.x2ManagerOut --> x2Manager.dataPort$i++;

        //# connections between X2 Manager and GTP (for handover)
        x2$i[0] --> x2Manager.x2Gtp$i;
        x2$o[0] <-- x2Manager.x2Gtp$o;

        //# connections between X2 Manager and the outer world
        for i=1..sizeof(x2)-1 {
            x2$i[i] --> x2Manager.x2$i++;
            x2$o[i] <-- x2Manager.x2$o++;
        }
}

//
// Relay of LTE stack
//
module LteNicRelay extends LteNicIpBase
{
    parameters:
        LtePhyType = "LtePhyRelay";

    submodules:
        // Feedback generator submodule
        dlFbGen: LteDlFeedbackGenerator {
            @display("p=50,50;is=s");
        }

        // TODO Feedback generator submodule
        // ulFbGen: LteUlFeedbackGenerator {
        //     @display("p=200,150;is=s");
        // }
}
