#ifndef RPSIDPLAYMESSAGES_H
#define RPSIDPLAYMESSAGES_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
    RPSIDPlayMessageIDLoadFileRequest       = 1,
    RPSIDPlayMessageIDLoadFileResponse      = 2,
    RPSIDPlayMessageIDSetConfigRequest      = 3,
    RPSIDPlayMessageIDSetConfigResponse     = 4,
    RPSIDPlayMessageIDReadAudioDataRequest  = 5,
    RPSIDPlayMessageIDReadAudioDataResponse = 6,
    RPSIDPlayMessageIDGetSongInfoRequest    = 7,
    RPSIDPlayMessageIDGetSongInfoResponse   = 8,
    RPSIDPlayMessageIDPlaySongRequest       = 9,
    RPSIDPlayMessageIDPlaySongResponse      = 10,
    RPSIDPlayMessageIDLoadROMsRequest       = 11,
    RPSIDPlayMessageIDLoadROMsResponse      = 12,
    RPSIDPlayMessageIDMuteChannelsRequest   = 13,
    RPSIDPlayMessageIDMuteChannelsResponse  = 14
}   RPSIDPlayMessageID;

typedef struct
{
    char filePath[256];
} RPSIDPlayLoadFileRequestContent;

typedef struct
{
    uint32_t success;
    uint32_t songCount;
    uint32_t firstSubsong;
    /* Followed by MD5 string. */
} RPSIDPlayLoadFileResponseContent;

enum
{
    RPSIDPlaybackModeMono   = 1,
    RPSIDPlaybackModeStereo = 2
};

typedef uint32_t RPSIDPlaybackMode;

enum
{
    RPSIDModelUnknown = 0,
    RPSIDModel6581    = 1,
    RPSIDModel8580    = 2,
    RPSIDModelAny     = 3
};

typedef uint32_t RPSIDModel;

enum
{
    RPC64ModelPAL     = 1,
    RPC64ModelNTSC    = 2,
    RPC64ModelOldNTSC = 3,
    RPC64ModelDrean   = 4
};

typedef uint32_t RPC64Model;

enum
{
    RPSIDSamplingMethodInterpolate = 1,
    RPSIDSamplingMethodResampleInterpolate = 2
};

typedef uint32_t RPSIDSamplingMethod;

enum
{
    RPSIDEmulatorReSID   = 1,
    RPSIDEmulatorReSIDFP = 2,
    RPSIDEmulatorHardSID = 3
};

typedef uint32_t RPSIDEmulator;

enum
{
    RPSIDCompatibilityC64   = 1,
    RPSIDCompatibilityPSID  = 2,
    RPSIDCompatibilityR64   = 3,
    RPSIDCompatibilityBASIC = 4
};

typedef uint32_t RPSIDCompatibility;

enum
{
    RPSIDSongSpeedVerticalBlank = 1,
    RPSIDSongSpeedCIA           = 2
};

typedef uint32_t RPSIDSongSpeed;

enum
{
    RPSIDClockSpeedUnknown = 1,
    RPSIDClockSpeedPAL     = 2,
    RPSIDClockSpeedNTSC    = 3,
    RPSIDClockSpeedAny     = 4
};

typedef uint32_t RPSIDClockSpeed;

typedef struct
{
    RPC64Model          defaultC64Model;
    uint32_t            forceC64Model;
    RPSIDModel          defaultSIDModel;
    uint32_t            forceSIDModel;
    RPSIDPlaybackMode   playbackMode;
    uint32_t            sampleRate;
    RPSIDEmulator       sidEmulator;
    RPSIDSamplingMethod samplingMethod;
    uint32_t            fastSampling;
} RPSIDPlaySetConfigRequestContent;

typedef struct
{
    uint32_t success;
} RPSIDPlaySetConfigResponseContent;

typedef struct
{
    uint32_t sampleCount;
} RPSIDPlayReadAudioDataRequestContent;

typedef struct
{
    uint32_t success;
    uint32_t sampleCount;
    /* Followed by actual waveform data. */
} RPSIDPlayReadAudioDataResponseContent;

typedef struct
{
    uint32_t subsong;
} RPSIDPlayGetSongInfoRequestContent;

typedef struct
{
    uint32_t baseAddress;
    RPSIDModel model;
} RPSIDInfo;

typedef struct
{
    uint32_t           success;
    uint32_t           loadAddress;
    uint32_t           initAddress;
    uint32_t           playAddress;
    RPSIDCompatibility compatibility;
    RPSIDSongSpeed     songSpeed;
    RPSIDClockSpeed    clockSpeed;
    uint32_t           sidCount;
    uint32_t           infoStringCount;
    uint32_t           commentStringCount;
    RPSIDInfo          sidInfo[0]; /* The actual number of entries is sidCount. */
    /* Followed by the info strings, the comment strings and the format string. */
} RPSIDPlayGetSongInfoResponseContent;

typedef struct
{
    uint32_t subsong;
} RPSIDPlayPlaySongRequestContent;

typedef struct
{
    uint32_t success;
} RPSIDPlayPlaySongResponseContent;

/* There is no RPSIDPlayLoadROMsRequestContent because in only consists of 3 strings (the kernal, basic and chargen ROM paths). */

typedef struct
{
    uint32_t success;
} RPSIDPlayLoadROMsResponseContent;

typedef struct
{
    uint32_t mutedChannels;
} RPSIDPlayMuteChannelsRequestContent;

typedef struct
{
    uint32_t success;
} RPSIDPlayMuteChannelsResponseContent;

#ifdef __cplusplus
}
#endif

#endif
