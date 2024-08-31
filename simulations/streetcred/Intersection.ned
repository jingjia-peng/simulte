// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
//
package lte.simulations.streetcred;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.networklayer.ipv4.RoutingTableRecorder;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.ethernet.Eth10G;

import lte.world.radio.LteChannelControl;
import lte.epc.PgwStandardSimplified;
import lte.corenetwork.binder.LteBinder;
import lte.corenetwork.deployer.LteDeployer;
import lte.corenetwork.nodes.eNodeB;
import lte.corenetwork.nodes.Ue;
import lte.corenetwork.nodes.cars.Car;

import org.car2x.veins.subprojects.veins_inet.VeinsInetManager;
import org.car2x.veins.visualizer.roads.RoadsCanvasVisualizer;

network Intersection
{
    parameters:
        double playgroundSizeX @unit(m); // x size of the area the nodes are in (in meters)
        double playgroundSizeY @unit(m); // y size of the area the nodes are in (in meters)
        double playgroundSizeZ @unit(m); // z size of the area the nodes are in (in meters)
		//# default (1000.1000)
		//# beijing (2000,2000)
		//# paris (1100,1300)
        @display("bgb=2000,2000");

    submodules:
        //# Veins manager module
        veinsManager: VeinsInetManager {
            @display("p=165,300;is=s");
        }
        roadsCanvasVisualizer: RoadsCanvasVisualizer {
            @display("p=1850,170");
        }
        //# LTE modules
        channelControl: LteChannelControl {
            @display("p=200,90;is=s");
        }
        binder: LteBinder {
            @display("p=610,300;is=s");
        }
        deployer: LteDeployer {
            @display("p=400,550;is=s");
        }
        //# Network Configurator
        configurator: IPv4NetworkConfigurator {
            @display("p=600,80;is=s");
        }

        //# Network instances
        rsu[1]: Car {
		    //# default (225,225)
			//# beijing (940,870)
			//# paris (670,570)
            @display("p=940,870;i=veins/sign/yellowdiamond;is=vs");
        }
}

