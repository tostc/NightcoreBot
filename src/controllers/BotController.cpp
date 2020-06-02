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

#include <controllers/BotController.hpp>
#include <commands/TextCommands.hpp>
#include <commands/VoiceCommands.hpp>
#include <services/MusicStreamer.hpp>

void CBotController::OnReady()
{
    Prefix = m_Settings.Prefix;

    RegisterCommand<CTextCommands>({"q", "Shows the queue data.", 0, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CTextCommands>({"np", "Shows the current playing song.", 0, "", DiscordBot::AccessMode::EVERYBODY}, Client);

    RegisterCommand<CVoiceCommands>({"join", "Joins the voice channel of the user.", 0, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CVoiceCommands>({"leave", "Leaves the voice channel.", 0, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CVoiceCommands>({"p", "Plays a given song in nightcore.", 1, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CVoiceCommands>({"a", "Plays a given song in anti-nightcore.", 1, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CVoiceCommands>({"d", "Plays a given song in double-nightcore.", 1, "", DiscordBot::AccessMode::EVERYBODY}, Client);
    RegisterCommand<CVoiceCommands>({"n", "Skips the current song.", 0, "", DiscordBot::AccessMode::EVERYBODY}, Client);
}