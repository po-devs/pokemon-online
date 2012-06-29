/* How to include from your script:
1) Put this at the top of your script:
   sys.include("lib-client.js");
*/

cli = new(function() {
              this.invalidTime = function (n) {
                  return n === null || typeof n !== 'number'
                  || isNaN(n) || n < 1;
              }

              this.invalidArg = function(arg) {
                       return arg === undefined || arg === null;
                   }

              this.secToMs = function (sec) {
                       return sec * 1000;
                   }

              this.closeWindow = function (time) {
                       if (this.invalidTime(time)) {
                           client.done();
                           return;
                       }

                       sys.callLater("client.done();", time);
                   }

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

          })();
