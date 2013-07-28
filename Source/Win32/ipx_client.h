#ifndef _IPX_CLIENT_H_
#define _IPX_CLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif

int IPXConnectToServer(char const *strAddr);
void IPXSendPacket(char const *p_data, unsigned int size);
int IPXClientPoll(char *p_data, unsigned int *size);
void IPXGetUniqueAddress(unsigned char address[6]);
unsigned char *IPXGetDestinationAddress(void);
void IPXSetDestinationAddress(unsigned char address[6]);
void IPXSetDestinationAddressAll(void);

#ifdef __cplusplus
}
#endif

#endif // _IPX_CLIENT_H_
