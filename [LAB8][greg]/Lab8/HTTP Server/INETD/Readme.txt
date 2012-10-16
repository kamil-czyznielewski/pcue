##############################################################################
#
# Readme.txt
#
#############################################################################

This inetd example is similar to a unix daemon, it runs a set of three servers
and spawns worker threads for the clients connection requests.

Description:
-----------
There are 3 servers in this application. 

1. Chargen Server  Port# 19
    - writes a character stream in to the incoming connection into this port.

2. Echo Server   Port# 7
    - echoes the data back to the client received on this port.

3. Discard Server Port# 9
    - Discards the data. Keeps on reading the data received on this port.

Build:
-----
1. Open inetd project file and build.
2. Load inetd executable.
3. Run the inetd application. 

Test:
-----
  The application contacts the DHCP server obtains an IP Address and prints it
  It starts three servers and shows something like below:
  ## 
  Link established
  IP ADDRESS: 10.64.204.167
  CharGen Server running (port 19)
  Discard Server running (port 9)
  Echo Server running (port 7)
  ##

  A windows socket client application is provided to connect to the server.
  <install_path>\Blackfin\Examples\ADSP-BF537 Ez-kit Lite\LAN\Host\InetdClient\

  An executable of this project is provided in 
  <install_path>\Blackfin\Examples\ADSP-BF537 Ez-kit Lite\LAN\Host\InetdClient\InetdClient.exe
   This executable can be used to connect to the servers.
  
  For example to connect to the Chargen server for the above obtained IP 
  address use below command at the dos prompt.
  <install_path>\Blackfin\Examples\ADSP-BF537 Ez-kit Lite\LAN\Host\InetdClient\InetdClient.exe 10.64.204.167 19

  Commandline options for InetdClient are
  InetdClient <ip_address> <port_no>
 
  Alternatively users can use 'telnet' to the servers.For example run the below command to check the echo server 
  for the above IP address.

  cmd>telnet 10.64.204.167 7
  
  Once the connection is established the typed characters will be echoed back.

  Similarly other servers can be tested using 'telnet <ip_address> <port>'

  
