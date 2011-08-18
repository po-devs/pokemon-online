/*
  Consolidates all network commands in one file.

  A network command is a unique enumeration value that identifies the type
  of message that follows it in the PO protocol.

  This file is included from the following places:
    - src/DOSTest/analyze.h
    - src/Registry/analyze.h
    - src/Server/analyze.h
    - src/TeamBuilder/analyze.h

  WARNING: Do not place header guards around this file as it is included in
  4 different places and must be included in those 4 places.
 */

enum Command {
    WhatAreYou = 0,
    WhoAreYou,
    Login,
    Logout,
    SendMessage,
    PlayersList,
    SendTeam,
    ChallengeStuff,
    EngageBattle,
    BattleFinished,
    BattleMessage = 10,
    BattleChat,
    KeepAlive, /* obsolete since we use a native Qt option now */
    AskForPass,
    Register,
    PlayerKick,
    PlayerBan,
    ServNumChange,
    ServDescChange,
    ServNameChange,
    SendPM = 20,
    Away,
    GetUserInfo,
    GetUserAlias,
    GetBanList,
    CPBan,
    CPUnban,
    SpectateBattle,
    SpectatingBattleMessage,
    SpectatingBattleChat,
    SpectatingBattleFinished = 30,
    LadderChange,
    ShowTeamChange,
    VersionControl,
    TierSelection,
    ServMaxChange,
    FindBattle,
    ShowRankings,
    Announcement,
    CPTBan,
    CPTUnban = 40,
    PlayerTBan,
    GetTBanList,
    BattleList,
    ChannelsList,
    ChannelPlayers,
    JoinChannel,
    LeaveChannel,
    ChannelBattle,
    RemoveChannel,
    AddChannel = 50,
    ChannelMessage,
    ChanNameChange,
    HtmlMessage,
    HtmlChannel,
    ServerName,
    SpecialPass,
    ServerListEnd,              // Indicates end of transmission for registry.
    SetIP                       // Indicates that a proxy server sends the real ip of client
};

enum ProtocolError {
    UnknownCommand = 0
};

