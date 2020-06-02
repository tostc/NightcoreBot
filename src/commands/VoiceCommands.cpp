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

#include <commands/VoiceCommands.hpp>
#include <services/MessageQueue.hpp>
#include <controllers/BotController.hpp>

CVoiceCommands::CVoiceCommands(DiscordBot::IDiscordClient *client) : ICommand(), m_Client(client)
{
    RegisterCommandHandler("join", std::bind(&CVoiceCommands::Join, this, std::placeholders::_1));
    RegisterCommandHandler("leave", std::bind(&CVoiceCommands::Leave, this, std::placeholders::_1));
    RegisterCommandHandler("p", std::bind(&CVoiceCommands::Play, this, std::placeholders::_1));
    RegisterCommandHandler("d", std::bind(&CVoiceCommands::Play, this, std::placeholders::_1));
    RegisterCommandHandler("a", std::bind(&CVoiceCommands::Play, this, std::placeholders::_1));
    RegisterCommandHandler("n", std::bind(&CVoiceCommands::Next, this, std::placeholders::_1));
}

void CVoiceCommands::Join(DiscordBot::CommandContext ctx)
{
    if((ctx->Msg->Member && !ctx->Msg->Member->State) || !ctx->Msg->Member)
    {
        SMessage msg = {ctx->Msg->ChannelRef, "Error", "Sorry you must connected to a channel to use this command!"};
        MessageQueue->PrintMessage(msg);
        return;
    }

    if(!CheckServer(ctx->Msg->GuildRef, ctx->Msg->ChannelRef))
        return;

    m_Client->Join(ctx->Msg->Member->State->ChannelRef);

    return;
}

void CVoiceCommands::Leave(DiscordBot::CommandContext ctx)
{
    m_Client->Leave(ctx->Msg->GuildRef);
}

void CVoiceCommands::Play(DiscordBot::CommandContext ctx)
{
    if((ctx->Msg->Member && !ctx->Msg->Member->State) || !ctx->Msg->Member)
    {
        SMessage msg = {ctx->Msg->ChannelRef, "Error", "Sorry you must connected to a channel to use this command!"};
        MessageQueue->PrintMessage(msg);
        return;
    }

    if(!CheckServer(ctx->Msg->GuildRef, ctx->Msg->ChannelRef))
        return;

    m_Client->Join(ctx->Msg->Member->State->ChannelRef);

    std::shared_ptr<CMusicInfo> Info = std::shared_ptr<CMusicInfo>(new CMusicInfo());
    SpeedType Type = SpeedType::NORMAL;
    if(ctx->Command == "d")
        Type = SpeedType::DOUBLE;
    else if(ctx->Command == "a")
        Type = SpeedType::ANTI;

    Info->Channel = ctx->Msg->ChannelRef; 
    Info->URL = ctx->Params.back();
    Info->Speed = Type;
    m_Client->AddToQueue(ctx->Msg->GuildRef, std::static_pointer_cast<DiscordBot::CSongInfo>(Info));

    if(!m_Client->IsPlaying(ctx->Msg->GuildRef))
        m_Client->StartSpeaking(ctx->Msg->Member->State->ChannelRef);
}

void CVoiceCommands::Next(DiscordBot::CommandContext ctx)
{
    m_Client->StopSpeaking(ctx->Msg->GuildRef);
}