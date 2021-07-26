 /*
 * This file is part of RPSIDPlay, a libsidplayfp based SID player.
 *
 * Copyright 2015, 2021 Mats Eirik Hansen <mats.hansen@triumph.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version   .
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

#include <cstdio>
#include <iostream>
#include <fstream>
#include "RPIPCMessage.h"
#include "RPSIDPlayMessages.h"
#include "sidplayfp/sidplayfp.h"
#include "sidplayfp/SidInfo.h"
#include "sidplayfp/SidTune.h"
#include "sidplayfp/SidTuneInfo.h"
#include "sidplayfp/builders/resid.h"
#include "sidplayfp/builders/residfp.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

static char *loadROM(const char *path, size_t romSize)
{
    char *buffer = 0;

    std::ifstream is(path, std::ios::binary);
    
    if(is.good())
    {
        buffer = new char[romSize];

        is.read(buffer, romSize);
    }
    
    is.close();
    
    return buffer;
}


class RPSIDPlay
{
public:
    RPSIDPlay();
    ~RPSIDPlay();
    void run(void);

private:
    void loadROMs(RPIPCMessageRef request);
    void loadFile(RPSIDPlayLoadFileRequestContent *requestContent);
    void setConfig(RPSIDPlaySetConfigRequestContent *requestContent);
    void readAudioData(RPSIDPlayReadAudioDataRequestContent *requestContent);
    void getSongInfo(RPSIDPlayGetSongInfoRequestContent *requestContent);
    void playSong(RPSIDPlayPlaySongRequestContent *requestContent);
    void muteChannels(RPSIDPlayMuteChannelsRequestContent *requestContent);
    void muteEngineChannels();
    int setDefaultC64Model(RPC64Model defaultC64Model);
    int setForceC64Model(uint32_t forceC64Model);
    int setDefaultSIDModel(RPSIDModel defaultSIDModel);
    int setForceSIDModel(uint32_t forceSIDModel);
    int setPlaybackMode(RPSIDPlaybackMode playbackMode);
    int setSIDEmulator(RPSIDEmulator sidEmulator);
    int setSamplingMethod(RPSIDSamplingMethod samplingMethod);
    int setFastSampling(uint32_t fastSampling);

    sidplayfp m_engine;
    SidTune *m_tune;
    SidConfig m_sidConfig;
    RPIPCMessageRef m_request;
    RPIPCMessageRef m_response;
    uint32_t m_mutedChannels;
};

RPSIDPlay::RPSIDPlay()
{
    m_tune = 0;

    m_request  = RPIPCMessageCreate();
    m_response = RPIPCMessageCreate();
}

RPSIDPlay::~RPSIDPlay()
{
    if(m_tune)
    {
        delete m_tune;
    }

    RPIPCMessageDelete(m_request);
    RPIPCMessageDelete(m_response);
}

void RPSIDPlay::run(void)
{
    int success = TRUE;

    do
    {
        success = RPIPCMessageReceive(m_request, stdin);

        if(success)
        {
            void *requestContent = RPIPCMessageContent(m_request);

            switch(RPIPCMessageID(m_request))
            {
                case RPSIDPlayMessageIDLoadROMsRequest:
                    loadROMs(m_request);
                    break;

                case RPSIDPlayMessageIDLoadFileRequest:
                    loadFile((RPSIDPlayLoadFileRequestContent *) requestContent);
                    break;

                case RPSIDPlayMessageIDSetConfigRequest:
                    setConfig((RPSIDPlaySetConfigRequestContent *) requestContent);
                    break;

                case RPSIDPlayMessageIDReadAudioDataRequest:
                    readAudioData((RPSIDPlayReadAudioDataRequestContent *) requestContent);
                    break;

                case RPSIDPlayMessageIDGetSongInfoRequest:
                    getSongInfo((RPSIDPlayGetSongInfoRequestContent *) requestContent);
                    break;

                case RPSIDPlayMessageIDPlaySongRequest:
                    playSong((RPSIDPlayPlaySongRequestContent *) requestContent);
                    break;

                case RPSIDPlayMessageIDMuteChannelsRequest:
                    muteChannels((RPSIDPlayMuteChannelsRequestContent *) requestContent);
                    break;

                default:
                    std::cerr << "RPSIPDPlay: Unknown request ID: " << RPIPCMessageID(m_request) << std::endl;
                    success = FALSE;
                    break;
            }
        }

    } while(success);    
}

void RPSIDPlay::loadROMs(RPIPCMessageRef request)
{
    int success = FALSE;

    const char *kernalPath  = 0;
    const char *basicPath   = 0;
    const char *chargenPath = 0;

    kernalPath = RPIPCMessageReadString(request);

    if(kernalPath)
    {
        basicPath = RPIPCMessageReadString(request);
    }

    if(basicPath)
    {
        chargenPath = RPIPCMessageReadString(request);
    }

    if(kernalPath && basicPath && chargenPath)
    {
        char *kernal  = loadROM(kernalPath, 8192);
        char *basic   = loadROM(basicPath, 8192);
        char *chargen = loadROM(chargenPath, 4096);

        if(kernal && basic && chargen)
        {
            m_engine.setRoms((const uint8_t *) kernal, (const uint8_t *) basic, (const uint8_t *) chargen);

            success = TRUE;
        }

        delete [] kernal;
        delete [] basic;
        delete [] chargen;            
    }

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDLoadROMsResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayLoadROMsResponseContent));

    RPSIDPlayLoadROMsResponseContent *responseContent = (RPSIDPlayLoadROMsResponseContent *) RPIPCMessageContent(m_response);
 
    responseContent->success = success;

    RPIPCMessageSend(m_response, stdout);
}

void RPSIDPlay::loadFile(RPSIDPlayLoadFileRequestContent *requestContent)
{
    int success = FALSE;

    RPSIDPlayLoadFileResponseContent *responseContent = (RPSIDPlayLoadFileResponseContent *) RPIPCMessageContent(m_response);

    std::cerr << "RPSIDPlay: Loading file: " << requestContent->filePath << std::endl;

    if(m_tune)
    {
        m_engine.load(0);

        delete m_tune;
    }

    m_tune = new SidTune(requestContent->filePath);

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDLoadFileResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayLoadFileResponseContent));
 
    if(m_tune && m_tune->getStatus())
    {
        success = TRUE;

        const SidTuneInfo *tuneInfo = m_tune->getInfo();

        responseContent->songCount    = tuneInfo->songs();
        responseContent->firstSubsong = tuneInfo->startSong();

        success = RPIPCMessageWriteString(m_response, m_tune->createMD5());
    }

    responseContent->success = success;

    RPIPCMessageSend(m_response, stdout);

}

int RPSIDPlay::setDefaultC64Model(RPC64Model defaultC64Model)
{
    int success = TRUE;

    switch(defaultC64Model)
    {
        case RPC64ModelPAL:
            m_sidConfig.defaultC64Model = SidConfig::PAL;
            break;

        case RPC64ModelNTSC:
            m_sidConfig.defaultC64Model = SidConfig::NTSC;
            break;

        case RPC64ModelOldNTSC:
            m_sidConfig.defaultC64Model = SidConfig::OLD_NTSC;
            break;

        case RPC64ModelDrean:
            m_sidConfig.defaultC64Model = SidConfig::DREAN;
            break;

        default:
            std::cerr << "RPSIDPlay: Illegal defaultC64Model: " << defaultC64Model << std::endl;
            success = FALSE;
            break;
    }

    return success;
}

int RPSIDPlay::setForceC64Model(uint32_t forceC64Model)
{
    if(forceC64Model)
    {
        m_sidConfig.forceC64Model = true;
    }
    else
    {
        m_sidConfig.forceC64Model = false;        
    }

    return TRUE;
}

int RPSIDPlay::setDefaultSIDModel(RPSIDModel defaultSIDModel)
{
    int success = TRUE;

    switch(defaultSIDModel)
    {
        case RPSIDModel6581:
            m_sidConfig.defaultSidModel = SidConfig::MOS6581;
            break;

        case RPSIDModel8580:
            m_sidConfig.defaultSidModel = SidConfig::MOS8580;
            break;            

        default:
            std::cerr << "RPSIDPlay: Illegal defaultSIDModel: " << defaultSIDModel;
            success = FALSE;
            break;
    }

    return success;
}

int RPSIDPlay::setForceSIDModel(uint32_t forceSIDModel)
{
    if(forceSIDModel)
    {
        m_sidConfig.forceSidModel = true;
    }
    else
    {
        m_sidConfig.forceSidModel = false;        
    }

    return TRUE;
}

int RPSIDPlay::setPlaybackMode(RPSIDPlaybackMode playbackMode)
{
    int success = TRUE;

    switch(playbackMode)
    {
        case RPSIDPlaybackModeMono:
            m_sidConfig.playback = SidConfig::MONO;
            break;

        case RPSIDPlaybackModeStereo:
            m_sidConfig.playback = SidConfig::STEREO;
            break;

        default:
            std::cerr << "RPSIDPlay: Illegal playbackMode: " << playbackMode;
            success = FALSE;
            break;
    }

    return success;
}

int RPSIDPlay::setSIDEmulator(RPSIDEmulator sidEmulator)
{
    int success = TRUE;

    switch(sidEmulator)
    {
        case RPSIDEmulatorReSID:
            {
                // Set up a SID builder
                ReSIDBuilder *sidEmulationBuilder = new ReSIDBuilder("RPSIDPlay");

                // Create SID emulators
                sidEmulationBuilder->create(m_engine.info().maxsids());

                // Check if builder is ok
                if(!sidEmulationBuilder->getStatus())
                {
                    std::cerr << sidEmulationBuilder->error() << std::endl;
                    success = FALSE;
                }
                else
                {
                    m_sidConfig.sidEmulation = sidEmulationBuilder;
                }                
            }
            break;

        case RPSIDEmulatorReSIDFP:
            {
                // Set up a SID builder
                ReSIDfpBuilder *sidEmulationBuilder = new ReSIDfpBuilder("RPSIDPlay");

                // Create SID emulators
                sidEmulationBuilder->create(m_engine.info().maxsids());

                // Check if builder is ok
                if(!sidEmulationBuilder->getStatus())
                {
                    std::cerr << sidEmulationBuilder->error() << std::endl;
                    success = FALSE;
                }
                else
                {
                    m_sidConfig.sidEmulation = sidEmulationBuilder;
                }                
            }
            break;

        case RPSIDEmulatorHardSID:
            success = FALSE;
            break;

        default:
            std::cerr << "RPSIDPlay: Illegal sidEmulator: " << sidEmulator;
            success = FALSE;
            break;
    }

    return success;
}

int RPSIDPlay::setSamplingMethod(RPSIDSamplingMethod samplingMethod)
{
    int success = TRUE;

    switch(samplingMethod)
    {
        case RPSIDSamplingMethodInterpolate:
            m_sidConfig.samplingMethod = SidConfig::INTERPOLATE;
            break;

        case RPSIDSamplingMethodResampleInterpolate:
            m_sidConfig.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
            break;            

        default:
            std::cerr << "RPSIDPlay: Illegal samplingMethod: " << samplingMethod;
            success = FALSE;
            break;
    }

    return success;
}

int RPSIDPlay::setFastSampling(uint32_t fastSampling)
{
    if(fastSampling)
    {
        m_sidConfig.fastSampling = true;
    }
    else
    {
        m_sidConfig.fastSampling = false;        
    }

    return TRUE;
}

void RPSIDPlay::setConfig(RPSIDPlaySetConfigRequestContent *requestContent)
{
    int success = FALSE;

    RPSIDPlaySetConfigResponseContent *responseContent = (RPSIDPlaySetConfigResponseContent *) RPIPCMessageContent(m_response);

    success = setDefaultC64Model(requestContent->defaultC64Model);

    if(success)
    {
        success = setForceC64Model(requestContent->forceC64Model);
    }

    if(success)
    {
        success = setDefaultSIDModel(requestContent->defaultSIDModel);
    }

    if(success)
    {
        success = setForceSIDModel(requestContent->forceSIDModel);
    }

    if(success)
    {
        success = setPlaybackMode(requestContent->playbackMode);
    }

    if(success)
    {
        m_sidConfig.frequency = requestContent->sampleRate;

        success = setSIDEmulator(requestContent->sidEmulator);
    }

    if(success)
    {
        success = setSamplingMethod(requestContent->samplingMethod);
    }

    if(success)
    {
        success = setFastSampling(requestContent->fastSampling);
    }

    if(!m_engine.config(m_sidConfig))
    {
        std::cerr <<  m_engine.error() << std::endl;
        success = FALSE;
    }

    responseContent->success = success;

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDSetConfigResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlaySetConfigResponseContent));
    RPIPCMessageSend(m_response, stdout);
}

void RPSIDPlay::readAudioData(RPSIDPlayReadAudioDataRequestContent *requestContent)
{
    RPSIDPlayReadAudioDataResponseContent *responseContent = (RPSIDPlayReadAudioDataResponseContent *) RPIPCMessageContent(m_response);

    uint32_t sampleCount = requestContent->sampleCount;

    uint32_t maxSampleCount = (RPIPCMessageMaxContentLength(m_response) - sizeof(RPSIDPlayReadAudioDataResponseContent)) / sizeof(int16_t);

    if(sampleCount > maxSampleCount)
    {
        sampleCount = maxSampleCount;
    }

    responseContent->sampleCount = m_engine.play((short *) &responseContent[1], sampleCount);

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDReadAudioDataResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayReadAudioDataResponseContent) + (sizeof(int16_t) * responseContent->sampleCount));
    RPIPCMessageSend(m_response, stdout);
}

RPSIDCompatibility convertCompatibility(SidTuneInfo::compatibility_t compatibility)
{
    switch(compatibility)
    {
        case SidTuneInfo::COMPATIBILITY_C64:
            return RPSIDCompatibilityC64;

        case SidTuneInfo::COMPATIBILITY_PSID:
            return RPSIDCompatibilityPSID;

        case SidTuneInfo::COMPATIBILITY_R64:
            return RPSIDCompatibilityR64;

        case SidTuneInfo::COMPATIBILITY_BASIC:
            return RPSIDCompatibilityBASIC;
    }    

    return RPSIDCompatibilityC64;
}

RPSIDSongSpeed convertSongSpeed(int songSpeed)
{
    switch(songSpeed)
    {
        case SidTuneInfo::SPEED_VBI:
            return RPSIDSongSpeedVerticalBlank;

        case SidTuneInfo::SPEED_CIA_1A:
            return RPSIDSongSpeedCIA;
    }

    return RPSIDSongSpeedVerticalBlank;
}

RPSIDClockSpeed convertClockSpeed(SidTuneInfo::clock_t clockSpeed)
{
    switch(clockSpeed)
    {
        case SidTuneInfo::CLOCK_UNKNOWN:
            return RPSIDClockSpeedUnknown;

        case SidTuneInfo::CLOCK_PAL:
            return RPSIDClockSpeedPAL;

        case SidTuneInfo::CLOCK_NTSC:
            return RPSIDClockSpeedNTSC;

        case SidTuneInfo::CLOCK_ANY:
            return RPSIDClockSpeedAny;
    }

    return RPSIDClockSpeedUnknown;
}

RPSIDModel convertSIDModel(SidTuneInfo::model_t sidModel)
{
    switch(sidModel)
    {
        case SidTuneInfo::SIDMODEL_UNKNOWN:
            return RPSIDModelUnknown;

        case SidTuneInfo::SIDMODEL_6581:
            return RPSIDModel6581;

        case SidTuneInfo::SIDMODEL_8580:
            return RPSIDModel8580;

        case SidTuneInfo::SIDMODEL_ANY:
            return RPSIDModelAny;

    }

    return RPSIDModelUnknown;
}

void RPSIDPlay::getSongInfo(RPSIDPlayGetSongInfoRequestContent *requestContent)
{
    int success = TRUE;

    RPSIDPlayGetSongInfoResponseContent *responseContent = (RPSIDPlayGetSongInfoResponseContent *) RPIPCMessageContent(m_response);

    const SidTuneInfo *tuneInfo = m_tune->getInfo(requestContent->subsong);

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDGetSongInfoResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayGetSongInfoResponseContent));

    if(tuneInfo)
    {
        responseContent->loadAddress        = tuneInfo->loadAddr();
        responseContent->initAddress        = tuneInfo->initAddr();
        responseContent->playAddress        = tuneInfo->playAddr();
        responseContent->sidCount           = tuneInfo->sidChips();
        responseContent->infoStringCount    = tuneInfo->numberOfInfoStrings();
        responseContent->commentStringCount = tuneInfo->numberOfCommentStrings();
        responseContent->compatibility      = convertCompatibility(tuneInfo->compatibility());
        responseContent->songSpeed          = convertSongSpeed(tuneInfo->songSpeed());
        responseContent->clockSpeed         = convertClockSpeed(tuneInfo->clockSpeed());

        for(unsigned int sidIndex = 0; success && (sidIndex < responseContent->sidCount); sidIndex++)
        {
            RPSIDInfo sidInfo =
            {
                .baseAddress = tuneInfo->sidChipBase(sidIndex),
                .model       = tuneInfo->sidModel(sidIndex)
            };
            
            success = RPIPCMessageWriteBlob(m_response, &sidInfo, sizeof(sidInfo));
        }
        
        for(unsigned int stringIndex = 0; success && (stringIndex < responseContent->infoStringCount); stringIndex++)
        {
            success = RPIPCMessageWriteString(m_response, tuneInfo->infoString(stringIndex));
        }

        for(unsigned int stringIndex = 0; success && (stringIndex < responseContent->commentStringCount); stringIndex++)
        {
            success = RPIPCMessageWriteString(m_response, tuneInfo->commentString(stringIndex));
        }

        if(success)
        {
            success = RPIPCMessageWriteString(m_response, tuneInfo->formatString());
        }
    }
    else
    {
        success = FALSE;
    }

    responseContent->success = success;

    RPIPCMessageSend(m_response, stdout);
}

void RPSIDPlay::playSong(RPSIDPlayPlaySongRequestContent *requestContent)
{
    int success = TRUE;

    unsigned int subsong = requestContent->subsong;

    if(subsong != m_tune->selectSong(subsong))
    {
        std::cerr << "RPSIDPlay: Failed to select subsong: " << subsong << std::endl;

        success = FALSE;
    }
    else if(!m_engine.load(m_tune))
    {
        std::cerr << "RPSIDPlay: Loading song into engine failed with error: " << m_engine.error() << std::endl;

        success = FALSE;        
    }
    else
    {
        muteEngineChannels();
    }

    RPSIDPlayPlaySongResponseContent *responseContent = (RPSIDPlayPlaySongResponseContent *) RPIPCMessageContent(m_response);

    responseContent->success = success;

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDPlaySongResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayPlaySongResponseContent));
    RPIPCMessageSend(m_response, stdout);
}

void RPSIDPlay::muteChannels(RPSIDPlayMuteChannelsRequestContent *requestContent)
{
    int success = TRUE;

    m_mutedChannels = requestContent->mutedChannels;

    muteEngineChannels();

    RPSIDPlayMuteChannelsResponseContent *responseContent = (RPSIDPlayMuteChannelsResponseContent *) RPIPCMessageContent(m_response);

    responseContent->success = success;

    RPIPCMessageSetID(m_response, RPSIDPlayMessageIDMuteChannelsResponse);
    RPIPCMessageSetContentLength(m_response, sizeof(RPSIDPlayMuteChannelsResponseContent));
    RPIPCMessageSend(m_response, stdout);
}

void RPSIDPlay::muteEngineChannels()
{
    for(unsigned int channel = 0; channel < 6; channel++)
    {
        bool channelMuted = (m_mutedChannels & (1 << channel)) ? true : false;

        m_engine.mute(channel / 3, channel % 3, channelMuted);
    }
}

int main(int argc, char const *argv[])
{
    RPSIDPlay sidPlay;

    sidPlay.run();
}
