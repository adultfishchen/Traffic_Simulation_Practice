#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Mini1ScriptExample");

int main(int argc, char *argv[])
{
  //Define variables
  bool verbose = true;
  uint32_t n0Wifi = 3;
  uint32_t n1Wifi = 3;
  bool tracing = false;

  CommandLine cmd(__FILE__);
  cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", n0Wifi);
  cmd.AddValue("nWifi", "Number of wifi STA devices", n1Wifi);
  cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue("tracing", "Enable pcap tracing", tracing);

  cmd.Parse(argc, argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (n0Wifi > 18 && n1Wifi > 18)
  {
    std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
    return 1;
  }

  if (verbose)
  {
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
  }

  /********************Topology part************************/
  //Build a P2P connection with 2 nodes
  NodeContainer p2pNodes;
  p2pNodes.Create(2);

  //Set ttansit rate and channel delay
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

  //Install P2P deices to P2P nodes
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);

  //For Wifi0
  NodeContainer wifi0StaNodes;
  wifi0StaNodes.Create(n0Wifi);
  //Set the first node as AP
  NodeContainer wifi0ApNode = p2pNodes.Get(0);

  //Initial physical channel
  YansWifiChannelHelper channel0 = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy0;
  phy0.SetChannel(channel0.Create());

  WifiHelper wifi0;
  wifi0.SetRemoteStationManager("ns3::AarfWifiManager");

  //Mac layer setting
  WifiMacHelper mac0;
  Ssid ssid0 = Ssid("ns-3-ssid");
  mac0.SetType("ns3::StaWifiMac",
               "Ssid", SsidValue(ssid0),
               "ActiveProbing", BooleanValue(false));

  //Install network card device to Wifi nodes and deploy parameters
  NetDeviceContainer sta0Devices;
  sta0Devices = wifi0.Install(phy0, mac0, wifi0StaNodes);

  mac0.SetType("ns3::ApWifiMac",
               "Ssid", SsidValue(ssid0));

  //Install network card device to Wifi AP nodes and deploy parameters
  NetDeviceContainer ap0Devices;
  ap0Devices = wifi0.Install(phy0, mac0, wifi0ApNode);

  MobilityHelper mobility0;

  //Add moving model
  mobility0.SetPositionAllocator("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(0.0),
                                 "MinY", DoubleValue(0.0),
                                 "DeltaX", DoubleValue(5.0),
                                 "DeltaY", DoubleValue(10.0),
                                 "GridWidth", UintegerValue(3),
                                 "LayoutType", StringValue("RowFirst"));

  mobility0.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
  //Install moving model on STA nodes
  mobility0.Install(wifi0StaNodes);

  //Set AP : on the fixed place
  mobility0.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility0.Install(wifi0ApNode);

  //For Wifi1
  NodeContainer wifi1StaNodes;
  wifi1StaNodes.Create(n1Wifi);
  //Set the first node as AP
  NodeContainer wifi1ApNode = p2pNodes.Get(1);

  //Initial physical channel
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
  YansWifiPhyHelper phy;
  phy.SetChannel(channel.Create());

  WifiHelper wifi;
  wifi.SetRemoteStationManager("ns3::AarfWifiManager");

  //Mac layer setting
  WifiMacHelper mac;
  Ssid ssid = Ssid("ns-3-ssid");
  mac.SetType("ns3::StaWifiMac",
               "Ssid", SsidValue(ssid),
               "ActiveProbing", BooleanValue(false));

  //Install network card device to Wifi nodes and deploy parameters
  NetDeviceContainer staDevices;
  staDevices = wifi.Install(phy, mac, wifi1StaNodes);

  mac.SetType("ns3::ApWifiMac",
               "Ssid", SsidValue(ssid));

  //Install network card device to Wifi AP nodes and deploy parameters
  NetDeviceContainer apDevices;
  apDevices = wifi.Install(phy, mac, wifi1ApNode);

  MobilityHelper mobility;

  //Add moving model
  mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue(40.0),
                                 "MinY", DoubleValue(30.0),
                                 "DeltaX", DoubleValue(5.0),
                                 "DeltaY", DoubleValue(10.0),
                                 "GridWidth", UintegerValue(3),
                                 "LayoutType", StringValue("RowFirst"));

  mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue(Rectangle(-50, 50, -50, 50)));
  //Install moving model on STA nodes
  mobility.Install(wifi1StaNodes);

  //Set AP : on the fixed place
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.Install(wifi1ApNode);

  //Install network protocol
  InternetStackHelper stack0;
  stack0.Install(wifi0ApNode);
  stack0.Install(wifi0StaNodes);

  InternetStackHelper stack1;
  stack1.Install(wifi1ApNode);
  stack1.Install(wifi1StaNodes);

  Ipv4AddressHelper address;

  //Arrange P2P network address
  address.SetBase("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign(p2pDevices);

  //Arrange csma network address
  address.SetBase("10.1.2.0", "255.255.255.0");
  p2pInterfaces = address.Assign(sta0Devices);
  address.Assign(ap0Devices);

  //Arrange Wifi network address
  address.SetBase("10.1.3.0", "255.255.255.0");
  address.Assign(staDevices);
  address.Assign(apDevices);

  /**********************Applicationv part*********************/
  UdpEchoServerHelper echoServer(9);

  //Server installed on the last node of CSMA
  ApplicationContainer serverApps = echoServer.Install(wifi0StaNodes.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(0), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));

  //Client application installed on the last two node
  ApplicationContainer clientApps =
      echoClient.Install(wifi1StaNodes.Get(n1Wifi - 1));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  Simulator::Stop(Seconds(10.0));

  if (tracing == true)
  {
    pointToPoint.EnablePcapAll("third");
    phy0.EnablePcap("third", ap0Devices.Get(0));
    phy.EnablePcap("third", apDevices.Get(0));
  }

  //Show on NetAnim
  AnimationInterface anim("mini1.xml");
  anim.SetConstantPosition(p2pNodes.Get(0), 1.0, 1.0);
  anim.SetConstantPosition(p2pNodes.Get(1), 50.0, 50.0);

  Simulator::Run();
  Simulator::Destroy();
  return 0;
}
