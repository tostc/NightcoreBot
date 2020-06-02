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

#include <services/DownloadQueue.hpp>
#include <process.hpp>
#include <algorithm>
#include <chrono>
#include <services/MessageQueue.hpp>
#include <tinyformat.h>
#include <time.h>
#include <Log.hpp>
#include <models/LogLockGuard.hpp>
#include <services/MusicStreamer.hpp>

CDownloadQueue::CDownloadQueue(const SSettings &Settings) : DiscordBot::IMusicQueue(), m_Terminate(false), m_Settings(Settings)
{
    m_DownloadCount = 0;
}

void CDownloadQueue::OnUpdate(DiscordBot::SongInfo Info, size_t Index)
{
    auto Song = std::static_pointer_cast<CMusicInfo>(Info);

    Song->URL = GetDownloadParam(Song->URL);
    std::hash<std::string> hash;
    Song->Path = m_Settings.CacheDir + "/" + std::to_string(hash(Song->URL + std::to_string(time(nullptr))));

    if(m_InfoThread.joinable())
        m_InfoThread.join();

    m_InfoThread = std::thread(&CDownloadQueue::AddInfo, this, Song);
}

DiscordBot::AudioSource CDownloadQueue::OnNext(DiscordBot::SongInfo Info)
{
    auto Song = std::static_pointer_cast<CMusicInfo>(Info);
    if(Song->Downloaded)
        return DiscordBot::AudioSource(new CMusicStreamer(std::static_pointer_cast<CMusicInfo>(Info)));

    StartThread();
    return nullptr;
}

void CDownloadQueue::OnFinishPlaying(DiscordBot::SongInfo Info)
{
    auto Song = std::static_pointer_cast<CMusicInfo>(Info);
    Song->Downloaded = false;

    m_DownloadCount--;
}

void CDownloadQueue::OnRemove(DiscordBot::SongInfo Info, size_t Index)
{
    CLogLockGuard guard1(m_MetaLock, "m_MetaLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")");
    m_MetaQueue.erase(m_MetaQueue.begin() + Index);

    if(std::static_pointer_cast<CMusicInfo>(Info)->Downloaded)
        m_DownloadCount--;
}

void CDownloadQueue::AddInfo(std::shared_ptr<CMusicInfo> Info)
{
    std::vector<std::string> Args = {m_Settings.YTDLPath, "-e", "--get-id", "--get-duration", "-r", "10485760", Info->URL};
    bool Error = false;

    //Porn filter.
    if(IsURL(Info->URL))
    {
        std::string URL = Info->URL;
        std::transform(URL.begin(), URL.end(), URL.begin(), [](unsigned char c){ return std::tolower(c); });

        //Porn filter.
        if (URL.find("porn") != std::string::npos || URL.find("xhamster") != std::string::npos || URL.find("redtube") != std::string::npos)
        {
            Info->Error = true;

            SMessage msg = {Info->Channel, "Failed to download", Info->Name.empty() ? Info->URL : Info->Name};
            MessageQueue->PrintMessage(msg);

            return;
        }
    }

    std::string Data;

    TinyProcessLib::Process ytdl(Args, "", 
    [&Data](const char *bytes, size_t n) mutable
    {
        Data += std::string(bytes, n);
    },
    [&Error](const char *bytes, size_t n) mutable
    {
        Error = true;
    });

    if(ytdl.get_exit_status() != 0 || Error)
    {
        Info->Error = Error;

        SMessage msg = {Info->Channel, "Failed to download", Info->Name.empty() ? Info->URL : Info->Name};
        MessageQueue->PrintMessage(msg);

        return;
    }

    size_t Pos = Data.find("\n");
    size_t Len = Data.find("\n", Pos + 1) - (Pos + 1);
    Info->Name = Data.substr(0, Pos);

    if(Pos != std::string::npos && !IsURL(Info->URL))
        Info->URL = "https://www.youtube.com/watch?v=" + Data.substr(Pos + 1, Len);

    std::string Name = Info->Name;
    std::transform(Name.begin(), Name.end(), Name.begin(), [](unsigned char c){ return std::tolower(c); });

    //Quick and dirty Nightcore check.
    if(Name.find("nightcore") != std::string::npos)
        Info->IsNightcore = true;
    else
        Info->IsNightcore = false;

    CLogLockGuard guard1(m_MetaLock, "m_MetaLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")");

    std::string Duration;

    if(Pos != std::string::npos)
    {
        Pos = Data.find("\n", Pos + 1);
        Len = Data.find("\n", Pos + 1) - (Pos + 1);
        Duration = Data.substr(Pos + 1, Len);

        std::string Tmp = Duration;

        if(Tmp.size() == 4)
            Tmp = "00:0" + Tmp;
        else if(Tmp.size() == 5)
            Tmp = "00:" + Tmp;
        else if(Tmp.size() == 7)
            Tmp = "0" + Tmp;

        struct tm tm;
        strptime(Tmp.c_str(), "%H:%M:%S", &tm);
        time_t t = mktime(&tm);

        struct tm tm1;
        strptime("10:00:00", "%H:%M:%S", &tm1);
        time_t t1 = mktime(&tm1);

        if(tm.tm_hour > tm1.tm_hour)
        {
            Info->Error = true;

            SMessage msg = {Info->Channel, "Failed to download! File to big!", Info->Name.empty() ? Info->URL : Info->Name};
            MessageQueue->PrintMessage(msg);

            return;
        }
    }

    m_MetaQueue.push_back({Info->Name, Duration});
    SMessage msg = {Info->Channel, "Added to queue", Info->Name, Info->URL};
    MessageQueue->PrintMessage(msg);

    Info->InfoLoaded = true;
    // StartThread();
}

void CDownloadQueue::StartThread()
{
    if(m_DownloadCount < CACHE_SIZE && GetQueueIndex() <= GetQueueSize())
    {
        if(m_DowloadThread.joinable())
            m_DowloadThread.join();

        m_DowloadThread = std::thread(&CDownloadQueue::Downloader, this);
    }
}

void CDownloadQueue::PrintQueue(DiscordBot::Channel channel)
{
    CLogLockGuard guard(m_MetaLock, "m_MetaLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")");
    if(m_MetaQueue.empty())
    {
        std::string Cmd;
        Cmd = "```nimrod\nNo Data in queue. :(\n```";

        SMessage msg = {channel, "", Cmd, "", false};
        MessageQueue->PrintCodeblock(msg);
        return;
    }

    std::string Q;

    size_t Current = GetQueueIndex();
    if(Current > 0)
        Current--;

    size_t Pos = (Current / QUEUE_PRINT_SIZE) * QUEUE_PRINT_SIZE;
    size_t End = Pos + QUEUE_PRINT_SIZE;
    if(End > m_MetaQueue.size())
        End = m_MetaQueue.size();

    for(size_t i = Pos; i < End; i++)
    {
        if(i == Current)
            Q += "        vvvvvvvv Playing\n";

        std::string Name = m_MetaQueue[i].Name;
        if(Name.length() > 40)
        {
            Name = Name.substr(0, 37);
            Name += "...";
        }

        Q += tfm::format(" %5lu) %-40s %10s", i + 1, Name, m_MetaQueue[i].Duration);

        if(i == Current)
            Q += "\n        ʌʌʌʌʌʌʌʌ Playing\n";
        else
            Q += '\n';
    }

    if(End < m_MetaQueue.size())
    {
        Q += "\n And " + std::to_string(m_MetaQueue.size() - End) + " more!\n";
    }

    SMessage msg = {channel, "", "```nimrod\n" + Q + "```"};
    MessageQueue->PrintCodeblock(msg);
}

CDownloadQueue::~CDownloadQueue() 
{
    m_Terminate = true;
    if(m_DowloadThread.joinable())
        m_DowloadThread.join();

    if(m_InfoThread.joinable())
        m_InfoThread.join();
}

void CDownloadQueue::Downloader()
{
    llog << ldebug << "Downloader started" << lendl;
    while(m_DownloadCount < CACHE_SIZE && GetQueueIndex() + m_DownloadCount <= GetQueueSize())
    {
        size_t Current = GetQueueIndex();
        if(Current > 0)
            Current--;

        if(Current < GetQueueSize())
        {           
            std::shared_ptr<CMusicInfo> Song; 
            for (; Current + m_DownloadCount < Current + m_DownloadCount + CACHE_SIZE; Current++)
            {
                Song = std::static_pointer_cast<CMusicInfo>(GetSong(Current));
                if(!Song)
                    break;

                while (!Song->InfoLoaded && !Song->Error)
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if(!Song->Downloaded && !Song->Error)
                    break;

                Song = nullptr;
            }

            if(Song)
            {
                try
                {
                    bool Error = false;

                    std::vector<std::string> Args = {m_Settings.YTDLPath, "-q", "-x", "--audio-format", "wav", "-r", "10485760", "-o", Song->Path + ".%(ext)s", Song->URL};
                    TinyProcessLib::Process ytdl(Args, "", nullptr,
                    [&Error](const char *bytes, size_t n) mutable
                    {
                        Error = true;
                    });

                    if(ytdl.get_exit_status() != 0 || Error)
                    {
                        Song->Error = Error;
                        SMessage msg = {Song->Channel, "Failed to download", Song->Name.empty() ? Song->URL : Song->Name};
                        MessageQueue->PrintMessage(msg);

                        WaitFailed();

                        // size_t Pos = IT - m_MusicQueue.begin();

                        // m_MusicQueue.erase(IT);
                        // m_MetaQueue.erase(m_MetaQueue.begin() + Pos);
                    }
                    else
                    {
                        m_DownloadCount++;
                        Song->Downloaded = true;

                        WaitFinished();
                    }
                }
                catch(const std::exception& e)
                {
                    WaitFailed();
                    llog << lerror << e.what() << lendl;
                }
            }
            else if(Current + 1 + m_DownloadCount > Current + 1 + CACHE_SIZE || Current + 1 + m_DownloadCount >= GetQueueSize())
                break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    llog << ldebug << "Downloader stopped" << lendl;
}

std::string CDownloadQueue::GetDownloadParam(const std::string &Url)
{
    if(IsURL(Url))
        return Url;

    return "ytsearch1:" + Url;
}

bool CDownloadQueue::IsURL(const std::string &Url)
{
    std::string Https = "https://";
    std::string Http = "http://";
    return (Url.compare(0, Https.length(), Https) == 0) || (Url.compare(0, Http.length(), Http) == 0);
}