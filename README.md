# RdkWanManager

**These are the mission objectives of the WAN Manager architecture:**
===================================================================

- Interface Managers (e.g. XDSL, Eth, DOCSIS, GPON, Cellular) are expected to handle the physical interface bring up and report the status of the physical layer to WAN Manager.
Along with reporting the status of the physical layer, they may also report additional configuration information such as the IP Mode (IPv4 only/IPv6 only/Dual Stack), any VLAN IDs to be used for egress traffic, or static IP addresses (e.g. received through PDP on a Cellular link).
- WAN Manager will then handle all link and IP layer configuration by interfacing with other RDK components (e.g. VLAN Manager, DHCP Manager, etc...) or Linux utility commands (e.g. ip addr, ip ro, etc...)
- WAN Manager will run all business logic to configure WAN interfaces and ensure Internet Connectivity.

WAN Manager comprises of several internal modules:
==================================================
- WAN Manager will run a single instance of the Failover Policy, which provides runtime failover/switching between different Groups of interfaces.
- WAN Manager will run N instances of a Selection Policy, one instance per interface Group.
- This Selection Policy may be AutoWAN, Parallel Scan, or a Fixed Mode policy. Different policies use different algorithms for selecting an interface to be used for WAN. (e.g. the AutoWAN policy tries one interface one by one until one has been validated for Internet Access.)
- The Selection Policies only run for interfaces within a selected Group. (E.g. if you want to run AutoWAN policy to select one interface between DOCSIS and EthWAN, you would mark both DOCSIS and EthWAN as belonging to Group 1, and then run the AutoWAN policy on Group 1 only.)
- For multiple Groups of interfaces, you need to run multiple Selection Policies, one for each Group.
- This allows for more flexible configurations, such as running an AutoWAN policy between DOCSIS and EthWAN (Group 1), and allowing for runtime failover between the Group 1 interface and a Group 2 interface (e.g. LTE).
- WAN Manager will run N instances of the WAN Interface State Machine, one instance per virtual interface.
- A virtual interface is configured in the data model as being a combination of link layer and ip layer interfaces/protocols to be configured on top of the base interface. The WAN Interface State Machine ensures correct configuration and deconfiguration of a virtual interface by building the stack from the bottom up. 
- WAN Manager supports both RBUS and WebConfig for remote configuration. RBUS is also used for internal communications between WAN Manager and Interface Managers.
