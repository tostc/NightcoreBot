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

#ifndef MESSAGEQUEUE_HPP
#define MESSAGEQUEUE_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <IDiscordClient.hpp>

struct SMessage
{
    public:
        DiscordBot::Channel ChannelRef;
        std::string Title;
        std::string Message;
        std::string URL;
        bool Embed;
};

#define MessageQueue CMessageQueue::Get()

class CBotClient;

/**
 * Message Queue to communicate with the user.
 */
class CMessageQueue
{
    public:
        /**
         * Creates a new instance.
         */
        static void Create(DiscordBot::DiscordClient Client);

        /**
         * @return Gets the message queue instance.
         */
        static std::shared_ptr<CMessageQueue> Get();

        /**
         * Adds a message to the queue.
         */
        void PrintMessage(SMessage Msg);

        /**
         * Adds a message to the queue.
         */
        void PrintCodeblock(SMessage Msg);

        ~CMessageQueue();
    private:
        CMessageQueue(DiscordBot::DiscordClient Client);
        void HandleQueue();

        DiscordBot::DiscordClient m_Client;
        std::thread m_MessageThread;
        std::mutex m_QueueLock;
        bool m_Terminate;
        std::vector<SMessage> m_Queue;
        

        static std::shared_ptr<CMessageQueue> m_Instance;
};

#endif //MESSAGEQUEUE_HPP