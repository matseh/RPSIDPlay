#ifndef RPIPCMESSAGE_H
#define RPIPCMESSAGE_H

#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct _RPIPCMessage;
typedef struct _RPIPCMessage *RPIPCMessageRef;

RPIPCMessageRef RPIPCMessageCreate(void);
void RPIPCMessageDelete(RPIPCMessageRef message);

void RPIPCMessageClear(RPIPCMessageRef message);

uint32_t RPIPCMessageID(RPIPCMessageRef message);
void RPIPCMessageSetID(RPIPCMessageRef message, uint32_t messageID);

void *RPIPCMessageContent(RPIPCMessageRef message);

size_t RPIPCMessageContentLength(RPIPCMessageRef message);
void RPIPCMessageSetContentLength(RPIPCMessageRef message, size_t newContentLength);

size_t RPIPCMessageMaxContentLength(RPIPCMessageRef message);

int RPIPCMessageSend(RPIPCMessageRef message, FILE *stream);
int RPIPCMessageReceive(RPIPCMessageRef message, FILE *stream);

int RPIPCMessageWriteBlob(RPIPCMessageRef message, const void *blob, size_t blobSize);
void *RPIPCMessageReadBlob(RPIPCMessageRef message, size_t blobSize);

int RPIPCMessageWriteString(RPIPCMessageRef message, const char *stringValue);
const char *RPIPCMessageReadString(RPIPCMessageRef message);

#ifdef __cplusplus
}
#endif

#endif
