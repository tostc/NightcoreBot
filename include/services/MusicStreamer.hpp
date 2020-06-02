/*
 * MIT License
 *
 * Copyright (c) 2020 Christian Tost
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MUSICSTREAMER_HPP
#define MUSICSTREAMER_HPP

#include <dr_wav.h>
#include <sts_mixer.h>
#include <services/DownloadQueue.hpp>
#include <controller/IAudioSource.hpp>
#include <atomic>

class CMusicStreamer : public DiscordBot::IAudioSource
{
    public:
        CMusicStreamer(std::shared_ptr<CMusicInfo> Info);

        uint32_t OnRead(uint16_t *Buf, uint32_t Samples) override;

        size_t GetProgressedSamples()
        {
            return m_ProgressedSamples;
        }

        size_t GetTotalSamples()
        {
            return m_Handle.totalPCMFrameCount;
        }

        int GetBitrate()
        {
            return m_SampleRate;
        }

        std::shared_ptr<CMusicInfo> GetMusic()
        {
            return m_Info;
        }

        virtual ~CMusicStreamer();
    private:
        static void refill_stream(sts_mixer_sample_t* sample, void* userdata);
        static const int SAMPLES = 4096;
        static const int DEFAULT_FREQ = 48000;
        static const int BUF_SIZE = 1920;

        std::shared_ptr<CMusicInfo> m_Info;
        drwav m_Handle;
        sts_mixer_t m_Mixer;
        sts_mixer_stream_t m_STSStream;

        int16_t *m_Data;
        std::atomic<size_t> m_LoadedSamples;
        std::atomic<size_t> m_ProgressedSamples;
        std::atomic<uint32_t> m_SampleRate;
};

#endif //MUSICSTREAMER_HPP