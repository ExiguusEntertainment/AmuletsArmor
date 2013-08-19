#include <string.h>
#include "ipx_client.h"
#include <time.h>
#ifdef _MSC_VER
#pragma pack(1)
#endif

#include "SDL_net.h"
extern "C" {
#include "TICKER.H"

extern void PacketPrint(void *aData, unsigned int aSize);

#define IPXBUFFERSIZE 1424

#define GCC_ATTRIBUTE(x) /* attribute not supported */
#define GCC_UNLIKELY(x) (x)
#define GCC_LIKELY(x) (x)

#define INLINE __forceinline
#define DB_FASTCALL __fastcall

#if defined(_MSC_VER) && (_MSC_VER >= 1400) 
#pragma warning(disable : 4996) 
#endif


/* The internal types */
typedef  unsigned char		Bit8u;
typedef    signed char		Bit8s;
typedef unsigned short		Bit16u;
typedef   signed short		Bit16s;
typedef  unsigned long		Bit32u;
typedef    signed long		Bit32s;
typedef unsigned __int64	Bit64u;
typedef   signed __int64	Bit64s;
typedef unsigned int		Bitu;
typedef signed int			Bits;

typedef Bit32u PhysPt;
typedef Bit8u * HostPt;
typedef Bit32u RealPt;

typedef Bit32s MemHandle;

#define LOG_MSG printf

struct PackedIP {
	Uint32 host;
	Uint16 port;
} GCC_ATTRIBUTE(packed);

struct nodeType {
	Uint8 node[6];
} GCC_ATTRIBUTE(packed) ;

struct IPXHeader {
	Uint8 checkSum[2];
	Uint8 length[2];
	Uint8 transControl; // Transport control
	Uint8 pType; // Packet type

	struct transport {
		Uint8 network[4];
		union addrtype {
			nodeType byNode;
			PackedIP byIP ;
		} GCC_ATTRIBUTE(packed) addr;
		Uint8 socket[2];
	} dest, src;

	Uint32 counter; //! Special frame counter that is ONLY in A&A.  Used to track order of packets (if order is needed)
} GCC_ATTRIBUTE(packed);

struct ipxnetaddr {
	Uint8 netnum[4];   // Both are big endian
	Uint8 netnode[6];
} localIpxAddr;

struct packetBuffer {
	Bit8u buffer[IPXBUFFERSIZE];
	Bit16s packetSize;  // Packet size remaining in read
	Bit16s packetRead;  // Bytes read of total packet
	bool inPacket;      // In packet reception flag
	bool connected;		// Connected flag
	bool waitsize;
};

#define CONVIP(hostvar) hostvar & 0xff, (hostvar >> 8) & 0xff, (hostvar >> 16) & 0xff, (hostvar >> 24) & 0xff
#define CONVIPX(hostvar) hostvar[0], hostvar[1], hostvar[2], hostvar[3], hostvar[4], hostvar[5]

static IPaddress ipxServConnIp;			// IPAddress for client connection to server
static Bit16u udpPort = 213;
static UDPsocket ipxClientSocket;
static int UDPChannel;						// Channel used by UDP connection
static Bit8u recvBuffer[IPXBUFFERSIZE];	// Incoming packet buffer
static Bit8u sendBuffer[IPXBUFFERSIZE];    // Incoming packet buffer
static unsigned char G_destinationAddr[6];
static T_word32 G_ipxFrameCounter = 0;

static Bit16u swapByte(Bit16u sockNum) {
	return (((sockNum>> 8)) | (sockNum << 8));
}

/*--------------------------------------------------------------------------*
 * Routine: UnpackIP
 *--------------------------------------------------------------------------*/
/**
 * Utility routine to split apart a PackedIP from the 6 byte value into
 * two parts: The 4 byte IP, and the 2 byte port number.
 *
 * @param ipPack -- Packed IP to split
 * @param ipAddr -- ipAddress structure to fill with IP and port.
 *
 * <!-----------------------------------------------------------------------*/
void UnpackIP(PackedIP ipPack, IPaddress * ipAddr)
{
    ipAddr->host = ipPack.host;
    ipAddr->port = ipPack.port;
}

/*--------------------------------------------------------------------------*
 * Routine: PackIP
 *--------------------------------------------------------------------------*/
/**
 * Utility routine to combine the 4 byte IP, and the 2 byte port number
 * into a single 6 byte number.
 *
 * @param ipPack -- Packed IP to split
 * @param ipAddr -- ipAddress structure to fill with IP and port.
 *
 * <!-----------------------------------------------------------------------*/
void PackIP(IPaddress ipAddr, PackedIP *ipPack)
{
    ipPack->host = ipAddr.host;
    ipPack->port = ipAddr.port;
}

/*--------------------------------------------------------------------------*
 * Routine: _IPXPingAck
 *--------------------------------------------------------------------------*/
/**
 * Because the UDP packets may also be used for special purposes (like ping)
 * handle those special cases here.
 *
 * @param p_data -- Pointer to place to receive data
 * @param size -- Size of returned data (no bigger than IPXBUFFERSIZE)
 *
 * @return Flag, 1=packet returned, else 0
 *
 * <!-----------------------------------------------------------------------*/
static void _IPXPingAck(IPaddress retAddr)
{
	IPXHeader regHeader;
	UDPpacket regPacket;
	int result;

	// Setup the checksum for the ping and size (just the header)
	SDLNet_Write16(0xffff, regHeader.checkSum);
	SDLNet_Write16(sizeof(regHeader), regHeader.length);

	// Set the destination network to be the return address (using network 0)
	SDLNet_Write32(0, regHeader.dest.network);
	PackIP(retAddr, &regHeader.dest.addr.byIP);
	SDLNet_Write16(0x2, regHeader.dest.socket);

	// We are the source (localIpx)
	SDLNet_Write32(0, regHeader.src.network);
	memcpy(regHeader.src.addr.byNode.node, localIpxAddr.netnode, sizeof(regHeader.src.addr.byNode.node));
	SDLNet_Write16(0x2, regHeader.src.socket);

	// This is a ping packet
	regHeader.transControl = 0;
	regHeader.pType = 0x0;

	// Prepare the packet structure for SDL_Net
	regPacket.data = (Uint8 *)&regHeader;
	regPacket.len = sizeof(regHeader);
	regPacket.maxlen = sizeof(regHeader);
	regPacket.channel = UDPChannel;

	// Send the packet.
	result = SDLNet_UDP_Send(ipxClientSocket, regPacket.channel, &regPacket);

	// Report error on failures
	if (result == 0) {
	    LOG_MSG("UDP packet send fail!");
	}
}

/*--------------------------------------------------------------------------*
 * Routine: _IPXPingAck
 *--------------------------------------------------------------------------*/
/**
 * Because the UDP packets may also be used for special purposes (like ping)
 * handle those special cases here.
 *
 * @param p_data -- Pointer to place to receive data
 * @param size -- Size of returned data (no bigger than IPXBUFFERSIZE)
 *
 * @return Flag, 1=packet returned, else 0
 *
 * <!-----------------------------------------------------------------------*/
void IPXSendPacket(char const *p_data, unsigned int size)
{
    UDPpacket regPacket;
    IPXHeader& regHeader = *((IPXHeader *)&sendBuffer);
    int result;
	T_word32 tick = clock();

	regHeader.src.socket[0] = 0x86;
    regHeader.src.socket[1] = 0x9C;
    regHeader.dest.socket[0] = 0x86;
    regHeader.dest.socket[1] = 0x9C;

#if 0
	printf("IPXSendPacket: [");
	for (int i=0; i<size; i++) {
		printf("%02X ", (unsigned char)p_data[i]);
	}
	printf("]\n");
#endif
#if 1
	printf("%02d:%02d:%02d.%03d IPX->", tick/3600000, (tick/60000) % 60, (tick/1000) % 60, tick%1000);
	PacketPrint((void *)p_data, size);
#endif
    if (size >= (IPXBUFFERSIZE - sizeof(IPXHeader))) {
        LOG_MSG("IPX: Packet too big!");
        return;
    }

    // Setup the checksum for the ping and size (just the header)
    SDLNet_Write16(0xffff, regHeader.checkSum);
    SDLNet_Write16(sizeof(regHeader) + size, regHeader.length);

    // Set the destination network to be the return address (using network 0)
    SDLNet_Write32(0, regHeader.dest.network);
    memcpy(regHeader.dest.addr.byNode.node, G_destinationAddr, 6);
    SDLNet_Write16(0x869C, regHeader.dest.socket);

    // We are the source (localIpx)
    SDLNet_Write32(0, regHeader.src.network);
    memcpy(regHeader.src.addr.byNode.node, localIpxAddr.netnode,
            sizeof(regHeader.src.addr.byNode.node));
    SDLNet_Write16(0x869C, regHeader.src.socket);

    // Copy over the data to send
    memcpy(sendBuffer + sizeof(IPXHeader), p_data, size);

    // Add a stamp on the packet number (in the old days, I was afraid
    // that the packets would get out of order and need to be sorted, so
    // I added a frame counter.  This really was stupid and not needed).
    regHeader.counter = G_ipxFrameCounter++;

    // Unspecified type
    regHeader.transControl = 0;
    regHeader.pType = 0x0;

    // Prepare the packet structure for SDL_Net
    // and direct back to the server
    regPacket.data = (Uint8 *)&regHeader;
    regPacket.len = sizeof(regHeader) + size;
    regPacket.maxlen = sizeof(regHeader) + size;
    regPacket.channel = UDPChannel;
    regPacket.address = ipxServConnIp;

    // Send the packet.
    result = SDLNet_UDP_Send(ipxClientSocket, regPacket.channel, &regPacket);

    // Report error on failures
    if (result == 0) {
        LOG_MSG("UDP packet send fail!");
    }
}



/*--------------------------------------------------------------------------*
 * Routine: _IPXHandleSpecialPacket
 *--------------------------------------------------------------------------*/
/**
 * Because the UDP packets may also be used for special purposes (like ping)
 * handle those special cases here.
 *
 * @param p_data -- Pointer to place to receive data
 * @param size -- Size of returned data (no bigger than IPXBUFFERSIZE)
 *
 * @return Flag, 1=packet processed, else 0
 *
 * <!-----------------------------------------------------------------------*/
static int _IPXHandleSpecialPacket(Bit8u *buffer, Bit16s bufSize)
{
	//ECBClass *useECB;
	//ECBClass *nextECB;
	Bit16u *bufword = (Bit16u *)buffer;
	Bit16u useSocket = swapByte(bufword[8]);
	IPXHeader * tmpHeader;
	tmpHeader = (IPXHeader *)buffer;

	// Check to see if ping packet on socket 2
	if (useSocket == 0x2) {
		// Is this a broadcast?
		if((tmpHeader->dest.addr.byIP.host == 0xffffffff) &&
			(tmpHeader->dest.addr.byIP.port == 0xffff)) {
			// Yes.  We should return the ping back to the sender
			IPaddress tmpAddr;

			UnpackIP(tmpHeader->src.addr.byIP, &tmpAddr);
			_IPXPingAck(tmpAddr);

			return 1;
		}
	}

	return 0;
}

/*--------------------------------------------------------------------------*
 * Routine: IPXClientPoll
 *--------------------------------------------------------------------------*/
/**
 * Check if a packet has arrived.  If so, return what was found and return
 * a flag.
 *
 * @param p_data -- Pointer to place to receive data
 * @param size -- Size of returned data (no bigger than IPXBUFFERSIZE)
 *
 * @return Flag, 1=packet returned, else 0
 *
 * <!-----------------------------------------------------------------------*/
int IPXClientPoll(char *p_data, unsigned int *size)
{
	int numrecv;
	UDPpacket inPacket;
	IPXHeader *p_header;
	inPacket.data = (Uint8 *)recvBuffer;
	inPacket.maxlen = IPXBUFFERSIZE;
	inPacket.channel = UDPChannel;
	T_word32 tick = clock();

	// Its amazing how much simpler UDP is than TCP
	// Is there a packet to process?
	numrecv = SDLNet_UDP_Recv(ipxClientSocket, &inPacket);
    if (numrecv) {
        // Is the received packet big enough to be an IPX packet over UDP?
        if (inPacket.len >= sizeof(IPXHeader)) {
            // Get access to the IPX header that is at the start of the UDP
            // packet data.
            p_header = (IPXHeader *)inPacket.data;

            // Process the packet for any special actions (like echo for ping)
            if (_IPXHandleSpecialPacket(inPacket.data, inPacket.len)) {
                // If handled by the special routines, this packet is
                // not for us to process.  Stop here and signal no data packet.
                return 0;
            }

            // Not a special packet.  Looks like one of ours (we catch all
            // the others).
            // Is there data in this packet?
            if (inPacket.len > sizeof(IPXHeader)) {
                // Copy the IPX data payload over
                memcpy((void *)p_data, (void *)&p_header[1],
                        inPacket.len - sizeof(IPXHeader));
                *size = inPacket.len;
#if 1
	printf("%02d:%02d:%02d.%03d IPX<-", tick/3600000, (tick/60000) % 60, (tick/1000) % 60, tick%1000);
	PacketPrint((void *)p_data, *size);
#endif

                // Signal data was returned
                return 1;
            } else {
                // No data payload, no packet
                return 0;
            }
        } else {
            // Packet is too small to be an IPX packet
            // Ignore it.
            LOG_MSG("IPX: Ignore small IPX packet of size %d\n", inPacket.len);
            return 0;
        }
    } else {
        // no data
        return 0;
    }
}

/*--------------------------------------------------------------------------*
 * Routine: IPXGetUniqueAddress
 *--------------------------------------------------------------------------*/
/**
 * Get the unique address to this location (it's just the 6 byte MAC
 * address).
 *
 * @param p_unique -- Place to store the unique address.
 *
 * <!-----------------------------------------------------------------------*/
void IPXGetUniqueAddress(unsigned char address[6])
{
    // Just returned the local IPX's node address (should be MAC-like)
    memcpy(address, localIpxAddr.netnode, 6);
}

/*--------------------------------------------------------------------------*
 * Routine: IPXSetDestinationAddress
 *--------------------------------------------------------------------------*/
/**
 * Declare where we are sending the next packet.
 *
 * @param address -- Address of target (MAC address)
 *
 * <!-----------------------------------------------------------------------*/
void IPXSetDestinationAddress(unsigned char address[6])
{
    // Just returned the local IPX's node address (should be MAC-like)
    memcpy(G_destinationAddr, address, 6);
}

/*--------------------------------------------------------------------------*
 * Routine: IPXGetDestinationAddress
 *--------------------------------------------------------------------------*/
/**
 * Return where we are sending the next packet.
 *
 * @param address -- Address of target (MAC address)
 *
 * <!-----------------------------------------------------------------------*/
unsigned char *IPXGetDestinationAddress(void)
{
    // This is weak, but for now, return the destination address
    return G_destinationAddr;
}

/*--------------------------------------------------------------------------*
 * Routine: IPXSetDestinationAddressAll
 *--------------------------------------------------------------------------*/
/**
 * Set next packet to be broadcast
 *
 * <!-----------------------------------------------------------------------*/
void IPXSetDestinationAddressAll(void)
{
    // Broadcast is all 0xFF's
    memset(G_destinationAddr, 0xFF, 6);
}

/*--------------------------------------------------------------------------*
 * Routine: IPXConnectToServer
 *--------------------------------------------------------------------------*/
/**
 * @brief     Attempt to connect to the server using the IPX over UDP
 * system.
 *
 * @param strAddr -- TCP/IP address of server
 *
 * @return Error code, 0=failure, 1=success
 *
 * <!-----------------------------------------------------------------------*/
int IPXConnectToServer(const char *strAddr)
{
	int numsent;
	UDPpacket regPacket;
	IPXHeader regHeader;
    Bits result;
    Bit32u ticks, elapsed;

	// First, determine where the target really is.
	if(SDLNet_ResolveHost(&ipxServConnIp, strAddr, (Bit16u)udpPort)) {
        LOG_MSG("IPX: Unable resolve connection to server\n");
        return 0;
    }

	regHeader.src.socket[0] = 0x86;
    regHeader.src.socket[1] = 0x9C;
    regHeader.dest.socket[0] = 0x86;
    regHeader.dest.socket[1] = 0x9C;

    // Determined the proper IP address of the target
    // Now, generate the MAC address we'll use for this computer.
    // This is made by zeroing out the first two
    // octets and then using the actual IP address for the last 4 octets.
    //
    // This idea is from the IPX over IP implementation as specified in RFC 1234:
    // http://www.faqs.org/rfcs/rfc1234.html
//TODO: Better MAC?

    // Create an anonymous UDP port
    ipxClientSocket = SDLNet_UDP_Open(0);
    if (!ipxClientSocket) {
        LOG_MSG("IPX: Unable to open socket\n");
        return 0;
    }

    // Bind UDP port to address to channel
    UDPChannel = SDLNet_UDP_Bind(ipxClientSocket, -1, &ipxServConnIp);
    if (UDPChannel == -1) {
        LOG_MSG("IPX: Channel not bound!");
        SDLNet_UDP_Close(ipxClientSocket);
        return 0;
    }

    //ipxClientSocket = SDLNet_TCP_Open(&ipxServConnIp);
    // Reset checksum to 0xFFFF and set the length to the proper regHeader size
    SDLNet_Write16(0xffff, regHeader.checkSum);
    SDLNet_Write16(sizeof(regHeader), regHeader.length);

    // An Echo packet with zeroed dest and src is a server registration packet
    // Zero the destination
    SDLNet_Write32(0, regHeader.dest.network);
    regHeader.dest.addr.byIP.host = 0x0;
    regHeader.dest.addr.byIP.port = 0x0;
    SDLNet_Write16(0x2, regHeader.dest.socket);

    // Zero the source
    SDLNet_Write32(0, regHeader.src.network);
    regHeader.src.addr.byIP.host = 0x0;
    regHeader.src.addr.byIP.port = 0x0;
    SDLNet_Write16(0x2, regHeader.src.socket);
    regHeader.transControl = 0; // echo packet

    // Now setup the packet to send with just the header
    regPacket.data = (Uint8 *)&regHeader;
    regPacket.len = sizeof(regHeader);
    regPacket.maxlen = sizeof(regHeader);
    regPacket.channel = UDPChannel;

    // Send registration echo packet to server.  If server doesn't get
    // this, client will not be registered
    numsent = SDLNet_UDP_Send(ipxClientSocket, regPacket.channel, &regPacket);

    if(!numsent) {
        // Failed to send packet (didn't even go out!)
        LOG_MSG("IPX: Unable to connect to server: %s\n", SDLNet_GetError());
        SDLNet_UDP_Close(ipxClientSocket);
        return 0;
    }

    // Wait for return packet from server.  Might still get lost.
    // This will contain our IPX address and port num
    ticks = TickerGet();

    while(true) {
        // Has 5 seconds paccked?
        elapsed = TickerGet() - ticks;
        if(elapsed > 5*TICKS_PER_SECOND) {
            // Yes.  Timeout, stop here
            LOG_MSG("Timeout connecting to server at %s\n", strAddr);
            SDLNet_UDP_Close(ipxClientSocket);

            return 0;
        }

        // See if we got a response
        //CALLBACK_Idle();
        result = SDLNet_UDP_Recv(ipxClientSocket, &regPacket);
        if (result != 0) {
            // Yes, got a response on the UDP port.
            // Record the send's information as the net node and number (basically UDP IP and port)
            memcpy(localIpxAddr.netnode, regHeader.dest.addr.byNode.node, sizeof(localIpxAddr.netnode));
            memcpy(localIpxAddr.netnum, regHeader.dest.network, sizeof(localIpxAddr.netnum));
            break;
        }

    }

    LOG_MSG("IPX: Connected to server.  IPX address is %d:%d:%d:%d:%d:%d\n", CONVIPX(localIpxAddr.netnode));

    //incomingPacket.connected = true;
    //TIMER_AddTickHandler(&IPXClientPoll);
    return 1;
}

} // extern C
