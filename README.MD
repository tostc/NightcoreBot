# NightcoreBot

This music bot for Discord is for all lovers of nightcore. This bot transforms all music to nightcore, anti nightcore or double speeded nightcore. This bot is for self hosting, this means you need a server or can simply run this bot on your pc.

## Prerequisites

First you need following software and libraries.

- CMake
- C++ compiler which supports C++11 
- [libDiscordBot](https://github.com/tostc/libDiscordBot)
- [youtube-dl](https://youtube-dl.org/)

## Getting Started

These instructions will help you compile and use the bot.

1. Download and compile [libDiscordBot](https://github.com/tostc/libDiscordBot).
2. Cloning the repository
```
git clone --recurse-submodules -j8 --single-branch https://github.com/tostc/NightcoreBot.git
```
3. Copy the compiled library of libDiscordBot inside the `externals/libDiscordBot/lib` folder.
4. Copy the header files of libDiscordBot inside the `externals/libDiscordBot/include` folder.
5. Building the project:

    ```
    cd NightcoreBot/
    mkdir build
    cd build/
    cmake ../ -DCMAKE_INSTALL_PREFIX=/path/to/install/dir
    ```

    or you can use the `cmake-gui`.

    For Linux execute following command inside the build directory
    ```
    make install
    ```

    For Windows open the *.sln file inside the build directory and build `BUILD_INSTALL`.

6. Edit the `settings.json` inside the install directory and insert your Discord Bot token. If you don't know how to get this token please look [here](https://github.com/tostc/libDiscordBot/wiki/Your-first-bot#prerequisites).

    Settings description:

    | Key | Descirption |
    |-----|-------------|
    | token | Your Discord token |
    | prefix | Command prefix which indicates the start of a command |
    | ytdl | Path to the youtube-dl |
    | cache | Cache dir where all files from youtube-dl are stored. (Dir must exists) |
    | debug | Prints debug messages of the bot |
    | log | Log file directory. (Dir must exists) |
7. Copy the youtube-dl inside the executable directory.
8. Start the bot and invite the bot to your server.

### Manually install

Put the libDiscordBot library, the executable, the settings.json and the youtube-dl inside one directory and look at step 6. above.

## How to use
Type `!h` (where `!` is your prefix) inside a text chat, where the bot has access to and you get following list.

```
!a                     -  Plays a given song in anti-nightcore.
!d                     -  Plays a given song in double-nightcore.
!h                     -  Prints a help dialog
!help                  -  Prints a help dialog
!join                  -  Joins the voice channel of the user.
!leave                 -  Leaves the voice channel.
!n                     -  Skips the current song.
!np                    -  Shows the current playing song.
!p                     -  Plays a given song in nightcore.
!q                     -  Shows the queue data.
```

The commands `a`, `d` and `p` excepts one parameter seperated by a ' '. The parameter can either a link to youtube or a search word for youtube.

## Features
- Transforms any youtube video to nightcore in real time
- Supports nightcore (20% Speed for none nightcore music)
- Supports anti-nightcore(-15% Speed for none nightcore and -30% for nightcore)
- Supports double-nightcore(40% Speed for none nightcore and 20% for nightcore)

## Libraries

- [libDiscordBot](https://github.com/tostc/libDiscordBot)

**Following libraries will be downloaded automatically if you checkout the repo as described above.**

- [tiny-process-library](https://gitlab.com/eidheim/tiny-process-library)
- [tinyformat](https://github.com/c42f/tinyformat)
- [CJSON](https://github.com/tostc/CJSON)
- [CLog](https://github.com/tostc/CLog)
- [dr_libs](https://github.com/mackron/dr_libs)
- [sts](https://github.com/kieselsteini/sts)

## License
This library is under the [MIT License](LICENSE.txt)