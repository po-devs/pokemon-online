/* How to include from your script:
1) Put this at the top of your script:
   sys.include("lib-client.js");
*/

cli = new(function() {
              /* Helpers */
              this.invalidTime = function (n) {
                       return n === null || typeof n !== 'number'
                               || isNaN(n) || n < 1;
                   }

              this.invalidArg = function(arg) {
                       return arg === undefined || arg === null;
                   }

              this.secToMs = this.sec2ms = function (sec) {
                       return sec * 1000;
                   }



              /* Messages */
              this.sendMessage = function (message, channel) {
                       if (this.invalidArg(channel)) {
                           client.printLine(message);
                           return;
                       }

                       client.printChannelMessage(message, channel, false);
                   }

              this.sendHtmlMessage = function (message, channel) {
                       if (this.invalidArg(channel)) {
                           client.printHtml(message);
                           return;
                       }

                       client.printChannelMessage(message, channel, true);
                   }

              /* General */
              this.postMessage = client.sendText;
              this.register = client.sendRegister;
              this.id = client.id;
              this.getTierList = this.tiers =
                   this.tierList = client.getTierList;

              this.kick = function (name) {
                       if (typeof name === "string") {
                           client.kick(this.id(name));
                           return;
                       }

                       client.kick(name);
                   }

              this.ban = function (name) {
                       client.requestBan(name);
                   }

              this.tempBan = this.tempban = function (name, time) {
                       if (this.invalidTime(time)) {
                           time = 10;
                       }
                       client.requestTempBan(name, time);
                   }

              this.startPM = function (name) {
                       if (typeof name === "string") {
                           client.startPM(this.id(name));
                           return;
                       }

                       client.startPM(name);
                   }

              this.endPM = function (name) {
                       if (typeof name === "string") {
                           client.closePM(this.id(name));
                           return;
                       }

                       client.closePM(name);
                   }

              this.cp = this.CP = function (name) {
                       if (typeof name === "string") {
                           client.controlPanel(this.id(name));
                           return;
                       }

                       client.controlPanel(name);
                   }

              this.idle = client.goAwayB;

              /* Channels */
              this.hasChannel = client.hasChannel;
              this.leaveChannel = client.leaveChannel;
              this.switchTo = this.switchToChannel = client.activateChannel;
              this.joinChannel = client.join;

              this.initChannel = function (cid) {
                       if (!this.hasChannel(cid)) {
                           client.channelPlayers(cid);
                       }
                   }

              this.changeChannelName = function (newName, cid) {
                       client.channelNameChanged(cid, newName);
                   }

              /* Options */
              this.sortChannelsByName = client.sortChannelsToggle;
              this.saveBattleLogs = client.saveBattleLogs;
              this.animateHpBar = this.animateHPBar = client.animateHpBar;

              this.changeBattleLogsFolder = client.changeBattleLogsFolder;
              this.openSoundConfig = this.soundConfig = client.openSoundConfig;


              /* Battles */
              this.forfeit = client.forfeitBattle;
              this.stopWatching = this.stopSpectating = client.stopWatching;
              this.watchBattle = client.watchBattle;
              this.watchBattleOf = function (name) {
                       if (typeof name === "string") {
                           client.watchBattleOf(this.id(name));
                           return;
                       }

                       client.watchBattleOf(name);
                   }

              this.challenge = function (name, tier) {
                       if (typeof name === "string") {
                           client.seeInfo(this.id(name));
                           return;
                       }

                       client.seeInfo(name);
                   }

              /* Other */
              this.attach = this.attachEvent = function (event, callBack) {
                       if (event in client) {
                           client[event].connect(callBack);
                       }
                   }

              this.closeWindow = this.goToMenu = function (time) {
                       if (this.invalidTime(time)) {
                           client.done();
                           return;
                       }

                       sys.callQuickly("client.done();", time);
                   }

          })();
