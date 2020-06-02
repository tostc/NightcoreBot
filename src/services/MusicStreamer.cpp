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

#include <services/MusicStreamer.hpp>
#include <services/MessageQueue.hpp>

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#define STS_MIXER_IMPLEMENTATION
#include <sts_mixer.h>

#include <Log.hpp>

void MonoToStereo(int16_t *Out, const int16_t *In, int Samples)
{
    for(; Samples--;)
    {
        int16_t Data = *In++;

        *Out++ = Data;
        *Out++ = Data;
    }
}

CMusicStreamer::CMusicStreamer(std::shared_ptr<CMusicInfo> Info) : m_Info(Info), m_Data(nullptr), m_LoadedSamples(0), m_ProgressedSamples(0)
{
    if(drwav_init_file(&m_Handle, std::string(Info->Path + ".wav").c_str(), nullptr))
    {
        m_STSStream.userdata = this;
        m_STSStream.callback = refill_stream;
        m_STSStream.sample.frequency = m_Handle.sampleRate;
        m_STSStream.sample.audio_format = STS_MIXER_SAMPLE_FORMAT_16;
        m_STSStream.sample.length = SAMPLES * 2;   //Samples mal Channel oder eher mal 2 da zwei Channel,
        m_STSStream.sample.data = new int16_t[SAMPLES * 2]; //malloc(SAMPLES * 2 * sizeof(float));
        refill_stream(&m_STSStream.sample, this);

        int Freq = DEFAULT_FREQ;
        m_SampleRate = m_Handle.sampleRate;

        if(Info->Speed == SpeedType::ANTI)
        {
            if(!Info->IsNightcore)
                m_STSStream.sample.frequency *= .85f;
            else
                m_STSStream.sample.frequency *= .7f;
        }
        else if(!Info->IsNightcore)
        {
            //Nightcore :D
            if(Info->Speed == SpeedType::NORMAL)
                m_STSStream.sample.frequency *= 1.2f;
            else if(Info->Speed == SpeedType::DOUBLE)
                m_STSStream.sample.frequency *= 1.4f;
            //Anti-Nightcore
            // Freq = m_Handle.sampleRate * 1.2f;
            // if(Freq < DEFAULT_FREQ)
            //     Freq = DEFAULT_FREQ * 1.2f;
        }
        else if(Info->Speed == SpeedType::DOUBLE)
            m_STSStream.sample.frequency *= 1.2f;

        m_Data = new int16_t[BUF_SIZE];

        sts_mixer_init(&m_Mixer, Freq, STS_MIXER_SAMPLE_FORMAT_16);
        sts_mixer_play_stream(&m_Mixer, &m_STSStream, 1);

        SMessage msg = {Info->Channel, "Now Playing", Info->Name, Info->URL};
        MessageQueue->PrintMessage(msg);
    }
}

uint32_t CMusicStreamer::OnRead(uint16_t *Buf, uint32_t Samples)
{
    sts_mixer_mix_audio(&m_Mixer, Buf, Samples);

    if(m_LoadedSamples > Samples)
        return Samples;

    return m_LoadedSamples;  
}

CMusicStreamer::~CMusicStreamer()
{
    drwav_uninit(&m_Handle);
    delete[] m_Data;

    remove(std::string(m_Info->Path + ".wav").c_str());
    llog << ldebug << m_Info->Path << ".wav Deleted!" << lendl;
}

void CMusicStreamer::refill_stream(sts_mixer_sample_t* sample, void* userdata)
{
    CMusicStreamer *This = (CMusicStreamer*)userdata;
    int Samples = sample->length / 2;
    int16_t *Buf = (int16_t*)sample->data;

    if(This->m_Handle.channels == 2)
    {
        drwav_uint64 Num = drwav_read_pcm_frames_s16(&This->m_Handle, Samples, Buf);
        This->m_LoadedSamples = Num;

        if (Num < Samples) 
            memset(Buf + Num * 2, 0, (Samples * 2 - Num * 2) * sizeof(int16_t));
    }
    else if(This->m_Handle.channels == 1)
    {
        int16_t *Data = new int16_t[Samples];

        drwav_uint64 Num = drwav_read_pcm_frames_s16(&This->m_Handle, Samples, Data);
        This->m_LoadedSamples = Num;

        MonoToStereo(Buf, Data, Samples);

        delete[] Data;

        if (Num < Samples) 
            memset(Buf + Num * 2, 0, (Samples * 2 - Num * 2) * sizeof(int16_t));
    }

    This->m_ProgressedSamples += This->m_LoadedSamples;
}