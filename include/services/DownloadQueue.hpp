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

#ifndef DOWNLOADQUEUE_HPP
#define DOWNLOADQUEUE_HPP

#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <functional>
#include <models/Settings.hpp>
#include <models/Channel.hpp>
#include <controller/IMusicQueue.hpp>

struct SMetaInfo
{
    std::string Name;   //!< Name of the song.
    std::string Duration;   //!< Duration of the song.
}; 

enum class SpeedType
{
    NORMAL,
    DOUBLE,
    ANTI
};

/**
 * Contains information about a song.
 */
class CMusicInfo : public DiscordBot::CSongInfo
{
    public:
        CMusicInfo() : CSongInfo(), Downloaded(false), IsNightcore(false), Error(false), InfoLoaded(false), Speed(SpeedType::NORMAL) {}

        DiscordBot::Channel Channel;
        std::string URL;    //!< Contains the url or search word.
        std::atomic<bool> Downloaded;    //!< True if the file is downloaded.
        std::atomic<bool> IsNightcore;   //!< Is this file already nightcore?
        std::atomic<bool> Error;
        std::atomic<bool> InfoLoaded;
        SpeedType Speed;

        virtual ~CMusicInfo()
        {
            if(Downloaded)
                remove(Path.c_str());
        }
};

class CDownloadQueue : public DiscordBot::IMusicQueue
{
    public:
        CDownloadQueue(const SSettings &Settings);

        void PrintQueue(DiscordBot::Channel channel);

        virtual ~CDownloadQueue();

    protected:
        void OnUpdate(DiscordBot::SongInfo Info, size_t Index) override;
        DiscordBot::AudioSource OnNext(DiscordBot::SongInfo Info) override;
        void OnRemove(DiscordBot::SongInfo Info, size_t Index) override;
        void OnFinishPlaying(DiscordBot::SongInfo Info) override;

    private:
        using MetaQueue = std::vector<SMetaInfo>;
        static const int CACHE_SIZE = 3;
        static const int QUEUE_PRINT_SIZE = 5;

        std::atomic<int> m_DownloadCount;

        void StartThread();

        void AddInfo(std::shared_ptr<CMusicInfo> Info);
        void Downloader();
        std::string GetDownloadParam(const std::string &Url);
        bool IsURL(const std::string &Url);

        SSettings m_Settings;

        std::thread m_DowloadThread;
        std::thread m_InfoThread;
        std::mutex m_MetaLock;

        MetaQueue m_MetaQueue;
        std::atomic<bool> m_Terminate;
};

using DownloadQueue = std::shared_ptr<CDownloadQueue>;

#endif //DOWNLOADQUEUE_HPP