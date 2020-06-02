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

#include <services/MessageQueue.hpp>
#include <chrono>
#include <models/LogLockGuard.hpp>

std::shared_ptr<CMessageQueue> CMessageQueue::m_Instance;

CMessageQueue::CMessageQueue(DiscordBot::DiscordClient Client) : m_Client(Client), m_Terminate(false)
{
    m_MessageThread = std::thread(&CMessageQueue::HandleQueue, this);
}

CMessageQueue::~CMessageQueue() 
{
    m_Terminate = true;
    if(m_MessageThread.joinable())
        m_MessageThread.join();
}

/**
 * Creates a new instance.
 */
void CMessageQueue::Create(DiscordBot::DiscordClient Client)
{
    m_Instance = std::shared_ptr<CMessageQueue>(new CMessageQueue(Client));
}

/**
 * @return Gets the message queue instance.
 */
std::shared_ptr<CMessageQueue> CMessageQueue::Get()
{
    return m_Instance;
}

/**
 * Adds a message to the queue.
 */
void CMessageQueue::PrintMessage(SMessage Msg)
{
    CLogLockGuard lock(m_QueueLock, "m_QueueLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")");
    Msg.Embed = true;
    m_Queue.push_back(Msg);
}

/**
 * Adds a message to the queue.
 */
void CMessageQueue::PrintCodeblock(SMessage Msg)
{
    CLogLockGuard lock(m_QueueLock, "m_QueueLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")");
    Msg.Embed = false;
    m_Queue.push_back(Msg);
}

void CMessageQueue::HandleQueue()
{
    while (!m_Terminate)
    {
        {
            CLogLockGuard lock(m_QueueLock, "m_QueueLock  (File: " + std::string(__FILE__) + " Line: " + std::to_string(__LINE__) + ")", false);
            if(!m_Queue.empty())
            {
                for (auto &&ele : m_Queue)
                {
                    if(ele.Embed)
                    {
                        DiscordBot::Embed e = DiscordBot::Embed(new DiscordBot::CEmbed);
                        e->Title = ele.Title;
                        e->Description = ele.Message;
                        e->URL = ele.URL;
                        e->Type = "link";
                        
                        m_Client->SendMessage(ele.ChannelRef, "", e);
                    }
                    else
                        m_Client->SendMessage(ele.ChannelRef, ele.Message);
                }

                m_Queue.clear();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}