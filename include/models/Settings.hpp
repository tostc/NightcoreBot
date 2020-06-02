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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <JSON.hpp>
#include <string>

struct SSettings
{
    public:
        std::string Token;      //!< Bot token from the Discord Developer portal.
        std::string CacheDir;   //!< Download directory for music.
        std::string YTDLPath;   //!< Path to the youtube-dl.
        std::string Prefix;     //!< Command prefix.
        std::string LogFile;
        bool DebugEnabled;

        void Deserialize(CJSON &json)
        {
            DebugEnabled = false;
            Token = json.GetValue<std::string>("token");
            CacheDir = json.GetValue<std::string>("cache");
            YTDLPath = json.GetValue<std::string>("ytdl");
            Prefix = json.GetValue<std::string>("prefix");
            LogFile = json.GetValue<std::string>("log");
            DebugEnabled = json.GetValue<bool>("debug");
        }
};

#endif //SETTINGS_HPP