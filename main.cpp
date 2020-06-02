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

#include <iostream>
#include <models/Settings.hpp>
#include <fstream>
#include <signal.h>

#include <IDiscordClient.hpp>
#include <controllers/BotController.hpp>
#include <services/MessageQueue.hpp>
#include <services/DownloadQueue.hpp>

#define CLOG_IMPLEMENTATION
#include <Log.hpp>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define BOT_WINDOWS
#elif defined(__linux)
    #include <dirent.h>
    #define BOT_UNIX
#endif

using namespace std;

DiscordBot::DiscordClient g_Client;
bool g_Hangs = false;

void EventHandler(int param)
{
    llog << ldebug << "Shutting down. Please wait!" << lendl;
    if(g_Hangs)
    {
        llog << ldebug << "Hangs!!!!" << lendl;
        exit(0);
    }

    g_Hangs = true;

    if(g_Client)
        g_Client->Quit();

    llog << ldebug << "EventHandler" << lendl;
    exit(0);
}

/*!
 * @brief Gets a list of the directory content.
 * 
 * @param Path: Path of the dir.
 */
static std::vector<std::string> GetDirContent(const std::string &Path)
{
        std::vector<std::string> Ret;
#ifdef BOT_UNIX
        DIR *dir = opendir(Path.c_str());
        if(dir)
        {
            dirent *Dir;

            while((Dir = readdir(dir)))
            {
                if((strncmp(Dir->d_name, ".", 1) != 0) && (strncmp(Dir->d_name, "..", 2) != 0))
                    Ret.push_back(Path + "/" + std::string(Dir->d_name));
            }

            closedir(dir);
        }
        else
            throw std::runtime_error(strerror(errno));
#elif defined(BOT_WINDOWS)
        WIN32_FIND_DATA FD;
        HANDLE Find = FindFirstFile(Path.c_str(), &FD);
        if(Find == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Failed to open directory");

        do
        {
            if((strncmp(FD.cFileName, ".", 1) != 0) && (strncmp(FD.cFileName, "..", 2) != 0))
                Ret.push_back(Path + "/" + std::string(FD.cFileName));
        } while (FindNextFile(Find, &FD) != 0);

        FindClose(Find);
#endif
        return Ret;
}

int main() 
{
    signal(SIGINT, EventHandler);
    signal(SIGABRT, EventHandler);
    signal(SIGSEGV, EventHandler);
    signal(SIGTERM, EventHandler);

    ifstream in("settings.json", ios::in);
    if(in.is_open())
    {
        std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        in.close();

        CJSON Json;
        SSettings Settings = Json.Deserialize<SSettings>(str);

        llog.EnableDebugLog(Settings.DebugEnabled);

        std::vector<std::string> Files = GetDirContent(Settings.CacheDir);
        for(auto &&e : Files)
            remove(e.c_str());

        g_Client = DiscordBot::IDiscordClient::Create(Settings.Token);

        CMessageQueue::Create(g_Client);

        g_Client->RegisterController<CBotController>(Settings);
        g_Client->RegisterMusicQueue<CDownloadQueue>(Settings);

        g_Client->Run();
    }
    else
        cerr << "settings.json not found!" << endl;
}