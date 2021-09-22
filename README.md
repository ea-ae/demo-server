# Demo Server

Demo Server is a multiplayer game server written in C++. The server uses a custom network protocol built on top of UDP, that implements multiple features such as a snapshot system, network entities, and reliable packets.

## Usage

**Setup (Windows)**

1. Download [Boost 1.71.0](https://www.boost.org/users/history/version_1_71_0.html).
2. Build the project using CMake.
3. Optionally open the project using [Visual Studio 2019](https://visualstudio.microsoft.com).
  1. Run the Google Test unit tests with `Test -> Run -> All Tests`.
5. Run `DemoServer.exe` to launch the project. A custom Unity game client is being used to interact with the server.

## Config

Located in [settings.xml](settings.xml).

| Variable               | Description                                                             |
|------------------------|-------------------------------------------------------------------------|
| GameServer/Version     | Set automatically by CMake.                                             |
| GameServer/Port        | Specifies the port that the server will be listening on.                |
| GameServer/Protocol    | The game's protocol ID that clients use to connect.                     |
| GameServer/Tickrate    | Amount of ticks per second.                                             |
| Game/Debug             | Boolean.                                                                |
| Game/Multithreading    | Boolean. If enabled, client packets will be sent concurrently.          |
| Game/MaxConnections    | Maximum amount of connections per game instance.                        |
| Client/Timeout         | Time (ms) to wait before timing out the client. Set to 0 to disable.    |
| Client/ReliableResend  | Time (ms) to wait before resending a reliable message.                  |
| Client/PacketLogCount  | Amount of latest packets to use when calculating packet loss.           |
| Logger/Severity        | Set to none, fatal, error, warning, info (default), debug, or verbose.  |
| Logger/LogFile         | Name of the log file.                                                   |
| NetSim/InLoss          | Percentage of incoming packets to skip.                                 |
| NetSim/OutLoss         | Percentage of outgoing packets to skip.                                 |

## Roadmap

Check the [project board](https://github.com/ea-ae/demo-server/projects/1) for the features roadmap.

## About

Created as a self-learning & school project in 2020.
