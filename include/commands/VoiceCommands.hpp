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

#ifndef VOICECOMMANDS_HPP
#define VOICECOMMANDS_HPP

#include <controller/ICommand.hpp>
#include <IDiscordClient.hpp>
#include <services/MessageQueue.hpp>

class CVoiceCommands : public DiscordBot::ICommand
{
    public:
        CVoiceCommands(DiscordBot::IDiscordClient *client);
        ~CVoiceCommands() {}
    private:
        void Join(DiscordBot::CommandContext ctx);
        void Leave(DiscordBot::CommandContext ctx);
        void Play(DiscordBot::CommandContext ctx);
        void Next(DiscordBot::CommandContext ctx);

        inline bool CheckServer(DiscordBot::Guild guild, DiscordBot::Channel Channel)
        {
            if(!guild)
            {
                SMessage msg = {Channel, "Error", "Sorry you must connected to a channel to use this command!"};
                MessageQueue->PrintMessage(msg);
                return false;
            }

            return true;
        }

        DiscordBot::IDiscordClient *m_Client;
};

#endif //VOICECOMMANDS_HPP