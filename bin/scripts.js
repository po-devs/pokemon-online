// Don't touch anything here if you don't know what you do.
var noPlayer = '*';
var mafia = new function() {
    this.roles1 = ["bodyguard", "mafia", "inspector", "werewolf", "hooker", "villager", "mafia",
                    "villager", "villager", "villager", "mayor"];
    this.roles2 = ["bodyguard", "mafia1", "mafia1", "inspector", "hooker", "villager", "mafia2",
                    "mafia2", "villager", "villager", "villager", "mayor", "villager", "spy", "villager",
                    "villager", "villager", "villager", "villager", "vigilante", "villager", "godfather",
                    "villager", "villager", "villager", "villager", "werewolf", "mafia1",
                    "mafia2", "mafia1"];
    this.translations = {mafia: "Mafia", inspector:"Inspector", werewolf:"WereWolf", hooker:"Pretty Lady",
                    villager: "Villager", mayor: "Mayor", mafia1: "French Canadian Mafia", bodyguard: "Bodyguard",
                    mafia2: "Italian Mafia", vigilante: "Vigilante", spy: "Spy", godfather: "Godfather"};
    this.roleDescs = {
        villager: "You dont have any special commands during the night! Vote to remove people in the day!",
        inspector: "Type /Inspect [name] to find his/her identity!",
        bodyguard: "Type /Protect [name] to protect someone!",
        mafia: "Type /Kill [name] to  kill someone! (try not to kill your partner)",
        werewolf: "Type /Kill [name] to  kill someone!",
        hooker: "Type /Distract [name] to distract someone! Vote to remove people in the day!",
        mayor: "You dont have any special commands during the night! Vote to remove people in the day! (your vote counts as 2)",
        spy: "You can find out who is going to get killed next!(no command for this ability) Vote to remove people in the day!",
        godfather: "Type /Kill [name] to kill someone! You can kill 2 targets, Type /kill [name2] again to select your second target!",
        vigilante: "Type /Kill [name] to kill someone!(dont kill the good people!)",
        mafia1: "Type /Kill [name] to  kill someone!(Try not to kill your partner)",
        mafia2: "Type /Kill [name] to  kill someone!(Try not to kill your partner)"
    };
	this.tr = function(string) {
        return this.translations[string];
    };
    this.isInGame = function(player) {
        return this.players.indexOf(player) != -1;
    };
	this.correctCase = function(string) {
		for (x in this.players) {
			if (this.players[x].toLowerCase() == string.toLowerCase()) 
				return this.players[x];
		}
		return noPlayer;
	};
    this.roleSides = {
        villager: "Good people",
        inspector: "Good people",
        bodyguard: "Good people",
        mafia: this.tr('mafia'),
        werewolf: this.tr('werewolf'),
        hooker: "Good people",
        mayor: "Good people",
        spy: "Good people",
        godfather: this.tr('godfather'),
        vigilante: "Good people",
        mafia1: this.tr('mafia1'),
        mafia2: this.tr('mafia2')
    };
    this.targetCommand = {
        inspector: "inspect",
        bodyguard: "protect",
        mafia: "kill",
        werewolf: "kill",
        hooker: "distract",
        godfather: "kill",
        vigilante: "kill",
        mafia1: "kill",
        mafia2: "kill"
    };
	
	
    this.clearVariables = function() {
        this.players = [];
        this.roles = {};
        this.state = "blank";
        this.ticks = 0;
        this.votes = {};
		this.ips = [];
        this.resetTargets();
    };
    this.resetTargets = function() {
        this.targets = {
            mafia: noPlayer,
            werewolf: noPlayer,
            godfather: [],
            vigilante: noPlayer,
            mafia1: noPlayer,
            mafia2: noPlayer,
            bodyguard: noPlayer, 
            inspector: noPlayer,
            hooker: noPlayer
        };
        this.nightKill = false;
    };
    this.clearVariables();
    this.startGame = function(src) {
        if (mafia.state != "blank") {
            sys.sendMessage(src, "±Game: A game is going on. Wait until it's finished to start another one", mafiachan);
			sys.sendMessage(src, "±Game: You can join the game by typing /join !", mafiachan);

            return;
        }
		
		sys.sendAll("", mafiachan);
		sys.sendAll("*** ************************************************************************************", mafiachan);
		sys.sendAll("±Game: " + sys.name(src) + " started a game!", mafiachan);
	    sys.sendAll("±Game: Type /Join to enter the game!", mafiachan);
	    sys.sendAll("*** ************************************************************************************", mafiachan);
        sys.sendAll("", mafiachan);
		
		if (sys.playersOfChannel(mafiachan).length < 20) {
			sys.sendAll("", 0);
			sys.sendAll("*** ************************************************************************************", 0);
			sys.sendAll("±Game: " + sys.name(src) + " started a mafia game!", 0);
			sys.sendAll("±Game: Go in the Mafia Channel and type /Join to enter the game!", 0);
			sys.sendAll("*** ************************************************************************************", 0);
			sys.sendAll("", 0);
		}
		
        mafia.clearVariables();
        mafia.state = "entry";

        mafia.ticks = 60;
    };
	this.endGame = function(src) {
        if (mafia.state == "blank") {
            sys.sendMessage(src, "±Game: No game is going on.",mafiachan);
            return;
        }
		sys.sendAll("*** ************************************************************************************", mafiachan);
        sys.sendAll("±Game: " + sys.name(src) + " has stopped the game!", mafiachan);
        sys.sendAll("*** ************************************************************************************", mafiachan);
        sys.sendAll("", mafiachan);
       
        mafia.clearVariables();
    };
    this.tickDown = function() {
        if (this.ticks <= 0) {
            return;
        }
        this.ticks = this.ticks - 1;
        if (this.ticks == 0)
            this.callHandler(this.state);
        else {
            if (this.ticks == 30 && this.state == "entry") {
                sys.sendAll("", mafiachan);
                sys.sendAll("±Game: Hurry up, you only have "+this.ticks+" seconds more to join!", mafiachan);
                sys.sendAll("", mafiachan);
            }
        }
    };
    this.sendPlayer = function(player, message) {
        var id = sys.id(player);
        if (id == undefined)
            return;
        sys.sendMessage(id, message, mafiachan);
    };
    this.getPlayersForRole = function(role) {
        var team = []
        for (var x in this.players) {
            if (this.roles[this.players[x]] == role) {
                team.push(this.players[x]);
            }
        }
        return team;
    };
    this.getPlayersForRoleS = function(role) {
        return mafia.getPlayersForRole(role).join(", ");
    };
    this.getCurrentRoles = function() {
        var list = []
        for (var x in this.roles) {
            if (list.indexOf(this.roles[x]) == -1)
                list.push(this.tr(this.roles[x]));
        }  
		 /* Sorting to not give out the order of the roles per player */
        return list.sort().join(", ");
    };
    this.player = function(role) {
        for (var x in this.roles) {
            if (this.roles[x] == role) //Checks sequentially all roles to see if this is the good one
                return x;
        }
        return noPlayer;
    };
    this.role = function(player) {
        return this.roles[player];
    };
    this.target = function(role) {
        return this.targets[role];
    };
    this.removePlayer = function(player) {
        var role = this.role(player);
        if (this.getPlayersForRole(role).length == 1) {
            this.removeTarget(player);
        }
        delete this.roles[player];
        this.players.splice(this.players.indexOf(player), 1);
    };
    this.kill = function(player) {
        sys.sendAll("±Kill: " + player + " (" + this.tr(this.role(player)) + ") died!", mafiachan);
        this.nightKill = true;
        this.removePlayer(player);
    };
    this.removeTarget = function(player) {
        this.targets[this.role(player)] = noPlayer;
		if (this.role(player) == "godfather") {
			this.targets[this.role(player)] = [];
		}
    };
	this.removeTarget2 = function(player, target) {
		if (this.role(player) != "godfather") {
			this.targets[this.role(player)] = noPlayer;
		} else if (this.role(player) == "godfather" && this.target("godfather").indexOf(target) != -1) {
			this.targets[this.role(player)].splice(this.target("godfather").indexOf(target), 1);
		}
    };
    this.setTarget = function(role, player) {
		if (role != "godfather")
			this.targets[role] = player;
		else {
			if (this.targets[role].indexOf(player) == -1) {
				this.targets[role].push(player);
				if (this.targets[role].length > 2) {
					this.targets[role].splice(0, 1);
				}
			}
			if (this.ticks > 0)
				this.sendPlayer(this.player(role), "±Game: Your target(s) are " + this.targets[role].join(', ') + "!");
		}
    };
    this.testWin = function() {
        var winRole = mafia.roleSides[mafia.role(mafia.players[0])];
        var winning = true;
        var players = [];
       
        for (var x in mafia.roles) {
            if (mafia.roleSides[mafia.roles[x]] != winRole)
                winning = false;
            else
                players.push(x);
        }
        if (winning) {
            sys.sendAll("±Game: The " + winRole + " (" + players.join(', ') + ") wins!", mafiachan);
            sys.sendAll("*** ************************************************************************************", mafiachan);
            mafia.clearVariables();
            return true;
        }
       
        return false;
    };
    this.handlers = {
        entry: function () {
            sys.sendAll("*** ************************************************************************************", mafiachan);
            sys.sendAll("Times Up! :", mafiachan);
           
            if (mafia.players.length < 5) {
                sys.sendAll("Well, Not Enough Players! :", mafiachan);
                sys.sendAll("You need at least 5 players to join (Current; " + mafia.players.length + ").", mafiachan);
                sys.sendAll("*** ************************************************************************************", mafiachan);
                mafia.clearVariables();
                return;
            }
           
            /* Creating the roles list */
            if (mafia.players.length <= mafia.roles1.length) {
                var srcArray = mafia.roles1.slice(0, mafia.players.length);
            } else {
                var srcArray = mafia.roles2.slice(0, mafia.players.length);
            }
           
            srcArray = shuffle(srcArray);
           
            for (var x in srcArray) {
                mafia.roles[mafia.players[x]] = srcArray[x];
            }
           
            sys.sendAll("The Roles have been Decided! :", mafiachan);
           
            for (var x in mafia.players) {
                var player = mafia.players[x];
                var role = mafia.role(player);

                mafia.sendPlayer(player, "±Game: You are a " + mafia.tr(role) + "!");
                mafia.sendPlayer(player, "±Game: " + mafia.roleDescs[role]);

                if (role == "mafia" || role == "mafia1" || role == "mafia2") {
                    mafia.sendPlayer(player, "±Game: Your team is " + mafia.getPlayersForRoleS(role) + ".");
                }
            }	
			sys.sendAll("Current Roles: " + mafia.getCurrentRoles() + ".", mafiachan);
            sys.sendAll("Current Players: " + mafia.players.sort().join(", ") + ".", mafiachan);
            sys.sendAll("Time: Night", mafiachan);
            sys.sendAll("Make your moves, you only have 30 seconds! :", mafiachan);
            sys.sendAll("*** ************************************************************************************", mafiachan);
           
            mafia.ticks = 30;
            mafia.state = "night";
            mafia.resetTargets();
        }
    ,
        night : function() {
            sys.sendAll("*** ************************************************************************************", mafiachan);
            sys.sendAll("Times Up! :", mafiachan);

          
			/* hooker */
            if (mafia.target('hooker') != noPlayer) {
                var player = mafia.target('hooker');
                var hooker = mafia.player('hooker');
                var role = mafia.roles[player];


                if (role == "werewolf") {
					mafia.sendPlayer(hooker, "±Game: You tried to distract the Werewolf (what an idea, srsly), you were ravishly devoured, yum !");
					mafia.sendPlayer(player, "±Game: The Pretty Lady came to you last night! You devoured her instead !");
					mafia.kill(hooker);
					mafia.removeTarget(player);
                } else if (role == "godfather") {
                    mafia.sendPlayer(hooker, "±Game: You tried to seduce the Godfather, you just were killed!");
                    mafia.sendPlayer(player, "±Game: The Pretty Lady came to you last night! You killed her instead!");
					mafia.kill(hooker);
					mafia.setTarget(role, hooker);
				}  else {
					mafia.sendPlayer(player, "±Game: The Pretty Lady came to you last night! You were too busy being distracted!");
					mafia.removeTarget(player);
					if (role == 'mafia' || role == 'mafia1' || role == 'mafia2') {
						var team = mafia.getPlayersForRole(role);
						   
							for (var x in team) {
								if (team[x] != player) {
									mafia.sendPlayer(team[x], "±Game: Your partner was too busy with the Pretty Lady during the night, you decided not to kill anyone during the night!");
								}
							}
					 }
				}
            }
            var killingRoles = ['werewolf', 'mafia', 'mafia2', 'mafia1', 'vigilante', 'godfather'];
            /* bodyguard */
            if (mafia.target('bodyguard') != noPlayer) {
                var pro = mafia.target('bodyguard');
                for (var x in killingRoles) {
				   var role = killingRoles[x];
					var list;
					if (role == "godfather") {
						list = mafia.target(role);
					} else {
						list = [mafia.target(role)];
					}
					for (var y in list) {
						if (list[y] == pro) {
							var team = mafia.getPlayersForRole(killingRoles[x]);
							for (var y in team) {
								mafia.sendPlayer(team[y], "±Game: Your target (" + pro + ") was protected!");
								mafia.removeTarget2(team[y], pro);
							}
						}
					}
                }
            }

            /* Various kills */
            for (var x in killingRoles) {
                var role = killingRoles[x];
				var list;
				if (role == "godfather") {
					list = mafia.target(role);
				} else {
					list = [mafia.target(role)];
				}
				for (var y in list) {
					if (list[y] != noPlayer && mafia.isInGame(list[y]) ) {
						print(list[y] + "(" + mafia.tr(mafia.role(list[y])) + ")");
						mafia.sendPlayer(list[y], "±Game: You were killed during the night!");
						mafia.kill(list[y]);
					}
				}
            }
			
            /* inspector */
            if (mafia.target('inspector') != noPlayer) {
                mafia.sendPlayer(mafia.player('inspector'), "±Info: " + mafia.target('inspector') + " is the " + mafia.tr(mafia.role(mafia.target('inspector'))) + "!!");
			}
            
            if (!mafia.nightKill) {
                sys.sendAll("No one Died! :", mafiachan);
            }
           
            if (mafia.testWin()) {
                return;
            }
           
            mafia.ticks = 30;
			if (mafia.players.length >= 15) {
				mafia.ticks = 40;
			} else if (mafia.players.length <= 4) {
				mafia.ticks = 15;
			}

            sys.sendAll("*** ************************************************************************************", mafiachan);

		    sys.sendAll("Current Roles: " + mafia.getCurrentRoles() + ".", mafiachan);
            sys.sendAll("Current Players: " + mafia.players.sort().join(", ") + ".", mafiachan);
            sys.sendAll("Time: Day", mafiachan);
            sys.sendAll("You have " + mafia.ticks + " seconds to debate who are the bad guys! :", mafiachan);
            sys.sendAll("*** ************************************************************************************", mafiachan);
           
            mafia.state = "standby";
        }
    ,
		standby : function() {
            mafia.ticks = 30;

			sys.sendAll("*** ************************************************************************************", mafiachan);

		    sys.sendAll("Current Roles: " + mafia.getCurrentRoles() + ".", mafiachan);
            sys.sendAll("Current Players: " + mafia.players.sort().join(", ") + ".", mafiachan);
            sys.sendAll("Time: Day", mafiachan);
            sys.sendAll("It's time to vote someone off, type /Vote [name],  you only have " + mafia.ticks + " seconds! :", mafiachan);
            sys.sendAll("*** ************************************************************************************", mafiachan);
           
            mafia.state = "day";
            mafia.votes = {};
		}
	,
        day : function() {
            sys.sendAll("*** ************************************************************************************", mafiachan);
            sys.sendAll("Times Up! :", mafiachan);
           
            var voted = {};
            for (var x in mafia.votes) {
                if (! (mafia.votes[x] in voted)) {
                    voted[mafia.votes[x]] = 0;
                }
                if (mafia.role(x) == 'mayor')
                    voted[mafia.votes[x]] = voted[mafia.votes[x]] + 2;
                else
                    voted[mafia.votes[x]] = voted[mafia.votes[x]] + 1;
            }
            var tie = true;
            var maxi = 0;
            var downed = noPlayer;
            for (var x in voted) {
                if (voted[x] == maxi) {
                    tie = true;
                } else if (voted[x] > maxi) {
                    tie = false;
                    maxi = voted[x];
                    downed = x;
                }
            }
           
            if (tie) {
                sys.sendAll("No one was voted off! :", mafiachan);
                sys.sendAll("*** ************************************************************************************", mafiachan);
            } else {
                sys.sendAll("±Game: " + downed + " (" + mafia.tr(mafia.role(downed)) + ") was removed from the game!", mafiachan);
                mafia.removePlayer(downed);
               
                if (mafia.testWin())
                    return;
            }
           
            sys.sendAll("Current Roles: " + mafia.getCurrentRoles() + ".", mafiachan);
            sys.sendAll("Current Players: " + mafia.players.sort().join(", ") + ".", mafiachan);
            sys.sendAll("Time: Night", mafiachan);
            sys.sendAll("Make your moves, you only have 30 seconds! :", mafiachan);
            sys.sendAll("*** ************************************************************************************", mafiachan);
           
            mafia.ticks = 30;
            mafia.state = "night";
            mafia.resetTargets();
        }
    };
    this.callHandler = function(state) {
        if (state in this.handlers)
            this.handlers[state]();
    };
    this.showCommands = function(src) {
        sys.sendMessage(src, "", mafiachan);
        sys.sendMessage(src, "Server Commands:", mafiachan);
        for (x in mafia.commands["user"]) {
            sys.sendMessage(src, "/" + cap(x) + " - " + mafia.commands["user"][x][1], mafiachan);
        }
        if (sys.auth(src) > 0) {
            sys.sendMessage(src, "Authority Commands:", mafiachan);
            for (x in mafia.commands["auth"]) {
                sys.sendMessage(src, "/" + cap(x) + " - " + mafia.commands["auth"][x][1], mafiachan);
            }
        }
        sys.sendMessage(src, "", mafiachan);
    };
    this.showHelp = function(src) {
        var help = [
            "*** *********************************************************************** ***",
            "±Game: The objective in this game on how to win depends on the role you are given.",
            "*** *********************************************************************** ***",
            "±Role: Mafia",
            "±Win: Eliminate the WereWolf and the Good People!",
            "*** *********************************************************************** ***",
            "±Role: WereWolf",
            "±Win: Eliminate everyone else in the game!",
            "*** *********************************************************************** ***",
            "±Role: Good people (Inspector, Bodyguard, Hooker, Villager, Mayor, Spy, Vigilante)",
            "±Win: Eliminate the WereWolf, Mafia (French and Italian if exists) and the Godfather!",
            "*** *********************************************************************** ***",
            "±Role: French Canadian Mafia",
            "±Win: Eliminate the Italian Mafia, Godfather and the Good People!",
            "*** *********************************************************************** ***",
            "±Role: Italian Mafia",
            "±Win: Eliminate the French Canadian Mafia, Godfather and the Good People!",
            "*** *********************************************************************** ***",
            "±More: Type /roles for more info on the characters in the game!",
            "±More: Type /rules to see some rules you should follow during a game!",
            "*** *********************************************************************** ***",
            ""
        ];
        dump(src, help);
    };
    this.showRoles = function(src) {
        var roles = [
            "*** *********************************************************************** ***",
            "±Role: Villager",
            "±Ability: The Villager has no command during night time. They can only vote during the day!",
            "±Game: 6-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Inspector",
            "±Ability: The Inspector can find out the identity of a player during the Night. ",
            "±Game: 5-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Bodyguard",
            "±Ability: The Bodyguard can protect one person during the night from getting killed, but the bodyguard cant protect itself.",
            "±Game: 5-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Pretty Lady",
            "±Ability: The Pretty Lady can distract people during the night thus cancelling their move, unless it's the WereWolf.",
            "±Game: 5-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Mafia",
            "±Ability: The Mafia is a group of 2 people. They get one kill each night. They strike after the WereWolf.",
            "±Game: 5-12 Players",
            "*** *********************************************************************** ***",
            "±Role: WereWolf",
            "±Ability: The WereWolf is solo. To win it has to kill everyone else in the game. The Werewolf strikes first.",
            "±Game: 5-12 27-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Italian Mafia",
            "±Ability: The Italian Mafia is a group of 2-3 people. They get one kill each night. They strike before the French Canadian Mafia.",
            "±Game: 12-30 Players",
            "*** *********************************************************************** ***",
            "±Role: French Canadian Mafia",
            "±Ability: The French Canadian Mafia is a group of 2-4 people. They get one kill each night. They strike after the Italian Mafia.",
            "±Game: 12-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Mayor",
            "±Ability: The Mayor has no command during the night but his/her vote counts as 2.",
            "±Game: 10-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Spy",
            "±Ability: The Spy has 33% chance of finding out who is going to get killed by The Italian or French Canadian Mafia during the night. And 10% chance to find out who is the killer!",
            "±Game: 13-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Vigilante",
            "±Ability: The Vigilante can kill a person during the night! He/she strikes after The French Canadian and Italian Mafia.",
            "±Game: 20-30 Players",
            "*** *********************************************************************** ***",
            "±Role: Godfather",
            "±Ability: The Godfather can kill 2 people during the night! He/she strikes Last!",
            "±Game: 20-30 Players",
            "*** *********************************************************************** ***",
            ""
        ];
        dump(src, roles);
    };
    this.showRules = function(src) {
        var rules = [
            "",
            "     Server Rules: ",
            "±Rule: No Spamming / flooding ",
            "±Rule: No insulting- especially not auth. ",
            "±Rule: No asking for auth.",
			"±Rule: No trolling.",
			"",
            "     Game Rules: ",
            "±Rule: Do not quote any of the Bots.",
            "±Rule: Do not quit the game before you are dead.",
            "±Rule: Do not talk once your dead or voted off. ",
			"±Rule: Do not use a hard to type name.",
			"±Rule: Do not group together to ruin the game",
			"±Rule: DO NOT REVEAL YOUR PARTNER IF YOU ARE MAFIA, OR KILL THEM",
            "",
            "±Game: Disobey them and you will be banned/muted/kicked according to the mod/admin's wish!",
            ""
        ];
        dump(src, rules);
    };
    this.commands = {
        user: {
	    commands : [this.showCommands, "To see the various commands."],
            start: [this.startGame, "Starts a Game of Mafia."],
            help: [this.showHelp, "For info on how to win in a game."],
            roles: [this.showRoles, "For info on all the Roles in the game."],
            rules: [this.showRules, "To see the Rules for the Game/Server."]            
        },
        auth: {
            end: [this.endGame, "To cancel a Mafia game!"],
        }
    };
    this.handleCommand = function(src, message) {
        var command;
        var commandData = '*';
        var pos = message.indexOf(' ');

        if (pos != -1) {
            command = message.substring(0, pos).toLowerCase();
            commandData = message.substr(pos+1);
        } else {
            command = message.substr(0).toLowerCase();
        }
        if (command in this.commands["user"]) {
            this.commands["user"][command][0](src);
            return;
        }		
	    if (this.state == "entry") {
            if (command == "join") {
                if (this.isInGame(sys.name(src))) {
                    sys.sendMessage(src, "±Game: You already joined!", mafiachan);
                    return;
                }
				if (this.ips.indexOf(sys.ip(src))!=-1)
				{
					sys.sendMessage(src, "±Game: This IP is already in list. You cannot register two times !", mafiachan);
					return;
				}
                if (this.players.length >= 30) {
                    sys.sendMessage(src, "±Game: There can't be more than 30 players!", mafiachan);
                    return;
                }
                var name = sys.name(src);
                for (x in name) {
                    var code = name.charCodeAt(x);
                    if (name[x] != ' ' && name[x] != '.' && (code < 'a'.charCodeAt(0) || code > 'z'.charCodeAt(0))
                        && (code < 'A'.charCodeAt(0) || code > 'Z'.charCodeAt(0)) && name[x] != '_' && name[x] !='<' && name[x] != '>' && (code < '0'.charCodeAt(0) || code > '9'.charCodeAt(0)))
                    {
                        sys.sendMessage(src, "±Name: You're not allowed to have the following character in your name: " + name[x] + ".", mafiachan);
                        sys.sendMessage(src, "±Rule: You must change it if you want to join!", mafiachan);
                        return;
                    }
                }
                if (name.length > 12) {
                    sys.sendMessage(src, "±Name: You're not allowed to have more than 12 letters in your name!", mafiachan);
                    sys.sendMessage(src, "±Rule: You must change it if you want to join!", mafiachan);
                    return;
                }
                this.players.push(name);
				this.ips.push(sys.ip(src));
                sys.sendAll("±Game: " + name + " joined the game !", mafiachan);
				
				if (this.players.length == 30) {
					this.ticks = 1;
				}
                return;
            }
        } else if (this.state == "night") {
			var name = sys.name(src);
            if (this.isInGame(name) && this.role(name) in this.targetCommand &&
                command == this.targetCommand[this.role(name)]) {
				commandData = this.correctCase(commandData);
                if (!this.isInGame(commandData)) {
                    sys.sendMessage(src, "±Hint: That person is not playing!", mafiachan);
                    return;
                }
                if (commandData == name) {
                    sys.sendMessage(src, "±Hint: Nope, this wont work... You can't target yourself!", mafiachan);
                    return;
                }
				
                var role = this.role(name);
               
                sys.sendMessage(src, "±Game: You have chosen to " + command + " " + commandData + "!", mafiachan);
                this.setTarget(role, commandData);
               
                if (role == "mafia" || role == "mafia1" || role == "mafia2") {
                    var team = this.getPlayersForRole(role);
                    for (x in team) {
                        if (team[x] != name) {
                            this.sendPlayer(team[x], "±Game: Your partner(s) have decided to kill '" + commandData + "'!");
                        }
                    }
                }
				
				if (role == "mafia1" || role == "mafia2") {
					if (this.player('spy') != noPlayer) {
						var p = this.player('spy');
						
						var r = Math.random();
						
						if (r < 0.3333) {
							this.sendPlayer(p, "±Game: The " + this.tr(role) + " are going to kill " + commandData + "!");
							if (r < 0.1) {
								this.sendPlayer(p, "±Game: " + sys.name(src) + " is one of The " + this.tr(role) + "!");
							}
						}
					}
				}

                return;
            }
        } else if (this.state == "day") {
            if (this.isInGame(sys.name(src)) && command == "vote") {
				commandData = this.correctCase(commandData);
                if (!this.isInGame(commandData)) {
                    sys.sendMessage(src, "±Game: That person is not playing!", mafiachan);
                    return;
                }
                if (sys.name(src) in this.votes) {
                    sys.sendMessage(src, "±Rule: You already voted!", mafiachan);
                    return;
                }
                sys.sendAll("±Game:" + sys.name(src) + " voted for " + commandData + "!", mafiachan);
                this.votes[sys.name(src)] = commandData;

		if (mafia.ticks < 8) {
			mafia.ticks = 8;
		}
                return;
            }
        }
		
        if (sys.auth(src) == 0)
            return;
		
        if (command in this.commands["auth"]) {
            this.commands["auth"][command][0](src);
            return;
        }
		
		throw ("no valid command");
	}
}();

/* stolen from here: http://snippets.dzone.com/posts/show/849 */
function shuffle(o) {
    for(var j, x, i = o.length; i; j = parseInt(Math.random() * i), x = o[--i], o[i] = o[j], o[j] = x);
    return o;
}

/* stolen from here: http://stackoverflow.com/questions/1026069/capitalize-first-letter-of-string-in-javascript */
function cap(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

function dump(src, mess) {
    for (x in mess) {
        sys.sendMessage(src, mess[x], mafiachan);
    }
}

({
/* Executed every second */
stepEvent: function() {
    mafia.tickDown();
}
,

repeatStepEvent: function(globalCounter) {
    if (stepCounter != globalCounter) {
        return;
    }
   
    stepCounter = stepCounter+1;
    sys.callLater("script.repeatStepEvent(" + stepCounter + ")", 1);
   
    /* Using script. instead of this. so as to stop it when this function is removed */
    script.stepEvent();
}

,
startStepEvent: function() {
    stepCounter = 0;

    this.repeatStepEvent(0);
}
,
serverStartUp : function() {
    scriptChecks = 0;
    this.init();
}
,

init : function() {
    lastMemUpdate = 0;
	this.startStepEvent();
	
	var dwlist = ["Rattata", "Raticate", "Nidoran-F", "Nidorina", "Nidoqueen", "Nidoran-M", "Nidorino", "Nidoking", "Oddish", "Gloom", "Vileplume", "Bellossom", "Bellsprout", "Weepinbell", "Victreebel", "Ponyta", "Rapidash", "Farfetch'd", "Doduo", "Dodrio", "Exeggcute", "Exeggutor", "Lickitung", "Lickilicky", "Tangela", "Tangrowth", "Kangaskhan", "Sentret", "Furret", "Cleffa", "Clefairy", "Clefable", "Igglybuff", "Jigglypuff", "Wigglytuff", "Mareep", "Flaaffy", "Ampharos", "Hoppip", "Skiploom", "Jumpluff", "Sunkern", "Sunflora", "Stantler", "Poochyena", "Mightyena", "Lotad", "Ludicolo", "Lombre", "Taillow", "Swellow", "Surskit", "Masquerain", "Bidoof", "Bibarel", "Shinx", "Luxio", "Luxray", "Psyduck", "Golduck", "Growlithe", "Arcanine", "Scyther", "Scizor", "Tauros", "Azurill", "Marill", "Azumarill", "Bonsly", "Sudowoodo", "Girafarig", "Miltank", "Zigzagoon", "Linoone", "Electrike", "Manectric", "Castform", "Pachirisu", "Buneary", "Lopunny", "Glameow", "Purugly", "Natu", "Xatu", "Skitty", "Delcatty", "Eevee", "Vaporeon", "Jolteon", "Flareon", "Espeon", "Umbreon", "Leafeon", "Glaceon", "Eevee", "Bulbasaur", "Charmander", "Squirtle", "Ivysaur", "Venusaur", "Charmeleon", "Charizard", "Wartortle", "Blastoise", "Croagunk", "Toxicroak", "Turtwig", "Grotle", "Torterra", "Chimchar", "Infernape", "Monferno", "Piplup", "Prinplup", "Empoleon", "Treecko", "Sceptile", "Grovyle", "Torchic", "Combusken", "Blaziken", "Mudkip", "Marshtomp", "Swampert", "Caterpie", "Metapod", "Butterfree", "Pidgey", "Pidgeotto", "Pidgeot", "Spearow", "Fearow", "Zubat", "Golbat", "Crobat", "Aerodactyl", "Hoothoot", "Noctowl", "Ledyba", "Ledian", "Yanma", "Yanmega", "Murkrow", "Honchkrow", "Delibird", "Wingull", "Pelipper", "Swablu", "Altaria", "Starly", "Staravia", "Staraptor", "Gligar", "Gliscor", "Drifloon", "Drifblim", "Skarmory", "Tropius", "Chatot", "Slowpoke", "Slowbro", "Slowking", "Krabby", "Kingler", "Horsea", "Seadra", "Kingdra", "Goldeen", "Seaking", "Magikarp", "Gyarados", "Omanyte", "Omastar", "Kabuto", "Kabutops", "Wooper", "Quagsire", "Qwilfish", "Corsola", "Remoraid", "Octillery", "Mantine", "Mantyke", "Carvanha", "Sharpedo", "Wailmer", "Wailord", "Barboach", "Whiscash", "Clamperl", "Gorebyss", "Huntail", "Relicanth", "Luvdisc", "Buizel", "Floatzel", "Finneon", "Lumineon", "Tentacool", "Tentacruel", "Corphish", "Crawdaunt", "Lileep", "Cradily", "Anorith", "Armaldo", "Feebas", "Milotic", "Shellos", "Gastrodon", "Lapras", "Dratini", "Dragonair", "Dragonite", "Elekid", "Electabuzz", "Electivire", "Poliwag", "Poliwrath", "Politoed", "Poliwhirl", "Vulpix", "Ninetales", "Mushaana", "Munna", "Hihidaruma", "Darumakka",];
    dwpokemons = [];
    for(var dwpok in dwlist) {
        dwpokemons.push(sys.pokeNum(dwlist[dwpok]));
    }
	var lclist = ["Bulbasaur", "Charmander", "Squirtle", "Croagunk", "Turtwig", "Chimchar", "Piplup", "Treecko","Torchic","Mudkip"]
    lcpokemons = [];
    for(var dwpok in lclist) {
        lcpokemons.push(sys.pokeNum(lclist[dwpok]));
    }
	
	var inconsistentList = ["Remoraid", "Bidoof", "Snorunt", "Smeargle", "Bibarel", "Octillery", "Glalie"];
	inpokemons = [];
    for(var inpok in inconsistentList) {
        inpokemons.push(sys.pokeNum(inconsistentList[inpok]));
    }
	
	var breedingList = ["Eevee", "Umbreon", "Espeon", "Vaporeon", "Jolteon", "Flareon", "Leafeon", "Glaceon", "Bulbasaur", "Ivysaur", "Venusaur", "Charmander", "Charmeleon", "Charizard", "Squirtle", "Wartortle", "Blastoise", "Croagunk", "Toxicroak", "Turtwig", "Grotle", "Torterra", "Chimchar", "Monferno", "Infernape", "Piplup", "Prinplup", "Empoleon", "Treecko", "Grovyle", "Sceptile", "Torchic", "Combusken", "Blaziken", "Mudkip", "Marshtomp", "Swampert"];
	breedingpokemons = [];
    for(var inpok in breedingList) {
        breedingpokemons.push(sys.pokeNum(breedingList[inpok]));
    }
	
	if (typeof(varsCreated) != 'undefined')
        return;

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
   
	if (sys.existChannel("Mafia Channel")) {
        mafiachan = sys.channelId("Mafia Channel");
    } else {
        mafiachan = sys.createChannel("Mafia Channel");
    }
	
	channelTopics[mafiachan] = 'Use /help to get started!';
    permChannels[mafiachan] = true;
  

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
	sys.setPA("mban");
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
	
	if (sys.existChannel("Tournaments")) {
        tourchannel = sys.channelId("Tournaments");
    } else {
        tourchannel = sys.createChannel("Tournaments");
    }
	
	channelTopics[tourchannel] = 'Useful commands are "/join" (to join a tournament), "/unjoin" (to leave a tournament), "/viewround" (to view the status of matches) and "/megausers" (for a list of users who manage tournaments). Please read the full Tournament Guidelines: http://pokemon-online.eu/forums/showthread.php?2079-Tour-Rules';
    permChannels[tourchannel] = true;
	
	if (sys.existChannel("League")== false) { 
        sys.createChannel("League");
    }
	channelTopics[sys.channelId("League")] = "Home of Gym Leaders and Elite Four";
    permChannels[sys.channelId("League")] = true;
   
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
    if (channel == staffchannel && (!megaUser[src] && sys.auth(src) <= 0 || sys.name(src) == "Emac")) {
        sys.sendMessage(src, "+Guard: Sorry, the access to that place is restricted!");
        sys.stopEvent();
        return;
    }    
	if (channel == mafiachan && mban[src] == true) {
	   sys.sendMessage(src, "+Guard: Sorry, but you are banned from Mafia!")
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
    /* roflma0z */
    if ( sys.ip(src).substr(0, 9) == "190.253.1" ||  sys.ip(src).substr(0, 7) == "190.67." ||  sys.ip(src).substr(0, 7) == "190.66.") {
        sys.kick(src);
        return;
    }
    /* Ema */
    if ( sys.ip(src).substr(0, 7) == "128.187"){
        sys.kick(src);
        return;
    }
    /* Vaginagirl */
    if ( sys.ip(src).substr(0, 5) == "41.96"){
        sys.kick(src);
        return;
    }
    sys.sendMessage(src, "*** Type in /Rules to see the rules. ***");
    sys.sendMessage(src, "+CommandBot: Use !commands to see the commands!");

    if (sys.getVal("muted_*" + sys.ip(src)) == "true")
        muted[src] = true;
    else
        muted[src] = false;
	if (sys.getVal("mban_*" + sys.ip(src)) == "true")
        mban[src] = true;
    else
        mban[src] = false;
       
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
   
    
   
    this.afterChangeTeam(src);

if (sys.auth(src) > 0 && sys.auth(src) <= 3 || megaUser[src] == true && sys.name(src) != "Emac")
        sys.putInChannel(src, staffchannel);
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
	if (tier != "Dream World" && tier != "Dream World Ubers" && tier != "LC Dream World" && tier != "Monotype" && tier != "Dream World UU" && tier != "Weatherless" && tier != "Challenge Cup" && tier != "Uber Triples" && tier != "OU Triples" && tier != "Uber Doubles" && tier != "OU Doubles") {
		this.dreamWorldAbilitiesCheck(src, false);
	}
	if (tier == "LC Wifi" || tier == "LC Ubers Wifi") {
		this.littleCupCheck(src, false);
			
	}
	if (tier == "Dream World" ||tier == "Dream World UU" || tier == "Wifi" || tier == "Wifi UU" || tier == "LC Wifi" || tier == "LC Dream World") {
		this.inconsistentCheck(src, false);
	}
	else if (tier == "Monotype"){
       this.monotypecheck(src)
	   
    }
    else if (tier == "Weatherless"){
       this.weatherlesstiercheck(src)
    }

}

,
beforeChatMessage: function(src, message, chan) {
    channel = chan;
    if (message.length > 350 && sys.auth(src) < 2) {
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
		
		if (channel == mafiachan) {
			try {
				mafia.handleCommand(src, message.substr(1));
				return;
			} catch (err) {
				if (err != "no valid command")
					return;
			}
		}
		
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
            sendChanMessage(src, "/selfkick: to kick any ghosts left behind...");
            sendChanMessage(src, "/join: allows you to join a tournament.");
			sendChanMessage(src, "/auth [owners/admins/mods]: allows you to view the auth of that class, will show all auth if left blank")
            sendChanMessage(src, "/unjoin: allows you to leave a tournament.");
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
            sendChanMessage(src, "/cancelBattle name1: Allows the user or his/her opponent to forfeit his/her current battle so he/she can battle again his/her opponent.");
            if (sys.auth(src) < 1)
                return;
            sendChanMessage(src, "*** Mod Commands ***");
            sendChanMessage(src, "/k [person]:[channel] : to kick someone (Channel is optional, will kick from server if no channel name is given)");
            sendChanMessage(src, "/banlist  [search term]: to search the banlist, shows full list if no search term is entered.");
            sendChanMessage(src, "/[mute/unmute] [person] : You know what i mean :p.");
            sendChanMessage(src, "/silence [x]: To call forth x minute of silence in the main chat (except for auth)");
            sendChanMessage(src, "/silenceoff: To undo that");
			sendChanMessage(src, "/mafiaban [person]: To ban a player from Mafia");
			sendChanMessage(src, "/mafiaunban [person]: To unban a player from Mafia");
            sendChanMessage(src, "/meon, /meoff: to deal with /me happy people");
            sendChanMessage(src, "/perm [on/off]: To make the current channel a permanent channel or not -- i.e. the channel wouldn't be destroyed on log off");
            if (sys.auth(src) < 2)
                return; 
            sendChanMessage(src, "*** Admin Commands ***");
            sendChanMessage(src, "/memorydump: To see the state of the memory.");
            sendChanMessage(src, "/megauser[off] xxx: Tourney powers.");
			sendChanMessage(src, "/aliases xxx: See the aliases of an IP.");
			sendChanMessage(src, "/ban [name]: To ban a user.");
			sendChanMessage(src, "/unban [name]: To unban a user.");
			sendChanMessage(src, "/destroychan [channel]: Will destroy a channel (Certain channels are immune obviously...)");
            if (sys.auth(src) < 3)
                return;
            sendChanMessage(src, "*** Owner Commands ***");
            sendChanMessage(src, "/changeRating [player] -- [tier] -- [rating]: to change the rating of a rating abuser");
            sendChanMessage(src, "/stopBattles: to stop all new battles. When you want to close the server, do that");
            sendChanMessage(src, "/imp [person] : to impersonate someone");
            sendChanMessage(src, "/impOff : to stop impersonating.");
			sendChanMessage(src, "/clearpass [name]: To clear a password")
			sendChanMessage(src, "/sendAll [message] : to send a message to everyone.");
            sendChanMessage(src, "/changeAuth [auth] [person]: to play the mega admin");
            sendChanMessage(src, "/showteam xxx: To help people who have problems with event moves or invalid teams.");
            
            return;
        }
       
        if (command == "me" && !muteall && channel != mafiachan) {
            if (typeof(meoff) != "undefined" && meoff != false) {
                sendChanMessage(src, "+Bot: /me was turned off.");
                return;
            }
              var m = message.toLowerCase();
			    if (m.indexOf("nigger") != -1 || m.indexOf('\u202E') != -1 || m.indexOf("drogendealer") != -1 || m.indexOf("penis") != -1 ||  m.indexOf("vagina")  != -1 || m.indexOf("fuckface") != -1) {
					sys.stopEvent();
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
		if (command == "auth") {
		var authlist = sys.dbAuths().sort()
		sendChanMessage(src, "");
		if(commandData == "owners") {
		   sendChanMessage(src, "*** Owners ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 3) {
		        if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
		   }
		   sendChanMessage(src, "");
		}
		if(commandData == "admins" || commandData == "administrators") {
		   sendChanMessage(src, "*** Administrators ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 2) {
		          if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
		   }
		   sys.sendMessage(src, "");
		}
		if(commandData == "mods" || commandData == "moderators") {
		   sendChanMessage(src, "*** Moderators ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 1) {
		        if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
		   }
		   sys.sendMessage(src, "");
		}
		
		if(commandData != "moderators" && commandData != "mods" && commandData != "administrators" && commandData != "admins" && commandData != "owners") {
		   
		   sendChanMessage(src, "*** Owners ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 3) {
		      if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
		   }
		   sendChanMessage(src, "");
		   sendChanMessage(src, "*** Administrators ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 2) {
		        if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
			
		   }
		   sendChanMessage(src, "");
		   sendChanMessage(src, "*** Moderators ***")
		   for(x in authlist) {
		   if(sys.dbAuth(authlist[x]) == 1) {
		      if(sys.id(authlist[x]) == undefined) {
		        sendChanMessage(src, authlist[x] + " (Offline)")
			  }
				if(sys.id(authlist[x]) != undefined) {
		        sys.sendHtmlMessage(src, '<timestamp/><font color = "green">' + sys.name(sys.id(authlist[x])) + ' (Online)</font>',channel)
		    }
			}
		   }
		}
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
		if (command == "unjoin") {
			if (tourmode == 0) {
                sendChanMessage(src, "+TourneyBot: Wait till the tournament has started.");
                return;
            }
            var name2 = sys.name(src).toLowerCase();
            
            if (tourmembers.indexOf(name2) != -1) {
                tourmembers.splice(tourmembers.indexOf(name2),1);
                delete tourplayers[name2];
                sys.sendAll("+TourneyBot: " + sys.name(src) + " left the tournament!", tourchannel);
                return;
            }
            if (tourbattlers.indexOf(name2) != -1) {
                battlesStarted[Math.floor(tourbattlers.indexOf(name2)/2)] = true;
                sys.sendAll("+TourneyBot: " + sys.name(src) + " left the tournament!", tourchannel);
                this.tourBattleEnd(this.tourOpponent(name2), name2);
            }
            return;
        }
        if (command == "selfkick" || command == "sk") {
		   var src_ip = sys.ip(src);
           var players = sys.playerIds();
           var players_length = players.length;
               for (var i = 0; i < players_length; ++i) {
                   var current_player = players[i];
                   if ((src != current_player) && (src_ip == sys.ip(current_player))) {
                   sys.kick(current_player);
                   sys.sendMessage(src, "+Bot: Your ghost was kicked...")
                   }
                }
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
			if (tourmode == 2) {
            sys.sendAll("+TourneyBot: " +commandData + " was added to the tournament by " + sys.name(src) + ".", tourchannel);
            tourmembers.push(commandData.toLowerCase());
            tourplayers[commandData.toLowerCase()] = commandData;
			}
			if (tourmode == 1) {
			tourmembers.push(commandData.toLowerCase());
            tourplayers[commandData.toLowerCase()] = commandData;
			sys.sendAll("+TourneyBot: " +commandData + " was added to the tournament by " + sys.name(src) + ". " + this.tourSpots() + " more spot(s) left!", tourchannel);
            
            }
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
		if (command == "mafiaban") {
		    
			if (tar == undefined) {
			    ip = sys.dbIp(commandData)	
            alias=sys.aliases(ip)
			y=0
            for(var x in alias) {
			z = sys.dbAuth(alias[x])
                if (z > y) {
                	y=z
				}
			}
			if(y>=sys.auth(src)) {
			   sendChanMessage(src, "+MafiaBot: Can't do that to higher auth!");
			   return;
			}
			    if(sys.dbIp(commandData) != undefined) {
			    ip = sys.dbIp(commandData)
				if (sys.getVal("mban_*" + ip) == "true") {
				sendChanMessage(src, "+MafiaBot: He/she's already banned from Mafia.");
				return;
				}
				sys.sendAll("+MafiaBot: " + commandData + " was banned from Mafia by " + sys.name(src) + "!");
                sys.saveVal("mban_*" + ip, "true");
				return;
				}
				
                sendChanMessage(src, "+MafiaBot: Couldn't find " + commandData);
                return;
            }
			
		
            if (mban[tar]) {
                sendChanMessage(src, "+MafiaBot: He/she's already banned from Mafia.");
                return;
            }
            if (sys.auth(tar) >= sys.auth(src)) {
                sendChanMessage(src, "+MafiaBot: You dont have sufficient auth to Mafia ban " + commandData + ".");
                return;
            }
			
            sys.sendAll("+MafiaBot: " + commandData + " was banned from Mafia by " + sys.name(src) + "!");
            mban[tar] = true;
            sys.saveVal("mban_*" + sys.ip(tar), "true");
			sys.kick(tar, mafiachan);
			commandData = mafia.correctCase(commandData)
			if (mafia.isInGame(commandData)) {
				mafia.removePlayer(commandData)
				mafia.testWin()
            }
			return;
		}
		if (command == "mafiaunban") {
		if (tar == undefined) {
			if(sys.dbIp(commandData) != undefined) {
			    ip = sys.dbIp(commandData)
				if (sys.getVal("mban_*" + ip) == "true") {
				sys.sendAll("+MafiaBot: " + commandData + " was unbanned from Mafia by " + sys.name(src) + "!");
                sys.removeVal("mban_*" + ip);
				return;
				}
				sendChanMessage(src, "+MafiaBot: He/she's not banned from Mafia.");
                return;
				}
                return;
            }
            if (!mban[tar]) {
                sendChanMessage(src, "+MafiaBot: He/she's not banned from Mafia.");
                return;
            }
			if(mban[src] && tar==src) {
			   sendChanMessage(src, "+MafiaBot: You may not unban yourself from Mafia");
			   return;
			}
            sys.sendAll("+MafiaBot: " + commandData + " was unbanned from Mafia by " + sys.name(src) + "!");
            mban[tar] = false;
            sys.removeVal("mban_*" + sys.ip(tar));
            return;
        }
		
        if (command == "impoff") {
            delete impersonation[src];
            sendChanMessage(src, "+Bot: Now you are yourself!");
            return;
        }
        if (command == "k") {
		    if (commandData.indexOf(':') != -1) {
			commandData = commandData.split(':');
		    tar = sys.id(commandData[0])
			if (tar == undefined) {
                return;
            }
			if(typeof(commandData[1]) != undefined) {
			if (sys.existChannel(commandData[1])) {		
             ch = sys.channelId(commandData[1])		
			 if(ch == 0) {
			 sendChanMessage(src, "+Bot: Cannot kick from main channel!")
			 return;			 
			 }
			 if(sys.isInChannel(tar, ch) != true) {
			 sendChanMessage(src, "+Bot: Player is not in this channel!")
			 return;
			 }
             if(sys.isInChannel(src, ch) != true) {			 
			 sendChanMessage(src, "+Bot: You kicked " + commandData[0] + "  from the channel" + commandData[1] + "!");
			 }
			 sys.sendAll("+Bot: " + commandData[0] + " was mysteriously kicked from this channel by " + sys.name(src) + "!",ch);
             sys.kick(tar, ch)
			 if (sys.isInChannel(tar, 0) != true) {
		     sys.putInChannel(tar, 0)
			 }
			 return;
		    }
			sendChanMessage(src, "+Bot: Channel " + commandData[1] + " does not exist!");
			return
			}
			}
			if (tar == undefined) {
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was mysteriously kicked by " + sys.name(src) + "!");
            sys.kick(tar);
            return;
        }
		
		
		if (command == "mute") {
            if (tar == undefined) {
			    ip = sys.dbIp(commandData)	
            alias=sys.aliases(ip)
			y=0
            for(var x in alias) {
			z = sys.dbAuth(alias[x])
                if (z > y) {
                	y=z
				}
			}
			if(y>=sys.auth(src)) {
			   sendChanMessage(src, "+Bot: Can't do that to higher auth!");
			   return;
			}
			    if(sys.dbIp(commandData) != undefined) {
			    ip = sys.dbIp(commandData)
				if (sys.getVal("muted_*" + ip) == "true") {
				sendChanMessage(src, "+Bot: He/she's already muted.");
				return;
				}
				sys.sendAll("+Bot: " + commandData + " was muted by " + sys.name(src) + "!");
                sys.saveVal("muted_*" + ip, "true");
			    return;
				}
				
                sendChanMessage(src, "+Bot: Couldn't find " + commandData);
                return;
            }
			
		
            if (muted[tar]) {
                sendChanMessage(src, "+Bot: He/she's already muted.");
                return;
            }
            if (sys.auth(tar) >= sys.auth(src)) {
                sendChanMessage(src, "+Bot: You dont have sufficient auth to mute " + commandData + ".");
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was muted by " + sys.name(src) + "!");
            muted[tar] = true;
            sys.saveVal("muted_*" + sys.ip(tar), "true");
			sys.appendToFile('mutes.txt', "\n" + sys.name(src) + ' muted ' + commandData)
            return;
        }
		if (command == "banlist") {
		list=sys.banList();
		list.sort();
		var nbr_banned=5;
		var table='';
		table+='<table border="1" cellpadding="5" cellspacing="0"><tr><td colspan='+nbr_banned+'><center><strong>Banned list</strong></center></td></tr><tr>';
		var j=0;
		for(var i=0;(i<list.length);i++){
		if(typeof(commandData) == 'undefined' || commandData.toLowerCase() == list[i].substr(0, commandData.length).toLowerCase()){
		j++;
		table+='<td>'+list[i]+'</td>';
		if(j == nbr_banned &&  i+1 != list.length){
		table+='</tr><tr>';
		j=0
		}
		}
		}
		table+='</tr></table>';
		sys.sendHtmlMessage(src, table.replace('</tr><tr></tr></table>', '</tr></table>'),channel);
		return;
		
}
        if (command == "unmute") {
            if (tar == undefined) {
			if(sys.dbIp(commandData) != undefined) {
			    ip = sys.dbIp(commandData)
				if (sys.getVal("muted_*" + ip) == "true") {
				sys.sendAll("+Bot: " + commandData + " was unmuted by " + sys.name(src) + "!");
                sys.removeVal("muted_*" + ip);
				sys.appendToFile('mutes.txt', "\n" + sys.name(src) + ' unmuted ' + commandData)
				return;
				}
				sendChanMessage(src, "+Bot: He/she's not muted.");
                return;
				}
                return;
            }
            if (!muted[tar]) {
                sendChanMessage(src, "+Bot: He/she's not muted.");
                return;
            }
            sys.sendAll("+Bot: " + commandData + " was unmuted by " + sys.name(src) + "!");
            muted[tar] = false;
            sys.removeVal("muted_*" + sys.ip(tar));
			sys.appendToFile('mutes.txt', "\n" + sys.name(src) + ' unmuted ' + commandData)
            return;
        }
        if (sys.auth(src) < 2) {
			sendChanMessage(src, "+CommandBot: The command " + command + " doesn't exist");
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
		if (command == "destroychan") {
		ch = commandData
		chid = sys.channelId(ch)
		if(sys.existChannel(ch) != true) {
		sendChanMessage(src, "+Bot: No channel exists by this name!");
		return;
		}
		if (chid == 0 || chid == staffchannel ||  chid == tourchannel || (chid in permChannels && permChannels[chid] == true) ) {
        sendChanMessage(src, "+Bot: This channel cannot be destroyed!");
        return;
        }
		players = sys.playersOfChannel(chid)
		for(x in players) {
		sys.kick(players[x], chid)
		if (sys.isInChannel(players[x], 0) != true) {
		sys.putInChannel(players[x], 0)
		}
		}
		return;
		}
		if (command == "ban") {
		    if(sys.dbIp(commandData) == undefined) {
			sendChanMessage(src, "+Bot: No player exists by this name!");
			return;
			}
		    if (sys.maxAuth(sys.ip(tar))>=sys.auth(src)) {
			   sendChanMessage(src, "+Bot: Can't do that to higher auth!");
			   return;
			}
            ip = sys.dbIp(commandData)	
            alias=sys.aliases(ip)
			y=0
            for(var x in alias) {
			z = sys.dbAuth(alias[x])
                if (z > y) {
                	y=z
				}
			}
			if(y>=sys.auth(src)) {
			   sendChanMessage(src, "+Bot: Can't do that to higher auth!");
			   return;
			}
			banlist=sys.banList()
			for(a in banlist) {
			if(sys.dbIp(commandData) == sys.dbIp(banlist[a])) {
			sendChanMessage(src, "+Bot: He/she's already banned!");
			return;
			}
			}
			
			sys.sendHtmlAll('<b><font color=red>' + commandData + ' was banned by ' + sys.name(src) + '!</font></b>');
			if(tar != undefined) {
			sys.kick(tar)
			}
			sys.ban(commandData)
			sys.appendToFile('bans.txt', sys.name(src) + ' banned ' + commandData + "\n")
			return;
		}	
		if (command == "unban") {
		if(sys.dbIp(commandData) == undefined) {
			sendChanMessage(src, "+Bot: No player exists by this name!");
			return;
			}
		banlist=sys.banList()
			for(a in banlist) {
			if(sys.dbIp(commandData) == sys.dbIp(banlist[a])) {
			sys.unban(commandData)
			sendChanMessage(src, "+Bot: You unbanned " + commandData + "!");
			sys.appendToFile('bans.txt', sys.name(src) + ' unbanned ' + commandData + "\n")
			return;
			
			}
			}			
		    sendChanMessage(src, "+Bot: He/she's not banned!");
			return;
	    }

		if (command == "aliases") {
			sendChanMessage(src, "+IpBot: The aliases for the IP " + commandData + " are: " + sys.aliases(commandData) + ".");
			return;
		}
        if (sys.auth(src) < 3) {
			sendChanMessage(src, "+CommandBot: The command " + command + " doesn't exist");
            return;
        }
        /** Owner Commands **/
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
        if (command == "showteam") {
            sendChanMessage(src, "");
            for (var i = 0; i < 6; i+=1) {sendChanMessage(src, sys.pokemon(sys.teamPoke(tar, i)) + " @ " + sys.item(sys.teamPokeItem(tar, i)));
            for (var j = 0; j < 4; j++) {sendChanMessage(src, '- ' + sys.move(sys.teamPokeMove(tar, i, j)));}}
            sendChanMessage(src, "");
			return;
        }
        if (command == "sendall") {
            sendChanAll(commandData);
            return;
        }
        if (command == "imp") {
            impersonation[src] = commandData;
            sendChanMessage(src, "+Bot: Now you are " + impersonation[src] + "!");
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
		if (command == "variablereset") {
		    delete varsCreated
			this.init()
		}
        if (command == "eval" && (sys.ip(src) == sys.dbIp("coyotte508") || sys.name(src).toLowerCase() == "crystal moogle")) {
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
		   if (command == "clearpass") {
            var mod = sys.name(src);
            sys.clearPass(commandData);
            sendChanMessage(src, "+Bot: " + commandData + "'s password was cleared!");
            sys.sendMessage(tar, "+Bot: Your password was cleared by " + mod + "!");
            return;
        }

		sendChanMessage(src, "+CommandBot: The command " + command + " doesn't exist");
        return;
    }
	if (channel == mafiachan && mafia.ticks > 0 && mafia.state!="blank" && !mafia.isInGame(sys.name(src)) && sys.auth(src) <= 0) {
		sys.stopEvent();
		sys.sendMessage(src, "±Game: You're not playing, so shush! Go in another channel to talk!", mafiachan);
		return;
	}
    if (typeof impersonation[src] != 'undefined') {
        sys.stopEvent();
        sendChanAll(impersonation[src] + ": " + message);
        return;
    }
    if (sys.auth(src) == 0 && muteall && channel != staffchannel && channel != mafiachan) {
        sendChanMessage(src, "+Bot: Respect the minutes of silence!");
        sys.stopEvent();
        return;
    }
    var m = message.toLowerCase();
   
    if (m.indexOf("nimp.org") != -1 ||m.indexOf("drogendealer") != -1 ||m.indexOf("nigger") != -1 || m.indexOf('\u202E') != -1 || m.indexOf("penis") != -1 ||  m.indexOf("vagina")  != -1 || m.indexOf("fuckface") != -1) {
        sys.stopEvent();
        return;
    }
}

,

afterChatMessage : function(src, message, chan)
{
    channel = chan;
    lineCount+=1;
   
    if (this.isMCaps(message) && sys.auth(src) < 2 && channel != staffchannel) {
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
   
    if (sys.auth(src) < 2 && channel != staffchannel) {
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
beforeChangeTier : function(src, oldtier, newtier) {
   if (newtier.toLowerCase() == "monotype"){
       this.monotypecheck(src)
	   
   }
    if (newtier.toLowerCase() == "weatherless"){
       this.weatherlesstiercheck(src)
   }
   
}
,
beforeChallengeIssued : function (src, dest, clauses, rated, mode) {
    if (battlesStopped) {
        sys.sendMessage(src, "+BattleBot: Battles are now stopped as the server will restart soon.");
        sys.stopEvent();
        return;
    }
   
    if (forceSameTier[dest] == true && (sys.tier(dest) != sys.tier(src))) {
        sys.sendMessage(src, "+BattleBot: That guy only wants to fight his/her own tier.");
        sys.stopEvent();
        return;
    }
   
    if (sys.tier(src) == "Challenge Cup" && sys.tier(dest) == "Challenge Cup" && clauses[4] == 0 || sys.tier(src) == "1v1 Challenge Cup" && sys.tier(dest) == "1v1 Challenge Cup" && clauses[4] == 0) {
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
                sys.sendMessage(src, "+TourneyBot: This guy is in the tournament and you are not, so you can't battle him/her.");
                sys.stopEvent();
                return;
            }
        }
    }
   
    /* Challenge Cup Clause */
    if (clauses[4] == 1)
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
       
        if (tier != "Dream World" && tier != "Dream World Ubers" && tier != "LC Dream World" && tier != "Monotype" && tier != "Dream World UU" && tier != "Weatherless" && tier != "Challenge Cup" && tier != "Uber Triples" && tier != "OU Triples" && tier != "Uber Doubles" && tier != "OU Doubles") {
            this.dreamWorldAbilitiesCheck(src,true);
            this.dreamWorldAbilitiesCheck(dest,true);
        }
		if (tier == "Dream World" ||tier == "Dream World UU" || tier == "Wifi" || tier == "Wifi UU" || tier == "LC Wifi" || tier == "LC Dream World") {
			this.inconsistentCheck(src, true);
			this.inconsistentCheck(dest, true);
		}
		if (tier == "LC Wifi" || tier == "LC Ubers Wifi") {
		    this.littleCupCheck(src, true);
			this.littleCupCheck(dest, true);
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
       
        if (tier != "Dream World" && tier != "Dream World Ubers" && tier != "LC Dream World" && tier != "Monotype" && tier != "Dream World UU" && tier != "Weatherless" && tier != "Challenge Cup" && tier != "Uber Triples" && tier != "OU Triples" && tier != "Uber Doubles" && tier != "OU Doubles") {
            this.dreamWorldAbilitiesCheck(src,true);
            this.dreamWorldAbilitiesCheck(dest,true);
        }
		if (tier == "Dream World" ||tier == "Dream World UU" || tier == "Wifi" || tier == "Wifi UU" || tier == "LC Wifi" || tier == "LC Dream World") {
			this.inconsistentCheck(src, true);
			this.inconsistentCheck(dest, true);
		}
		if (tier == "LC Wifi" || tier == "LC Ubers Wifi") {
		    this.littleCupCheck(src, true);
			this.littleCupCheck(dest, true);
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
littleCupCheck : function(src, se) {
 for (var i = 0; i < 6; i++) {
        var x = sys.teamPoke(src, i);
        if (x != 0 && sys.hasDreamWorldAbility(src, i) && lcpokemons.indexOf(x) != -1 ) {
            if (se) {
					sys.sendMessage(src, "+CheckBot: " + sys.pokemon(x) + " is not allowed with a Dream World ability in this tier. Change it in the teambuilder.");
					}
            
			if (sys.tier(src) == "LC Wifi" && sys.hasLegalTeamForTier(src, "LC Dream World") || sys.tier(src) == "LC Ubers Wifi" && sys.hasLegalTeamForTier(src, "Dream World")) {
                sys.changeTier(src, "LC Dream World");
            }else {
                if (se)
                    sys.changePokeNum(src, i, 0);
            
			}
            if (se)
                sys.stopEvent();
        }
    }
}
,
dreamWorldAbilitiesCheck : function(src, se) {
	 if (sys.gen(src) <= 4)
			return;
    for (var i = 0; i < 6; i++) {
        var x = sys.teamPoke(src, i);
        if (x != 0 && sys.hasDreamWorldAbility(src, i) && (dwpokemons.indexOf(x) == -1 || (breedingpokemons.indexOf(x) != -1 && sys.compatibleAsDreamWorldEvent(src, i) != true))) {
            if (se) {
				if (dwpokemons.indexOf(x) == -1)
					sys.sendMessage(src, "+CheckBot: " + sys.pokemon(x) + " is not allowed with a Dream World ability in this tier. Change it in the teambuilder.");
				else
					sys.sendMessage(src, "+CheckBot: " + sys.pokemon(x) + " has to be Male and have no egg moves with its Dream World ability in  " + sys.tier(src) + " tier. Change it in the teambuilder.");
			}
            if (sys.tier(src) == "Wifi" && sys.hasLegalTeamForTier(src, "Dream World")) {
                sys.changeTier(src, "Dream World");
            } else if (sys.tier(src) == "Wifi Ubers") {
                sys.changeTier(src, "Dream World Ubers");
            } 
			else if (sys.tier(src) == "1v1 Gen 5" && sys.hasLegalTeamForTier(src, "Dream World")) {
                sys.changeTier(src, "Dream World");
			}
			else if (sys.tier(src) == "1v1 Gen 5" && sys.hasLegalTeamForTier(src, "Dream World Ubers")) {
                sys.changeTier(src, "Dream World Ubers");
			}
			else if (sys.tier(src) == "Wifi UU" && sys.hasLegalTeamForTier(src, "Dream World UU")) {
                sys.changeTier(src, "Dream World UU");
			}
			else if (sys.tier(src) == "LC Wifi" && sys.hasLegalTeamForTier(src, "LC Wifi") || sys.tier(src) == "LC Ubers Wifi" && sys.hasLegalTeamForTier(src, "LC Ubers Wifi")) {
                sys.changeTier(src, "LC Dream World");
            }else {
                if (se)
                    sys.changePokeNum(src, i, 0);
            
			}
            if (se)
                sys.stopEvent();
        }
    }
}
,

inconsistentCheck : function(src, se) {
    for (var i = 0; i < 6; i++) {
        var x = sys.teamPoke(src, i);
       
        if (x != 0 && inpokemons.indexOf(x) != -1 && sys.hasDreamWorldAbility(src, i)) {
            if (se)
                sys.sendMessage(src, "+CheckBot: " + sys.pokemon(x) + " is not allowed with Inconsistent in this tier. Change it in the teambuilder.");
            if (sys.tier(src) == "Wifi" || sys.tier(src) == "Wifi UU" ) {
                sys.changeTier(src, "Wifi Ubers");
            } else if (sys.tier(src) == "Dream World" || sys.tier(src) == "Dream World UU") {
                sys.changeTier(src, "Dream World Ubers");
			  }
			  else if(sys.tier(src) == "LC Wifi") {
			    sys.changeTier(src, "LC Ubers Wifi") 
			  }
			  else if (sys.tier(src) == "LC Dream World") {
			     sys.changeTier(src, "Dream World Ubers") 
			  
            } else {
                if (se)
                    sys.changePokeNum(src, i, 0);
            }
            if (se)
                sys.stopEvent();
        }
    }
}
,
weatherlesstiercheck : function(src) {
	for (var i = 0; i < 6; i++){
	     ability = sys.ability(sys.teamPokeAbility(src, i))
		 if(ability.toLowerCase() == "drizzle" || ability.toLowerCase() == "drought" || ability.toLowerCase() == "snow warning" || ability.toLowerCase() == "sand stream") {
		 sys.sendMessage(src, "+Bot: Your team has a pokémon with the ability: " + ability + ", please remove before entering this tier.");
		 if(sys.hasLegalTeamForTier(src, "Dream World")) {
			if(sys.hasLegalTeamForTier(src,"Wifi")) {
			sys.changeTier(src, "Wifi");
			sys.stopEvent()
			return;
			}
			sys.changeTier(src, "Dream World");
			sys.stopEvent()
			return;
			}
			if(sys.hasLegalTeamForTier(src,"Wifi Ubers")) {
			sys.changeTier(src, "Wifi Ubers");
			sys.stopEvent()
			return;
			}
			sys.changeTier(src, "Dream World Ubers");
			sys.stopEvent()
			return;
			}
		}
	}
	,
	monotypecheck : function(src) {
	TypeA = sys.pokeType1(sys.teamPoke(src, 0), 5)
    TypeB = sys.pokeType2(sys.teamPoke(src, 0), 5)
    for (var i = 0; i < 6; i++) {
         temptypeA = sys.pokeType1(sys.teamPoke(src, i), 5)
	     temptypeB = sys.pokeType2(sys.teamPoke(src, i), 5)
             if (temptypeA == 0) {
                 temptypeA = TypeA
             }
	     if (temptypeA != TypeA && temptypeB != TypeA && temptypeA != TypeB && temptypeB != TypeB) {
			sys.sendMessage(src, "+Bot: Team not Monotype");
			if(sys.hasLegalTeamForTier(src, "Dream World")) {
			if(sys.hasLegalTeamForTier(src,"Wifi")) {
			sys.changeTier(src, "Wifi");
			sys.stopEvent()
			return;
			}
			sys.changeTier(src, "Dream World");
			sys.stopEvent()
			return;
			}
			if(sys.hasLegalTeamForTier(src,"Wifi Ubers")) {
			sys.changeTier(src, "Wifi Ubers");
			sys.stopEvent()
			return;
			}
			sys.changeTier(src, "Dream World Ubers");
			sys.stopEvent()
			return;
			}
		}
	}

})