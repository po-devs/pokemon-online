({

serverStartUp : function() {
    scriptChecks = 0;
    this.init();
}
,

init : function() {
    lastMemUpdate = 0;

    key = function(a,b) {
        return a + "*" + sys.name(b);
    }

    saveKey = function(thing, id, val) {
        sys.saveVal(key(thing,id), val);
    }
    
    getKey = function(thing, id) {
        return sys.getVal(key(thing,id));
    }
    
    hasBan = function(id, poke) {
        return clauses[id].indexOf("*" + poke + "*") != -1;
    }
	
	if (typeof(permChannels) == 'undefined') {
		permChannels = [];
	}
	
	cmp = function(a, b) {
		return a.toLowerCase() == b.toLowerCase();
	}
	
	if (typeof(channelTopics) == 'undefined')
		channelTopics = [];
	
	var dwlist = ["Munna", "Mushaana", "Darumakka", "Hihidaruma", "Eevee", "Umbreon", "Jolteon", "Vaporeon", "Flareon", "Espeon", "Leafeon", "Glaceon",
 "Bellsprout", "Weepinbell", "Victreebel", "Nidoran-M", "Nidorino", "Nidoking", "Sentret", "Furret",
	"Sunkern", "Sunflora", "Hoppip", "Skiploom", "Jumpluff", "Lickitung", "Lickylicky", "Ponyta", "Rapidash",
	"Exeggcute", "Exeggutor", "Farfetch'd", "Nidoran-F", "Nidorina", "Nidoqueen", "Stantler", "Oddish", "Gloom", "Vileplume",
   "Bidoof", "Mareep", "Flaaffy", "Ampharos", "Doduo", "Dodrio", "Tangela", "Tangrowth", "Surskit", "Masquerain",
	"Igglybuff", "Jigglypuff", "Wigglytuff"];
	dwpokemons = [];
	for(var dwpok in dwpokemons) {
		dwpokemons.push(sys.pokeNum(dwlist[i]));
	}

rules = [ "",
    "*** Rules ***",
"",
"Rule #1 - Do Not Abuse CAPS:",
"- The occasional word in CAPS is acceptable, however repeated use is not.",
"Rule #2 - No Flooding the Chat:",
"- Please do not post a large amount of short messages when you can easily post one or two long messages.",
"Rule #3 - Do not Challenge Spam:",
"- If a person refuses your challenge, this means they do not want to battle you. Find someone else to battle with.",
"Rule #4 - Don't ask for battles in the main chat:",
"- There is a 'Find Battle' tab that you can use to find a battle immediately. If after a while you cannot find a match, then you can ask for one in the chat.",
"Rule #5 - No Trolling/Flaming/Insulting of Any kind:",
"- Behaving stupidly and excessive vulgarity will not be tolerated",
"Rule #6 - Be Respectable of Each Others Cultures:",
"- Not everyone speaks the same language. This server is not an English-Only Server. Do not tell someone to only speak a certain language.",
"Rule #7 - No Advertising:",
"- There will be absolutely no advertising on the server.",
"Rule #8 - No Obscene or Pornographic Content Allowed:",
"- This includes links, texts, images, and any other kind of media. This will result in a instant ban.",
"Rule #9 - Do not ask for Auth:",
"- Authority is given upon merit. By asking you have pretty much eliminated your chances at becoming an Auth in the future.",
"Rule #10 - Do not Insult Auth:",
"- Insulting Auth will result in immediate punishment. "   ];

	
	if (typeof(varsCreated) != 'undefined')
        return;

	if (sys.existChannel("Tournaments")) { 
        tourchannel = sys.channelId("Tournaments");
    } else {
        tourchannel = sys.createChannel("Tournaments");
		channelTopics[tourchannel] = "Welcome to the tournament channel, where tournaments are held! type /join to join a tour in its signup phase! type /viewround to see the progress of an ongoing tournament!";
    }
	
    battlesStopped = false;
	channelUsers = [];
	channelTopics = [];
		
    sys.setPA("forceSameTier");
    sys.setPA("megaUser");
    megausers = sys.getVal("megausers");
    
    siggamute = (sys.getVal("SiggaMute") == "1");
    siggaban = (sys.getVal("SiggaBan") == "1");

    muteall = false;
    
    sys.setPA ("impersonation");
    sys.setPA ("muted");
    sys.setPA("caps");
    sys.setPA("timeCount");
    sys.setPA("floodCount");
	maxPlayersOnline = 0;
    
    lineCount = 0;
    tourmode = 0;
    border = "»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»»:";
  

    pokeNatures = [];
    
    var list = "Heatran-Eruption/Quiet=Suicune-ExtremeSpeed/Relaxed|Sheer Cold/Relaxed|Aqua Ring/Relaxed|Air Slash/Relaxed=Raikou-ExtremeSpeed/Rash|Weather Ball/Rash|Zap Cannon/Rash|Aura Sphere/Rash=Entei-ExtremeSpeed/Adamant|Flare Blitz/Adamant|Howl/Adamant|Crush Claw/Adamant";
    
    var sepPokes = list.split('=');
    for (var x in sepPokes) {
        sepMovesPoke = sepPokes[x].split('-');
        sepMoves = sepMovesPoke[1].split('|');
        
        var poke = sys.pokeNum(sepMovesPoke[0]);
        pokeNatures[poke] = [];
        
        for (y in sepMoves) {
            movenat = sepMoves[y].split('/');
            pokeNatures[poke][sys.moveNum(movenat[0])] = sys.natureNum(movenat[1]);
        }
    }
    
    if (sys.existChannel("Indigo Plateau")) { 
        staffchannel = sys.channelId("Indigo Plateau");
    } else {
        staffchannel = sys.createChannel("Indigo Plateau");
    }
	
	channelTopics[staffchannel] = "Welcome to the Staff Channel! Discuss of all what users shouldn't hear here! Or more serious stuff...";
	permChannels[staffchannel] = true;
    
    sendChanMessage = function(id, message) {
        sys.sendMessage(id, message, channel);
    }
    
    sendChanAll = function(message) {
        sys.sendAll(message, channel);
    }
    
    varsCreated = true;
}

,

beforeChannelJoin : function(src, channel) {
    if (channel == staffchannel && !megaUser[src] && sys.auth(src) <= 0) {
        sys.sendMessage(src, "+Guard: Sorry, the access to that place is restricted!");
        sys.stopEvent();
        return;
    }    
}

,

afterChannelCreated : function (chan, name, src) {
	if (src == 0)
		return;
	
	channelUsers[chan] = src;
}

,

afterChannelJoin : function(player, chan) {
	if (typeof(channelTopics[chan]) != 'undefined') {
		sys.sendMessage(player, "Welcome Message: " + channelTopics[chan], chan);
	}
	if (typeof(channelUsers[chan]) != 'undefined' && player == channelUsers[chan]) {
		sys.sendMessage(player, "+ChannelBot: use /topic <topic> to change the welcome message of this channel", chan);
		return;
	}
}

,

beforeChannelDestroyed : function(channel) {
    if (channel == staffchannel ||  channel == tourchannel || (channel in permChannels && permChannels[channel] == true) ) {
        sys.stopEvent();
        return;
    }
	
	delete permChannels[channel];
	delete channelUsers[channel];
	delete channelTopics[channel];
}
,

afterNewMessage : function (message) {
    if (message == "Script Check: OK") {
        sys.sendAll("+ScriptBot: Scripts were updated!");
        if (typeof(scriptChecks)=='undefined')
            scriptChecks = 0;
        scriptChecks += 1;
        this.init();
    }
}

,

afterLogIn : function(src) {
    if (sys.ip(src).substr(0, 7) == "125.237") {
        sys.kick(src);
        return;
    }
   /* Armonio*/
    if ( sys.ip(src).substr(0, 7) == "125.60." || sys.ip(src).substr(0,9) == "151.60.19") {
        sys.kick(src);
        return;
    }
    
    sys.sendMessage(src, "*** Type in /Rules to see the rules. ***");
    sys.sendMessage(src, "+CommandBot: Use !commands to see the commands!");

    if (sys.getVal("muted_*" + sys.ip(src)) == "true") 
        muted[src] = true;
    else
        muted[src] = false;

        
    if (sys.numPlayers() > maxPlayersOnline) {
        maxPlayersOnline = sys.numPlayers();
    }

    if (maxPlayersOnline > sys.getVal("MaxPlayersOnline")) {
        sys.saveVal("MaxPlayersOnline", maxPlayersOnline);
    }

    sys.sendMessage(src, "+CountBot: Max number of players online was " + sys.getVal("MaxPlayersOnline") + ".");
    sys.sendMessage(src, "");
    if (tourmode == 1){
        sys.sendMessage(src,"*** A " + tourtier + " tournament is in its signup phase, " + this.tourSpots() + " space(s) are left!");
        sys.sendMessage(src, "");
        sys.sendMessage(src, border);
        sys.sendMessage(src, "");
    
    } else if (tourmode == 2){
        sys.sendMessage(src, "");
        sys.sendMessage(src, border);
        sys.sendMessage(src, "");
        sys.sendMessage(src, "~~Server~~: A tournament (" + tourtier + ") is currently running.");
        sys.sendMessage(src, "");
        sys.sendMessage(src, border);
        sys.sendMessage(src, "");
    }

    caps[src] = 0;
    timeCount[src] = parseInt(sys.time());
    floodCount[src] = 0;
    impersonation[src] = undefined;
	
	if (sys.auth(src) > 0 && sys.auth(src) <= 3) 
		sys.putInChannel(src, staffchannel);
    
    this.afterChangeTeam(src);
}

,


afterChangeTeam : function(src)
{
    forceSameTier[src] = getKey("forceSameTier", src) == "1";
    
    if (megausers.indexOf("*" + sys.name(src) + "*") != -1)
        megaUser[src] = true;
    else megaUser[src] = false;
    
    for (var i = 0; i < 6; i++) {
        var poke = sys.teamPoke(src, i);
        if (poke in pokeNatures) {
            for (x in pokeNatures[poke]) {
                if (sys.hasTeamPokeMove(src, i, x) && sys.teamPokeNature(src, i) != pokeNatures[poke][x])
                {
                    sys.sendMessage(src, "+CheckBot: " + sys.pokemon(poke) + " with " + sys.move(x) + " must be a " + sys.nature(pokeNatures[poke][x]) + " nature. Change it in the teambuilder.");
                    sys.changePokeNum(src, i, 0);
                }
            }
        }
    }
	var tier = sys.tier(src);
		if (tier != "Dream World" && tier != "Full Dream World") {
			this.dreamWorldAbilitiesCheck(src, false);
		}
}

,
beforeChatMessage: function(src, message, chan) {
    channel = chan;
    if (message.length > 350) {
        sys.stopEvent();
        return;
    }
    
    if (sys.auth(src) < 3 && muted[src] === true && message != "!join" && message != "/rules" && message != "/join" && message != "!rules") {
        sendChanMessage(src, "+Bot: You are muted");
        sys.stopEvent();
        return;
    }

	if (message != message.replace(/[\u0300-\u036F]/gi,'')) {
		sys.stopEvent();
		return; 
	}
	

    if ((message[0] == '/' || message[0] == '!') && message.length > 1) {
        if (parseInt(sys.time()) - lastMemUpdate > 500) {
            sys.clearChat();
            lastMemUpdate = parseInt(sys.time());
        }

        sys.stopEvent();
        var command;
        var commandData;
        var pos = message.indexOf(' ');

        if (pos != -1) {
            command = message.substring(1, pos).toLowerCase();
            commandData = message.substr(pos+1);
        } else {
            command = message.substr(1).toLowerCase();
        }
        var tar = sys.id(commandData);

        if (command == "commands" || command == "command") {
            sendChanMessage(src, "");
            sendChanMessage(src, "*** Commands ***");
            sendChanMessage(src, "/rules: To see the rules");
            sendChanMessage(src, "/me [message]: to speak with *** before its name");
            sendChanMessage(src, "/players: to get the number of players online");
            sendChanMessage(src, "/ranking: to get your ranking in your tier");
            sendChanMessage(src, "/join: allows you to join a tournament.");
            sendChanMessage(src, "/viewround: allows you to view the pairings for the round.");
            sendChanMessage(src, "/megausers: to see the list of people who have power over tournaments.");
            sendChanMessage(src, "/sameTier [on/off]: to force or not the same tier when people challenge you");
			sendChanMessage(src, "/topic <topic>: to change the topic of a channel. Only works if you're the first to log on a channel.");
            if (megaUser[src] != true && sys.auth(src) == 0)
                return;
			sendChanMessage(src, "*** Megauser Commands ***");
            sendChanMessage(src, "/tour tier:number: starts a tier tournament consisting of number of players.");
            sendChanMessage(src, "/endtour: ends the current tournament.");
            sendChanMessage(src, "/dq name: DQs someone in the tournament.");
            sendChanMessage(src, "/changecount [entrants]: Change the number of entrants during the signup phase.");
            sendChanMessage(src, "/push name: Adds someone in the tournament.");
            sendChanMessage(src, "/sub name1:name2: Replaces someone with someone else.");
            sendChanMessage(src, "/cancelBattle name1: Allows the user or his opponent to forfeit his current battle so he can battle again his opponent.");
            if (sys.auth(src) < 1) 
                return;
            sendChanMessage(src, "*** Mod Commands ***");
            sendChanMessage(src, "/sendAll [message] : to send a message to everyone.");
            sendChanMessage(src, "/k [person] : to kick someone");
            sendChanMessage(src, "/[mute/unmute] [person] : You know what i mean :p.");
            sendChanMessage(src, "/silence [x]: To call forth x minute of silence in the main chat (except for auth)");
            sendChanMessage(src, "/silenceoff: To undo that");
            sendChanMessage(src, "/meon, /meoff: to deal with /me happy people");
			sendChanMessage(src, "/perm [on/off]: To make the current channel a permanent channel or not -- i.e. the channel wouldn't be destroyed on log off");
            if (sys.auth(src) < 2)
                return;
            sendChanMessage(src, "*** Admin Commands ***");
            sendChanMessage(src, "/changeRating [player] -- [tier] -- [rating]: to change the rating of a rating abuser");
            sendChanMessage(src, "/stopBattles: to stop all new battles. When you want to close the server, do that");
            sendChanMessage(src, "/siggaban : to ban sigga.");
            sendChanMessage(src, "/siggaunban : to unban sigga.");
            sendChanMessage(src, "/imp [person] : to impersonate someone");
            sendChanMessage(src, "/impOff : to stop impersonating.");
            sendChanMessage(src, "/changeAuth [auth] [person]: to play the mega admin");
            sendChanMessage(src, "/setPA paname: to add a new pa, use with scripting caution");
            sendChanMessage(src, "/megauser[off] xxx: Tourney powers.");
            sendChanMessage(src, "/showteam xxx: To help people who have problems with event moves or invalid teams.");
            sendChanMessage(src, "/memorydump: To see the state of the memory.");
            return;
        }
		
        if (command == "me" && !muteall) {
            if (typeof(meoff) != "undefined" && meoff != false) {
                sendChanMessage(src, "+Bot: /me was turned off.");
                return;
            }
                if (message.length == 3)
                    return;
            sendChanAll("*** " + sys.name(src) + " " + commandData);
            this.afterChatMessage(src, message);
            return;
        }
        if (command == "megausers") {
            sendChanMessage(src, "");
            sendChanMessage(src, "*** MEGA USERS ***");
            sendChanMessage(src, "");
            var spl = megausers.split('*');
            for (x in spl) {
                if (spl[x].length > 0)
                    sendChanMessage(src, spl[x]);
            }
            sendChanMessage(src, "");
            return;
        }
        if (command == "rules") {
            for (rule in rules) {
                sendChanMessage(src, rules[rule]);
            }
            return;
        }
        if (command == "players") {
            sendChanMessage(src, "+CountBot: There are " + sys.numPlayers() + " players online.");
            return;
        }
        if (command == "ranking") {
            var rank = sys.ranking(src);
            if (rank == undefined) {
                sendChanMessage(src, "+RankingBot: You are not ranked in " + sys.tier(src) + " yet!");
            } else {
                sendChanMessage(src, "+RankingBot: Your rank in " + sys.tier(src) + " is " + rank + "/" + sys.totalPlayersByTier(sys.tier(src)) + "!");
            }
            return;
        }
		if (command == "topic") {
			if (commandData == undefined) {
				sendChanMessage(src, "+Bot: Specify a topic!");
				return;
			}
			if (channel == 0) {
				sendChanMessage(src, "+Bot: You can't do that in main channel");
				return;
			}
			if (sys.auth(src) == 0 && (typeof(channelUsers[chan]) == 'undefined' || channelUsers[chan] != src)) {
				sendChanMessage(src, "+Bot: You don't have the rights");
				return;
			}
			channelTopics[chan] = commandData;
			sendChanAll("+ChannelBot: " + sys.name(src) + " changed the topic to: " + commandData);
			return;
		}
        if (command == "sametier") {
            if (commandData == "on") 
                sendChanMessage(src, "+SleepBot: You enforce same tier in your battles.");
            else
                sendChanMessage(src, "+SleepBot: You allow different tiers in your battles.");
            forceSameTier[src] = commandData == "on";
            saveKey("forceSameTier", src, forceSameTier[src] * 1);
            return;
        }
        if (command == "join"){
            if (!sys.isInChannel(src, tourchannel)) {
                sendChanMessage(src, "+TourBot: You must be in the tournaments channel to join a tournament!");
                return;
            }
            if (tourmode != 1){
                sendChanMessage(src, "Sorry, you are unable to join because a tournament is not currently running or has passed the signups phase.");
                return;
            }
            var name = sys.name(src).toLowerCase();
            if (tourmembers.indexOf(name.toLowerCase()) != -1){
                sendChanMessage(src, "Sorry, you are already in the tournament. You are not able to join more than once.");
                return;
            }
            var srctier = sys.tier(src);
            if (!cmp(srctier, tourtier)){
                sendChanMessage(src, "You are currently not battling in the " + tourtier + " tier. Change your tier to " + tourtier + " to be able to join.");
                return;
            }
            if (this.tourSpots() > 0){
                tourmembers.push(name);
                tourplayers[name] = sys.name(src);
                sys.sendAll("~~Server~~: " + sys.name(src) + " joined the tournament! " + this.tourSpots() + " more spot(s) left!", tourchannel);
                if (this.tourSpots() == 0){
                    tourmode = 2;
                    roundnumber = 0;
                    this.roundPairing();
                }
            }    
            return;
        }
        if (command == "viewround"){
			if (tourmode != 2){
                sendChanMessage(src, "Sorry, you are unable to view the round because a tournament is not currently running or is in signing up phase.");
                return;
            }
            
            sendChanMessage(src, "");
            sendChanMessage(src, border);
            sendChanMessage(src, "");
            sendChanMessage(src, "*** ROUND " + roundnumber + " OF " + tourtier.toUpperCase() + " TOURNAMENT ***");
            
            if (battlesLost.length > 0) {
                sendChanMessage(src, "");
                sendChanMessage(src, "*** Battles finished ***");
                sendChanMessage(src, "");
                for (var i = 0; i < battlesLost.length; i+=2) {
                    sendChanMessage(src, battlesLost[i] + " won against " + battlesLost[i+1]);
                }
                sendChanMessage(src, "");
            }
            
            if (tourbattlers.length > 0) {
                if (battlesStarted.indexOf(true) != -1) {
                    sendChanMessage(src, "", channel);
                    sendChanMessage(src, "*** Ongoing battles ***");
                    sendChanMessage(src, "");
                    for (var i = 0; i < tourbattlers.length; i+=2) {
                        if (battlesStarted [i/2] == true)
                            sendChanMessage(src, this.padd(tourplayers[tourbattlers[i]]) + " VS " + tourplayers[tourbattlers[i+1]]);
                    }
                    sendChanMessage(src, "");
                }
                if (battlesStarted.indexOf(false) != -1) {
                    sendChanMessage(src, "");
                    sendChanMessage(src, "*** Yet to start battles ***");
                    sendChanMessage(src, "");
                    for (var i = 0; i < tourbattlers.length; i+=2) {
                        if (battlesStarted [i/2] == false)
                            sendChanMessage(src, tourplayers[tourbattlers[i]] + " VS " + tourplayers[tourbattlers[i+1]]);
                    }
                    sendChanMessage(src, "");
                }
            }
            
            if (tourmembers.length > 0) {
                sendChanMessage(src, "");
                sendChanMessage(src, "*** Members to the next round ***");
                sendChanMessage(src, "");
                var str = "";
                
                for (x in tourmembers) {
                    str += (str.length == 0 ? "" : ", ") + tourplayers[tourmembers[x]];
                }
                sendChanMessage(src, str);
                sendChanMessage(src, "");
            }
                        
            sendChanMessage(src, border);
            sendChanMessage(src, "");
            
            return;
        }
        if (megaUser[src] != true && sys.auth(src) == 0) {
            sendChanMessage(src, "CommandBot: The command " + command + " doesn't exist");
            return;
        }
        if (command == "dq") {
            if (tourmode == 0) {
                sendChanMessage(src, "+TourneyBot: Wait till the tournament has started.");
                return;
            }
            var name2 = commandData.toLowerCase();
            
            if (tourmembers.indexOf(name2) != -1) {
                tourmembers.splice(tourmembers.indexOf(name2),1);
                delete tourplayers[name2];
                sys.sendAll("+TourneyBot: " + commandData + " was removed from the tournament by " + sys.name(src) + "!", tourchannel);
                return;
            }
            if (tourbattlers.indexOf(name2) != -1) {
                battlesStarted[Math.floor(tourbattlers.indexOf(name2)/2)] = true;
                sys.sendAll("+TourneyBot: " + commandData + " was removed from the tournament by " + sys.name(src) + "!", tourchannel);
                this.tourBattleEnd(this.tourOpponent(name2), name2);
            }
            return;
        }
        if (command == "push") {
            if (tourmode == 0) {
                sendChanMessage(src, "+TourneyBot: Wait untill the tournament has started.");
                return;
            }
            if (this.isInTourney(commandData.toLowerCase())) {
                sendChanMessage(src, "+TourneyBot: " +commandData + " is already in the tournament.");
                return;
            }
            sys.sendAll("+TourneyBot: " +commandData + " was added to the tournament by " + sys.name(src) + ".", tourchannel);
            tourmembers.push(commandData.toLowerCase()); 
            tourplayers[commandData.toLowerCase()] = commandData;
            
            if (tourmode == 1 && this.tourSpots() == 0) {
                tourmode = 2;
                roundnumber = 0;
                this.roundPairing();
            }
            return;
        }
        if (command == "cancelbattle") {
            if (tourmode != 2) {
                sendChanMessage(src, "Wait until a tournament starts");
                return;
            }
            var name = commandData.toLowerCase();
            
            if (tourbattlers.indexOf(name) != -1) {
                battlesStarted[Math.floor(tourbattlers.indexOf(name)/2)] = false;
                sendChanMessage(src, "+TourBot: " + commandData + " can forfeit their battle and rematch now.");
            }
            
            return;
        }
        if (command == "sub") {
            if (tourmode != 2) {
                sendChanMessage(src, "Wait until a tournament starts");
                return;
            }
            var players = commandData.split(':');
            
            if (!this.isInTourney(players[0]) && !this.isInTourney(players[1])) {
                sendChanMessage(src, "+TourBot: Neither are in the tourney.");
                return;
            }
            sys.sendAll("+TourBot: " + players[0] + " and " + players[1] + " were exchanged places in the ongoing tournament by " + sys.name(src) + ".", tourchannel);
            
            var p1 = players[0].toLowerCase();
            var p2 = players[1].toLowerCase();
            
            for (x in tourmembers) {
                if (tourmembers[x] == p1) {
                    tourmembers[x] = p2;
                } else if (tourmembers[x] == p2) {
                    tourmembers[x] = p1;
                }
            }
            for (x in tourbattlers) {
                if (tourbattlers[x] == p1) {
                    tourbattlers[x] = p2;
                    battlesStarted[Math.floor(x/2)] = false;
                } else if (tourbattlers[x] == p2) {
                    tourbattlers[x] = p1;
                    battlesStarted[Math.floor(x/2)] = false;
                }
            }
            
            if (!this.isInTourney(p1)) {
                tourplayers[p1] = players[0];
                delete tourplayers[p2];
            } else if (!this.isInTourney(p2)) {
                tourplayers[p2] = players[1];
                delete tourplayers[p1];
            }
            
            return;
        }
        if (command == "tour"){
            if (typeof(tourmode) != "undefined" && tourmode > 0){
                sendChanMessage(src, "Sorry, you are unable to start a tournament because one is still currently running.");
                return;
            }
            
            if (commandData.indexOf(':') == -1)
                commandpart = commandData.split(' ');
            else
                commandpart = commandData.split(':');
                
            tournumber = parseInt(commandpart[1]);
            
            if (isNaN(tournumber) || tournumber <= 2){                        
                sendChanMessage(src, "You must specify a tournament size of 3 or more.");
                return;
            }
            
            var tier = sys.getTierList();
			var found = false;
            for (var x in tier) {
				if (cmp(tier[x], commandpart[0])) {
					tourtier = tier[x];
					found = true;
					break;
				}
            }
			if (!found) {
				sendChanMessage(src, "Sorry, the server does not recognise the " + commandpart[0] + " tier.");
				return;
			}

			tourmode = 1;
			tourmembers = [];
			tourbattlers = [];
			tourplayers = [];
			battlesStarted = [];
			battlesLost = [];
			
			var chans = [0, tourchannel];

			for (var x in chans) {
				var y = chans[x];
				sys.sendAll("", y);
				sys.sendAll(border, y);
				sys.sendAll("*** A Tournament was started by " + sys.name(src) + "! ***", y);
				sys.sendAll("PLAYERS: " + tournumber, y);
				sys.sendAll("TYPE: Single Elimination", y);
				sys.sendAll("TIER: " + tourtier, y);
				sys.sendAll("", y);
				sys.sendAll("*** Go in the Tournaments channel and type /join or !join to enter the tournament! ***", y);
				sys.sendAll(border, y);
				sys.sendAll("", y);
			}
			return;
        }
        
        if (command == "changecount") {
            if (tourmode != 1) {
                sendChanMessage(src, "Sorry, you are unable to join because the tournament has passed the sign-up phase.");
                return;
            }
            var count = parseInt(commandData);
            
            if (isNaN(count) || count < 3) {
                return;
            }
            
            if (count < tourmembers.length) {
                sendChanMessage(src, "There are more than that people registered");
                return;
            }
            
            tournumber = count;
            
			sys.sendAll("", tourchannel);
			sys.sendAll(border, tourchannel);
            sys.sendAll("~~Server~~: " +  sys.name(src) + " changed the numbers of entrants to " + count + "!", tourchannel);
			sys.sendAll("*** " + this.tourSpots() + " more spot(s) left!", tourchannel);
			sys.sendAll(border, tourchannel);
            sys.sendAll("", tourchannel);
			
            if (this.tourSpots() == 0 ){
                tourmode = 2;
                roundnumber = 0;
                this.roundPairing();
            }
                
            return;
        }
        if (command == "endtour"){
            if (tourmode != 0){
                tourmode = 0;
                sys.sendAll("", tourchannel);
				sys.sendAll(border, tourchannel);
				sys.sendAll("~~Server~~: The tournament was cancelled by " + sys.name(src) + "!", tourchannel);
				sys.sendAll(border, tourchannel);
				sys.sendAll("", tourchannel);
            }else
                sendChanMessage(src, "Sorry, you are unable to end a tournament because one is not currently running.");
            return;
        }

        /** Moderator Commands **/
        if (sys.auth(src) < 1) {
            sendChanMessage(src, "+CommandBot: The command " + command + " doesn't exist");
            return;
        }		
		
		if (command == "perm") {
			if (channel == staffchannel || channel == 0) {
				sendChanMessage("+ChannelBot: you can't do that here.");
				return;
			}
			
			permChannels[channel] = (commandData.toLowerCase() == 'on');
			
			sendChanAll("+ChannelBot: " + sys.name(src) + (permChannels[channel] ? " made the channel permanent." : " made the channel a temporary channel again."));
			return;
		}        
        if (command == "meoff") {
            meoff=true;
            sys.sendAll("+Bot: " + sys.name(src) + " turned off /me.");
            return;
        }        
        if (command == "meon") {
            meoff=false;
            sys.sendAll("+Bot: " + sys.name(src) + " turned on /me.");
            return;
        }
        if (command == "silence") {
            if (typeof(commandData) == "undefined") {
                return;
            }
            sys.sendAll("+Bot: " + sys.name(src) + " called for " + commandData + " Minutes of Silence!");
            muteall = true;
            
            var delay = parseInt(commandData * 60);
            
            if (!isNaN(delay) && delay > 0)
                sys.callLater('if (!muteall) return; muteall = false; sys.sendAll("+Bot: Silence is over.");', delay);
            
            return;
        }
        if (command == "silenceoff") {
            if (!muteall) {
                sendChanMessage(src, "+Bot: Nah.");
                return;
            }
            sys.sendAll("+Bot: " + sys.name(src) + " cancelled the Minutes of Silence!");
            muteall = false;
            return;
        }
        if (command == "impoff") {
            delete impersonation[src];
            sendChanMessage(src, "+Bot: Now you are yourself!");
            return;
        }
        if (command == "k") {
            if (tar == undefined) {
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was mysteriously kicked by " + sys.name(src) + "!");
            sys.kick(tar);
            return;
        }
        if (command == "mute") {
            if (tar == undefined) {
                sendChanMessage(src, "+Bot: Couldn't find " + commandData);
                return;
            }
            if (muted[tar]) {
                sendChanMessage(src, "+Bot: He's already muted.");
                return;
            }
            if (sys.auth(tar) >= sys.auth(src)) {
                sendChanMessage(src, "+Bot: you dont have sufficient auth to mute " + commandData + ".");
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was muted by " + sys.name(src) + "!");
            muted[tar] = true;
            sys.saveVal("muted_*" + sys.ip(tar), "true");
            return;
        }
        if (command == "unmute") {
            if (tar == undefined) {
                return;
            }
            if (!muted[tar]) {
                sendChanMessage(src, "+Bot: He's not muted.");
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was unmuted by " + sys.name(src) + "!");
            muted[tar] = false;
            sys.removeVal("muted_*" + sys.ip(tar));
            return;
        }
        if (sys.auth(src) < 3) {
            return;
        }
        /** Admin Commands **/
        if (command == "changerating") {
            var data =  commandData.split(' -- ');
            if (data.length != 3) {
                sendChanMessage(src, "+Bot: You need to give 3 parameters.");
                return;
            }
            var player = data[0];
            var tier = data[1];
            var rating = parseInt(data[2]);
                        
            sys.changeRating(player, tier, rating);
            sendChanMessage(src, "+Bot: Rating of " + player + " in tier " + tier + " was changed to " + rating);
            return;
        }
        if (command == "siggaban") {
            siggaban = true;
            sys.saveVal("SiggaBan", "1");
            return;
        }
        if (command == "siggaunban") {
            siggaban = false;
            sys.saveVal("SiggaUnban", "0");
            return;
        }
        if (command == "showteam") {
            sendChanMessage(src, "");
            for (var i = 0; i < 6; i+=1) {sendChanMessage(src, sys.pokemon(sys.teamPoke(tar, i)) + " @ " + sys.item(sys.teamPokeItem(tar, i))); 
            for (var j = 0; j < 4; j++) {sendChanMessage(src, '- ' + sys.move(sys.teamPokeMove(tar, i, j)));}}
            sendChanMessage(src, "");
        }
        if (command == "sendall") {
            sys.sendChanAll(commandData);
            return;
        }
        if (command == "imp") {
            impersonation[src] = commandData;
            sendChanMessage(src, "+Bot: Now you are " + impersonation[src] + "!");
            return;
        }
        if (command == "memorydump") {
            sendChanMessage(src, sys.memoryDump());
            return;
        }
        if (command == "megauser") {
            if (tar != "undefined") {
                megaUser[tar] = true;
                sys.sendAll("+Bot: " + sys.name(tar) + " was megausered.");
                megausers += "*" + sys.name(tar) + "*";
                sys.saveVal("megausers", megausers);
            }
            return;
        }
        if (command == "megauseroff") {
            if (tar != undefined) {
                megaUser[tar] = false;
                sys.sendAll("+Bot: " + sys.name(tar) + " was removed megauser.");
                megausers = megausers.split("*" + sys.name(tar) + "*").join("");
                sys.saveVal("megausers", megausers);
            } else {
                sys.sendAll("+Bot: " + commandData + " was removed megauser.");
                megausers = megausers.split("*" + commandData + "*").join("");
                sys.saveVal("megausers", megausers);
            }
            return;
        }
        if (command == "setpa") {
            sys.setPA(commandData);
            sendChanMessage(src, "+Bot: -" + commandData + "- was set!");
            return; 
        }
        if (command == "changeauth") {
            var pos = commandData.indexOf(' ');
            if (pos == -1) {
                return;
            }
            var newauth = commandData.substring(0, pos);
            var tar = sys.id(commandData.substr(pos+1));
            sys.changeAuth(tar, newauth);
            sys.sendAll("+Bot: " + sys.name(src) + " changed auth of " + sys.name(tar) + " to " + newauth);
            return;
        }
        if (command == "eval") {
            sys.eval(commandData);
            return;
        }
        if (command == "stopbattles") {
            battlesStopped = !battlesStopped;
            if (battlesStopped)  {
                sys.sendAll("");
                sys.sendAll("*** ********************************************************************** ***");
                sys.sendAll("+BattleBot: The battles are now stopped. The server will restart soon.");
                sys.sendAll("*** ********************************************************************** ***");
                sys.sendAll("");
            } else {
                sys.sendAll("+BattleBot: False alarm, battles may continue.");
            }
        }
        return;
    }
    if (typeof impersonation[src] != 'undefined') {
        sys.stopEvent();
        sendChanAll(impersonation[src] + ": " + message);
        return;
    }
    if (sys.auth(src) == 0 && muteall) {
        sendChanMessage(src, "+Bot: Respect the minutes of silence!");
        sys.stopEvent();
        return;
    }
    var m = message.toLowerCase();
    
    if (m.indexOf("nigger") != -1 || m.indexOf("penis") != -1 ||  m.indexOf("vagina")  != -1 || m.indexOf("fuckface") != -1) {
        sys.stopEvent();
        return;
    }
}
,

afterChatMessage : function(src, message, chan)
{
    channel = chan;
    lineCount+=1;
    
    if (this.isMCaps(message) && sys.auth(src) < 2) {
        caps[src] += 3;
        if (caps[src] >= 9) {
            sendChanAll("+MuteBot: " + sys.name(src) + " was muted for caps.");
            muted[src] = true;
            return;
        }
    } else if (caps[src] > 0) {
        caps[src] -= 1;
    }

    if (typeof(timeCount[src]) == "undefined") {
        timeCount[src] = parseInt(sys.time());
    }
    
    if (sys.auth(src) < 2) {
        floodCount[src] += 1;
        var time = parseInt(sys.time());
        if (time > timeCount[src] + 7) {
            var dec = Math.floor((time - timeCount[src])/7);
            floodCount[src] = floodCount[src] - dec; 
            if (floodCount[src] <= 0) {
                floodCount[src] = 1;
            }
            timeCount[src] += dec*7;
        }
        if (floodCount[src] > 7) {
            sendChanAll("+KickBot: " + sys.name(src) + " was kicked for flood.");
            sys.kick(src);
            return;
        }
    }
}

,

tourSpots : function() {
    return tournumber - tourmembers.length;
}

,

roundPairing : function() {
    roundnumber += 1;

    battlesStarted = [];
    tourbattlers = [];
    battlesLost = [];
    
    if (tourmembers.length == 1) {
		var chans = [0, tourchannel];
		
		for (x in chans) {
			var tchan = chans[x];
			sys.sendAll("", tchan);
			sys.sendAll(border, tchan);
			sys.sendAll("", tchan);
			sys.sendAll("THE WINNER OF THE TOURNAMENT IS : " + tourplayers[tourmembers[0]], tchan);
			sys.sendAll("", tchan);
			sys.sendAll("*** Congratulations, " + tourplayers[tourmembers[0]] + ", on your success! ***", tchan);
			sys.sendAll("", tchan);
			sys.sendAll(border, tchan);
			sys.sendAll("", tchan);
		}
        tourmode = 0;
        return;
    }
    
    var finals = tourmembers.length == 2;
    
	if (!finals) {
		sys.sendAll("", tourchannel);
		sys.sendAll(border, tourchannel);
        sys.sendAll("*** Round " + roundnumber + " of " + tourtier + " tournament ***", tourchannel);
		sys.sendAll("", tourchannel);
	}
    else {
		sys.sendAll("", tourchannel);
		sys.sendAll(border, tourchannel);
        sys.sendAll("*** FINALS OF " + tourtier.toUpperCase() + " TOURNAMENT ***", tourchannel);
		sys.sendAll("", tourchannel);
		sys.sendAll("", 0);
		sys.sendAll(border, 0);
        sys.sendAll("*** FINALS OF " + tourtier.toUpperCase() + " TOURNAMENT ***", 0);
		sys.sendAll("", 0);
	}
	
	var i = 0;
    while (tourmembers.length >= 2) {
		i += 1;
        var x1 = sys.rand(0, tourmembers.length);
        tourbattlers.push(tourmembers[x1]);
        var name1 = tourplayers[tourmembers[x1]];
        tourmembers.splice(x1,1);
        
        
        x1 = sys.rand(0, tourmembers.length);
        tourbattlers.push(tourmembers[x1]);
        var name2 = tourplayers[tourmembers[x1]];
        tourmembers.splice(x1,1);
        
        battlesStarted.push(false);
        
        if (!finals)
            sys.sendAll (i + "." + this.padd(name1) + " VS " + name2, tourchannel);
        else {
            sys.sendAll ("  " + this.padd(name1) + " VS " + name2, tourchannel);
			sys.sendAll ("  " + this.padd(name1) + " VS " + name2, 0);
		}
    }
    
    if (tourmembers.length > 0) {
        sys.sendAll ("", tourchannel);
        sys.sendAll ("*** " + tourplayers[tourmembers[0]] + " is randomly selected to go to next round!", tourchannel);
    }
	
	sys.sendAll(border, tourchannel);
	sys.sendAll("", tourchannel);
	if (finals) {
		sys.sendAll(border, 0);
		sys.sendAll("", 0);
	}
}

,

padd : function(name) {
    var ret = name;
    
    while (ret.length < 20) ret = ' ' + ret;
    
    return ret;
}

,

isInTourney : function (name) {
    var name2 = name.toLowerCase();
    return name2 in tourplayers;
}

,

tourOpponent : function (nam) {
    var name = nam.toLowerCase();
    
    var x = tourbattlers.indexOf(name);
    
    if (x != -1) {
        if (x % 2 == 0) {
            return tourbattlers[x+1];
        } else {
            return tourbattlers[x-1];
        }
    }
    
    return "";
}

,

areOpponentsForTourBattle : function(src, dest) {
    return this.isInTourney(sys.name(src)) && this.isInTourney(sys.name(dest)) && this.tourOpponent(sys.name(src)) == sys.name(dest).toLowerCase();
}
,

areOpponentsForTourBattle2 : function(src, dest) {
    return this.isInTourney(src) && this.isInTourney(dest) && this.tourOpponent(src) == dest.toLowerCase();
}
,

ongoingTourneyBattle : function (name) {
    return tourbattlers.indexOf(name.toLowerCase()) != -1 && battlesStarted[Math.floor(tourbattlers.indexOf(name.toLowerCase())/2)] == true;
}

,

afterBattleStarted: function(src, dest) {
    if (tourmode == 2) {
        if (this.areOpponentsForTourBattle(src, dest)) {
            if (sys.tier(src) == sys.tier(dest) && cmp(sys.tier(src), tourtier))
                battlesStarted[Math.floor(tourbattlers.indexOf(sys.name(src).toLowerCase())/2)] = true;
        }
    }
}

,

afterBattleEnded : function(src, dest, desc) {
    if (tourmode != 2 ||desc == "tie")
        return;
    this.tourBattleEnd(sys.name(src), sys.name(dest));
}

,

tourBattleEnd : function(src, dest)
{
    if (!this.areOpponentsForTourBattle2(src, dest) || !this.ongoingTourneyBattle(src))
        return;
    battlesLost.push(src);
    battlesLost.push(dest);
    
    var srcL = src.toLowerCase();
    var destL = dest.toLowerCase();
    
    battlesStarted.splice(Math.floor(tourbattlers.indexOf(srcL)/2), 1);
    tourbattlers.splice(tourbattlers.indexOf(srcL), 1);
    tourbattlers.splice(tourbattlers.indexOf(destL), 1);
    tourmembers.push(srcL);
    delete tourplayers[destL];
    
	if (tourbattlers.length != 0 || tourmembers.length > 1) {
		sys.sendAll("", tourchannel);
		sys.sendAll(border, tourchannel);
		sys.sendAll("~~Server~~: " + src + " advances to the next round.", tourchannel);
		sys.sendAll("~~Server~~: " + dest + " is out of the tournament.", tourchannel);
	}
    
    if (tourbattlers.length > 0) {
        sys.sendAll("*** " + tourbattlers.length/2 + " battle(s) remaining.", tourchannel);
		sys.sendAll(border, tourchannel);
        sys.sendAll("", tourchannel);
        return;
    }
    
    this.roundPairing();
}

,

isLCaps: function(letter) {
    return letter >= 'A' && letter <= 'Z';
}

,

isMCaps : function(message) {
    var count = 0;
    
    var i = 0;
    while ( i < message.length ) {
        c = message[i];

        if (this.isLCaps(c)) {
            count += 1;
            if (count == 5)
                return true;
        } else {
            count -= 2;
            if (count < 0)
                count = 0;
        }
        i += 1;
    }
    
    return false;
}

,

beforeChallengeIssued : function (src, dest, clauses, rated, mode) {
    if (battlesStopped) {
        sys.sendMessage(src, "+BattleBot: Battles are now stopped as the server will restart soon.");
        sys.stopEvent();
        return;
    }
    
    if (forceSameTier[dest] == true && (sys.tier(dest) != sys.tier(src))) {
        sys.sendMessage(src, "+BattleBot: That guy only wants to fight his own tier.");
        sys.stopEvent();
        return;
    }
    
    if (sys.tier(src) == "Challenge Cup" && sys.tier(dest) == "Challenge Cup" && clauses[6] == 0) {
        sys.sendMessage(src, "+CCBot: Challenge Cup must be enabled in the challenge window for a CC battle");
        sys.stopEvent();
        return;
    }
    
    if (tourmode == 2) {
        var name1 = sys.name(src);
        var name2 = sys.name(dest);
        
        if (this.isInTourney(name1)) {
            if (this.isInTourney(name2)) {
                if (this.tourOpponent(name1) != name2.toLowerCase()) {
                    sys.sendMessage(src, "+TourneyBot: This guy isn't your opponent in the tourney.");
                    sys.stopEvent();
                    return;
                }
            } else {
                sys.sendMessage(src, "+TourneyBot: This guy isn't your opponent in the tourney.");
                sys.stopEvent();
                return;
            }
            if (sys.tier(src) != sys.tier(dest) || !cmp(sys.tier(src),tourtier)) {
                sys.sendMessage(src, "+TourneyBot: You must be both in the tier " + tourtier+ " to battle in the tourney.");
                sys.stopEvent();
                return;
            }
        } else {
            if (this.isInTourney(name2)) {
                sys.sendMessage(src, "+TourneyBot: This guy is in the tournament and you are not, so you can't battle him.");
                sys.stopEvent();
                return;
            }
        }
    }
    
    /* Challenge Cup Clause */
    if (clauses[6] == 1)
        return;

    
    if (sys.tier(src).indexOf("Doubles") != -1 && sys.tier(dest).indexOf("Doubles") != -1 && mode == 0) {
        sys.sendMessage(src, "+Bot: To fight in doubles, enable doubles in the challenge window!");
        sys.stopEvent();
        return;
    }

    this.eventMovesCheck(src);
    this.eventMovesCheck(dest);
	
	if (sys.tier(src) == sys.tier(dest)) {
		var tier = sys.tier(src);
		
		if (tier != "Dream World" && tier != "Full Dream World") {
			this.dreamWorldAbilitiesCheck(src,true);
			this.dreamWorldAbilitiesCheck(dest,true);
		}		
	}
}

,

beforeBattleMatchup : function(src,dest,clauses,rated)
{
    if (battlesStopped) {
        sys.stopEvent();
        return;
    }

    if (tourmode == 2 && (this.isInTourney(sys.name(src)) || this.isInTourney(sys.name(dest)) )) {
        sys.stopEvent();
        return;
    }
	
	this.eventMovesCheck(src);
    this.eventMovesCheck(dest);
	
	if (sys.tier(src) == sys.tier(dest)) {
		var tier = sys.tier(src);
		
		if (tier != "Dream World" && tier != "Full Dream World") {
			this.dreamWorldAbilitiesCheck(src,true);
			this.dreamWorldAbilitiesCheck(dest,true);
		}		
	}
}
,

eventMovesCheck : function(src)
{
    for (var i = 0; i < 6; i++) {
        var poke = sys.teamPoke(src, i);
        if (poke in pokeNatures) {
            for (x in pokeNatures[poke]) {
                if (sys.hasTeamPokeMove(src, i, x) && sys.teamPokeNature(src, i) != pokeNatures[poke][x])
                {
                    sys.sendMessage(src, "+CheckBot: " + sys.pokemon(poke) + " with " + sys.move(x) + " must be a " + sys.nature(pokeNatures[poke][x]) + " nature. Change it in the teambuilder.");
                    sys.stopEvent();
                    sys.changePokeNum(src, i, 0);
                }
            }
        }
    }
}
,


dreamWorldAbilitiesCheck : function(src, se) {
	for (var i = 0; i < 6; i++) {
		var x = sys.teamPoke(src, i);
		
		if (x != 0 && dwpokemons.indexOf(x) == -1 && sys.hasDreamWorldAbility(src, i)) {
			if (se)
				sys.sendMessage(src, "+CheckBot: " + sys.pokemon(x) + " is not allowed with a Dream World ability in this tier. Change it in the teambuilder.");
			if (sys.tier(src) == "Wifi") {
				sys.changeTier(src, "Dream World");
			} else if (sys.tier(src) == "Full Wifi") {
				sys.changeTier(src, "Full Dream World");
			} else 	{
				if (se)
					sys.changePokeNum(src, i, 0);
			}
			if (se)
				sys.stopEvent();
		}
	}
}

})