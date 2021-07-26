 /*
 * Copyright 2015, 2020-2021 Mats Eirik Hansen <mats.hansen@triumph.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdlib.h>
#include <string.h>
#include "RPIPCMessage.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define kRPIPCMessageBufferSize 8129
#define kRPIPCMaxContentLength  (kRPIPCMessageBufferSize - sizeof(RPIPCMessageHeader))

typedef struct
{
    uint32_t id;
    uint32_t contentLength;
} RPIPCMessageHeader;

typedef struct _RPIPCMessage
{
    uint8_t buffer[kRPIPCMessageBufferSize];
    size_t readIndex; /* Index for the next item to be read from the content of a received message. */
} RPIPCMessage;

RPIPCMessageRef RPIPCMessageCreate(void)
{
    RPIPCMessageRef message = malloc(sizeof(RPIPCMessage));

    RPIPCMessageClear(message);

    return message;
}

void RPIPCMessageDelete(RPIPCMessageRef message)
{
    free(message);
}

void RPIPCMessageClear(RPIPCMessageRef message)
{
    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        messageHeader->id            = 0;
        messageHeader->contentLength = 0;
    }
}

uint32_t RPIPCMessageID(RPIPCMessageRef message)
{
    uint32_t messageID = 0;

    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        messageID = messageHeader->id;        
    }

    return messageID;
}

void RPIPCMessageSetID(RPIPCMessageRef message, uint32_t messageID)
{
    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        messageHeader->id = messageID;        
    }
}

void *RPIPCMessageContent(RPIPCMessageRef message)
{
    void *content = NULL;

    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        content = &messageHeader[1];
    }

    return content;
}

size_t RPIPCMessageContentLength(RPIPCMessageRef message)
{
    size_t contentLength = 0;

    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        contentLength = messageHeader->contentLength;
    }

    return contentLength;
}

void RPIPCMessageSetContentLength(RPIPCMessageRef message, size_t newContentLength)
{
    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        messageHeader->contentLength = (uint32_t) newContentLength;        
    }
}

size_t RPIPCMessageMaxContentLength(RPIPCMessageRef message)
{
    return kRPIPCMaxContentLength;
}

int RPIPCMessageSend(RPIPCMessageRef message, FILE *stream)
{
    int success = FALSE;

    if(message && stream)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        if(1 == fwrite(message->buffer,
                       sizeof(RPIPCMessageHeader) + messageHeader->contentLength,
                       1,
                       stream))
        {
            fflush(stream);

            success = TRUE;
        }
    }

    return success;
}

int RPIPCMessageReceive(RPIPCMessageRef message, FILE *stream)
{
    int success = FALSE;

    if(message && stream)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        if(1 == fread(message->buffer,
                      sizeof(RPIPCMessageHeader),
                      1,
                      stream))
        {
            if(1 == fread(&messageHeader[1],
                          messageHeader->contentLength,
                          1,
                          stream))
            {
                message->readIndex = 0;

                success = TRUE;
            }
        }
    }

    return success;
}

int RPIPCMessageWriteBlob(RPIPCMessageRef message, const void *blob, size_t blobSize)
{
    int success = FALSE;

    if(message && blob)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        if((messageHeader->contentLength + blobSize) <= kRPIPCMaxContentLength)
        {
            memcpy(&message->buffer[sizeof(RPIPCMessageHeader) + messageHeader->contentLength], blob, blobSize);

            messageHeader->contentLength += blobSize;

            success = TRUE;
        }
    }

    return success;
}

void *RPIPCMessageReadBlob(RPIPCMessageRef message, size_t blobSize)
{
    void *blob = NULL;

    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        if((message->readIndex + blobSize) <= messageHeader->contentLength)
        {
            blob = &message->buffer[sizeof(RPIPCMessageHeader) + message->readIndex];

            message->readIndex += blobSize;
        }        
    }

    return blob;
}

int RPIPCMessageWriteString(RPIPCMessageRef message, const char *stringValue)
{
    int success = FALSE;

    if(message)
    {
        if(stringValue)
        {
            success = RPIPCMessageWriteBlob(message, stringValue, strlen(stringValue) + 1);
        }
        else
        {
            success = RPIPCMessageWriteBlob(message, "", 1);
        }
    }

    return success;
}

const char *RPIPCMessageReadString(RPIPCMessageRef message)
{
    const char *stringValue = NULL;

    if(message)
    {
        RPIPCMessageHeader *messageHeader = (RPIPCMessageHeader *) message->buffer;

        size_t contentIndex = message->readIndex;

        while((contentIndex < messageHeader->contentLength) && (message->buffer[sizeof(RPIPCMessageHeader) + contentIndex] != 0))
        {
            contentIndex++;
        }

        if(contentIndex < messageHeader->contentLength)
        {
            stringValue = (const char *) &message->buffer[sizeof(RPIPCMessageHeader) + message->readIndex];

            message->readIndex = contentIndex + 1;
        }        
    }

    return stringValue;
}
