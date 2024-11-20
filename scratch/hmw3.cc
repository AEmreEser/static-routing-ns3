/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

// Test program for this 3-router scenario, using static routing
//
// (a.a.a.a/32)A<--x.x.x.0/30-->B<--y.y.y.0/30-->C(c.c.c.c/32)

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
//Standard C++ libraries
#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

using namespace ns3;

//Sets up a logging component named StaticRoutingSlash32Test for tracking log outputs from this script.
//1.Organized Output: By naming the logging component, we can organize log output and control which
// logs are enabled or disabled.
//2. Debugging and Monitoring: This helps in debugging, as it allows you to see step-by-step execution
// and network events.
NS_LOG_COMPONENT_DEFINE("hmw3");
//Defines a named component for logging.

#define SUBNET_MASK "255.255.255.252" // /30

void inline print_interface_ipv4(const Ipv4InterfaceContainer & ifc){
    static uint32_t call = 0;
    for (uint32_t i = 0; i < ifc.GetN(); ++i){
        Ipv4Address ipAddr = ifc.GetAddress(i);
        std::cout << "call: " << call << " ip addr: " << ipAddr << std::endl;
    }
    call++;
}

int
main(int argc, char* argv[])
{
    // allows users to modify or override certain parameters of the simulation from the command
    // line when they execute the script

    //int argc and char* argv[] are used in the main function to receive arguments from the
    //command line. argc represents the number of arguments passed.
    //argv[] is an array of character pointers that hold the arguments.
    CommandLine cmd(__FILE__);
    //creates a CommandLine object to handle user-provided arguments.
    cmd.Parse(argc, argv);
    //reads and applies any arguments passed at runtime, allowing users to adjust settings without
    // modifying the code directly.
    //Node Creation
    Ptr<Node> nA = CreateObject<Node>();
    Ptr<Node> nB = CreateObject<Node>();
    Ptr<Node> nC = CreateObject<Node>();
    Ptr<Node> nD = CreateObject<Node>();

    // Gorouping nodes  into container c

    NodeContainer c = NodeContainer(nA, nB, nC, nD);

    //Installs the Internet stack (IPv4, TCP, UDP) on all nodes in c.
    InternetStackHelper internet;
    internet.Install(c);

    // Point-to-point links
    //Defines two node pairs for creating links: nAnB connects nA to nB. nBnC connects nB to nC.

    NodeContainer nAnB = NodeContainer(nA, nB);
    NodeContainer nBnC = NodeContainer(nB, nC);
    NodeContainer nCnD = NodeContainer(nC, nD);

    // Create channels first without IP addressing information
    //Configures a point-to-point link with a data rate of 5 Mbps and 2 ms delay.
    //Installs these configurations on the node pairs nAnB and nBnC.
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    NetDeviceContainer dAdB = p2p.Install(nAnB);
    NetDeviceContainer dBdC = p2p.Install(nBnC);
    NetDeviceContainer dCdD = p2p.Install(nCnD);
    
    //Creates a CSMA (Carrier-Sense Multiple Access) network device,
    //(CSMA allows devices to "listen" to the channel before sending data, reducing collisions.)
    // assigns it a MAC address,
    // and attaches it to node nA.Assigning a MAC address allows the CSMA device to be uniquely recognized on the network
    //Sets up a drop-tail queue to handle packet buffering

    Ptr<CsmaNetDevice> deviceA = CreateObject<CsmaNetDevice>();
    deviceA->SetAddress(Mac48Address::Allocate());
    nA->AddDevice(deviceA);
    deviceA->SetQueue(CreateObject<DropTailQueue<Packet>>());

    Ptr<CsmaNetDevice> deviceD = CreateObject<CsmaNetDevice>();
    deviceD->SetAddress(Mac48Address::Allocate());
    nD->AddDevice(deviceD);
    deviceD->SetQueue(CreateObject<DropTailQueue<Packet>>());

    // Add IP addresses
    //The base IP, 10.1.1.0, with the subnet mask 255.255.255.252, is used to create a small subnet that 
    // provides exactly two usable IP addresses (one for each node on this point-to-point link).
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", SUBNET_MASK);
    Ipv4InterfaceContainer iAiB = ipv4.Assign(dAdB);
    print_interface_ipv4(iAiB);
    //The subnet mask 255.255.255.252 (or /30 in CIDR notation) means:
    //The first 30 bits of the IP address identify the network.
    //The last 2 bits are available for hosts within that network.

    //IP Range in 10.1.1.0/30:
    //Network Address: 10.1.1.0 — Identifies the network itself and cannot be assigned to a device.
    //Usable IP Addresses: 10.1.1.1 and 10.1.1.2 — Can be assigned to the two devices on this link (nA and nB).
    //Broadcast Address: 10.1.1.3 — Used to send data to all devices on the subnet.

    //repeats it for nodes nB, nC
    ipv4.SetBase("10.1.2.0", SUBNET_MASK);
    Ipv4InterfaceContainer iBiC = ipv4.Assign(dBdC);
    print_interface_ipv4(iBiC);
    //repeats it for nodes nC, nD
    ipv4.SetBase("10.1.3.0", SUBNET_MASK);
    Ipv4InterfaceContainer iCiD = ipv4.Assign(dCdD);
    print_interface_ipv4(iCiD);

    //Manually assigns the IP 172.16.1.1/32 to nA on deviceA.
    Ptr<Ipv4> ipv4A = nA->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4B = nB->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4C = nC->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4D = nD->GetObject<Ipv4>();
    //Gets the IPv4 configuration object for each node (nA, nB, nC).

    int32_t ifIndexA = ipv4A->AddInterface(deviceA);
    int32_t ifIndexD = ipv4D->AddInterface(deviceD);
    //The AddInterface function adds a new network interface to each node's IPv4 object.
    //deviceA is the interface being added to nA, and deviceD is being added to nD.

    Ipv4InterfaceAddress ifInAddrA = Ipv4InterfaceAddress(Ipv4Address("172.16.1.1"), Ipv4Mask("/30"));
    // creates an Ipv4InterfaceAddress object, which includes an IP address (172.16.1.1) 
    //and a subnet mask (/32), for the interface on nA.

    ipv4A->AddAddress(ifIndexA, ifInAddrA);
    //assigns the previously created IP address (ifInAddrA) to the specific interface (ifIndexA)
    // on node nA.

    ipv4A->SetMetric(ifIndexA, 1);
    //SetMetric sets a routing metric (or cost) of 1 for the interface on node nA, which can influence routing
    // decisions if multiple routes are available.

    ipv4A->SetUp(ifIndexA);
    //SetUp activates the interface so it can begin sending and receiving packets.

    //repeats for node nD
    Ipv4InterfaceAddress ifInAddrD = Ipv4InterfaceAddress(Ipv4Address("192.168.1.1"), Ipv4Mask("/30"));
    ipv4D->AddAddress(ifIndexD, ifInAddrD);
    ipv4D->SetMetric(ifIndexD, 1);
    ipv4D->SetUp(ifIndexD);

    NS_LOG_UNCOND("IP addresses assigned and interfaces set up.");

    // Set up static routing from A to D
    Ipv4StaticRoutingHelper ipv4RoutingHelper;
    Ptr<Ipv4StaticRouting> staticRoutingA = ipv4RoutingHelper.GetStaticRouting(ipv4A);
    staticRoutingA->AddHostRouteTo(Ipv4Address("192.168.1.1"), Ipv4Address("10.1.1.2"), 1);

    Ptr<Ipv4StaticRouting> staticRoutingB = ipv4RoutingHelper.GetStaticRouting(ipv4B);
    staticRoutingB->AddHostRouteTo(Ipv4Address("192.168.1.1"), Ipv4Address("10.1.2.2"), 2);

    Ptr<Ipv4StaticRouting> staticRoutingC = ipv4RoutingHelper.GetStaticRouting(ipv4C);
    staticRoutingC->AddHostRouteTo(Ipv4Address("192.168.1.1"), Ipv4Address("10.1.3.2"), 2);

    // Set up static routing from D to A
    Ptr<Ipv4StaticRouting> staticRoutingD_rev = ipv4RoutingHelper.GetStaticRouting(ipv4D);
    staticRoutingD_rev->AddHostRouteTo(Ipv4Address("172.16.1.1"), Ipv4Address("10.1.3.1"), 1);

    Ptr<Ipv4StaticRouting> staticRoutingC_rev = ipv4RoutingHelper.GetStaticRouting(ipv4C);
    staticRoutingC_rev->AddHostRouteTo(Ipv4Address("172.16.1.1"), Ipv4Address("10.1.2.1"), 1);

    Ptr<Ipv4StaticRouting> staticRoutingB_rev = ipv4RoutingHelper.GetStaticRouting(ipv4B);
    staticRoutingB_rev->AddHostRouteTo(Ipv4Address("172.16.1.1"), Ipv4Address("10.1.1.1"), 1);

    NS_LOG_UNCOND("Static routes configured.");
    //is used to log a message that indicates the IP addresses have been assigned and the interfaces have been
    // configured successfully.


    // Create the OnOff application to send UDP datagrams of size 210 bytes at a rate of 448 Kb/s
    uint16_t port = 9; 
    //Defines the port number for the data packets that will be sent.
    
    OnOffHelper onoff = OnOffHelper("ns3::UdpSocketFactory", Address(InetSocketAddress(ifInAddrD.GetLocal(), port)));
    // Sets up an OnOffHelper to create a data-generating application that sends UDP packets.
    //InetSocketAddress(ifInAddrC.GetLocal(), port) defines the destination IP and port for the packets, which are 
    //directed to the IP address assigned to nC at the specified port.

    onoff.SetConstantRate(DataRate(6000));
    //Configures the application to send data at a constant rate.

    ApplicationContainer apps = onoff.Install(nA);
    //OnOff application on node nA.

    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    //Specifies the time window for the application to start and stop


    NS_LOG_UNCOND("OnOff application installed on Node A to send packets to Node D.");


    // Create a packet sink to receive packets

    PacketSinkHelper sink("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), port)));
    // Initializes a PacketSinkHelper, which is an application designed to receive packets.
    //that this application will receive UDP packets from nC by listening on the same port 
    //that nA is sending packets to
    //Ipv4Address::GetAny()? This tells nC to listen for packets on any of its IP addresses.

    
    apps = sink.Install(nD);
    //Installs the PacketSinkHelper application on node nD.

    apps.Start(Seconds(1.0));
    apps.Stop(Seconds(10.0));
    //Specifies the active period for the packet sink application

    NS_LOG_UNCOND("PacketSink application installed on Node D to receive packets.");


    // Enable ASCII and PCAP tracing
    AsciiTraceHelper ascii;
    //Tracing records packet activity for analysis, allowing you to inspect the events that occur during the simulation

    p2p.EnableAsciiAll(ascii.CreateFileStream("hmw3.tr"));
    //Records detailed events (like packet enqueueing and dequeueing) to a .tr file in text format.

    p2p.EnablePcapAll("hmw3");
    //Generates .pcap files, which can be viewed with tools like Wireshark to see the packet-level activity.

    NS_LOG_UNCOND("Tracing enabled for the simulation.");

    // Run the simulation
    Simulator::Run();
    //Simulator::Run(): Starts the simulation, executing all events (like packet transmissions) over the defined duration.

    NS_LOG_UNCOND("Simulation running...");

    // Destroy the simulation
    Simulator::Destroy();
    NS_LOG_UNCOND("Simulation completed.");
    //NS_LOG_UNCOND("Simulation completed."); logs that the simulation has finished, marking the end of the process.

    return 0;
}
