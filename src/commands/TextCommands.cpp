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

#include <commands/TextCommands.hpp>
#include <controllers/BotController.hpp>
#include <services/MessageQueue.hpp>
#include <services/MusicStreamer.hpp>
#include <sstream>
#include <iomanip>
#include <tinyformat.h>

std::string ConvertToTime(size_t Time)
{
    std::string Ret;

    int Sec = Time % 60;
    int Min = Time / 60;
    int Hour = Min / 60;

    Min -= Hour * 60;

    Ret = tfm::format("%02i:%02i", Min, Sec);

    if(Hour != 0)
        Ret = tfm::format("%02i:%s", Hour, Ret);

    return Ret;
}

CTextCommands::CTextCommands(DiscordBot::IDiscordClient *client) : ICommand(), m_Client(client)
{
    RegisterCommandHandler("q", std::bind(&CTextCommands::PrintQueue, this, std::placeholders::_1));
    RegisterCommandHandler("np", std::bind(&CTextCommands::NowPlaying, this, std::placeholders::_1));
}

void CTextCommands::PrintQueue(DiscordBot::CommandContext ctx)
{
    DiscordBot::Guild guild = ctx->Msg->GuildRef;
    if(guild)
    {
        DownloadQueue Queue = std::static_pointer_cast<CDownloadQueue>(m_Client->GetMusicQueue(guild)); 
        if(Queue)
            Queue->PrintQueue(ctx->Msg->ChannelRef);
        else
        {
            std::string Cmd;
            Cmd = "```nimrod\nNo Data in queue. :(\n```";

            SMessage msg = {ctx->Msg->ChannelRef, "", Cmd, "", false};
            MessageQueue->PrintCodeblock(msg);
        }
    }
}

void CTextCommands::NowPlaying(DiscordBot::CommandContext ctx)
{
    auto strm = std::static_pointer_cast<CMusicStreamer>(m_Client->GetAudioSource(ctx->Msg->GuildRef));
    if(strm)
    {
        size_t Progressed = strm->GetProgressedSamples();
        size_t TotalSamples = strm->GetTotalSamples();
        int Bitrate = strm->GetBitrate();

        size_t TotalTime = TotalSamples / Bitrate;
        size_t CurrentTime = Progressed / Bitrate;

        float Percent = CurrentTime / (float)TotalTime;

        int ProgressbarSize = 40;
        int CursorPos = ProgressbarSize * Percent;
        std::string Bar;

        for(int i = 0; i < ProgressbarSize; i++)
        {
            if(i != CursorPos)
                Bar += "-";
            else
                Bar += "|";
        }

        auto Info = strm->GetMusic();

        SMessage msg = {ctx->Msg->ChannelRef, "", tfm::format("```nimrod\nNow playing\n%s\n%s %s %s\n```", Info->Name, ConvertToTime(CurrentTime), Bar, ConvertToTime(TotalTime))};
        MessageQueue->PrintCodeblock(msg);

        // 1:15----------|------------------------------3:15
    }
    else
    {
        SMessage msg = {ctx->Msg->ChannelRef, "", "```nimrod\nNothing playing ;-;\n```"};
        MessageQueue->PrintCodeblock(msg);
    }
}