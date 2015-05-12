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
    ZipCommand = 0,
    Login,
    Reconnect,
    Logout,
    SendChatMessage,
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
    ServNumChange,ServerInfoChanged = ServNumChange,
    ServDescChange,
    ServNameChange,
    SendPM = 20,
    OptionsChange,
    GetUserInfo,
    GetUserAlias,
    GetBanList,
    CPBan,
    CPUnban,
    SpectateBattle,
    SpectatingBattleMessage,
    SpectatingBattleChat,
    DatabaseMod = 30,
    LoadPlugin,
    Cookie, /* Set/get cookie */
    VersionControl_,
    TierSelection,
    ServMaxChange,
    FindBattle,
    ShowRankings,
    Announcement,
    CPTBan,
    CPTUnban = 40,
    PlayerTBan,
    ShowRankings2,
    BattleList,
    ChannelsList,
    ChannelPlayers,
    JoinChannel,
    LeaveChannel,
    ChannelBattle,
    RemoveChannel,
    AddChannel = 50,
    HtmlAuthChange,
    ChanNameChange,
    Unused53,
    Unused54,
    ServerName,
    SpecialPass,
    ServerListEnd,              // Indicates end of transmission for registry.
    SetIP,                      // Indicates that a proxy server sends the real ip of client
    ServerPass                 // Prompts for the server password
};

enum ProtocolError {
    UnknownCommand = 0
};

namespace LoginCommand {
    enum NetworkFlagsCS {
        HasClientType = 0,
        HasVersionNumber,
        HasReconnect,
        HasDefaultChannel,
        HasAdditionalChannels,
        HasColor,
        HasTrainerInfo,
        /* markerbit = 7 */
        HasTeams = 8,
        HasEventSpecification,
        HasPluginList,
        HasCookie,
        HasUniqueId
    };
    enum NetworkFlagsSC {
        HasReconnectPass = 0
    };
}
