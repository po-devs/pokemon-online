if (typeof load === "undefined") {
    load = 0;
}

load += 1;

if (load === 1 || load === 2) {
    SESSION.identifyScriptAs("SESSION.1");
} else {
    SESSION.identifyScriptAs("SESSION.2");
}

SESSION.registerUserFactory(function () { this.messageSent = false; });
SESSION.registerChannelFactory(function () {});

function fail(state) { sys.sendAll("fail", 0); print("state: " + state); }
function success() { sys.sendAll("accept", 0); }
function reloadScripts() {     sys.changeScript(sys.getFileContent("scripts.js")); }
function state2(src, chan) {
    reloadScripts();
    // Using the same script id shouldn't reset objects
    print("TestSESSION: State #2");
    if (SESSION.hasUser(src) && SESSION.hasChannel(chan) && SESSION.users(src).messageSent === true) {
        state3(src, chan);
    } else {
        fail(2);
    }
}

function state3(src, chan) {
    reloadScripts();
    // Objects should be reset if the script id is changed
    print("TestSESSION: State #3");
    if (SESSION.hasUser(src) && SESSION.hasChannel(chan) && SESSION.users(src).messageSent === false) {
        success();
    } else {
        fail(3);
    }
}

({
    afterChatMessage: function (src, message, chan) {
        if (message === "Commence testing!") {
            print("TestSESSION: State #1");
            if (SESSION.hasUser(src) && SESSION.hasChannel(chan)) {
                SESSION.users(src).messageSent = true;
                state2(src, chan);
            } else {
                fail(1);
            }
        }
    }
})