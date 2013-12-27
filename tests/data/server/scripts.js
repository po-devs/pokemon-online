if (typeof load === "undefined") {
    load = 0;
}

if (typeof tests === "undefined") {
    tests = {};
}

load += 1;

if (load === 1 || load === 2) {
    SESSION.identifyScriptAs("SESSION.1");
} else {
    SESSION.identifyScriptAs("SESSION.2");
}

SESSION.registerUserFactory(function () { this.messageSent = false; });
SESSION.registerChannelFactory(function () {});
SESSION.registerGlobalFactory(function () { this.messageSent = false; });

function fail(state) { sys.sendAll("fail", 0); print("state: " + state); }
function success() { sys.sendAll("accept", 0); }
function reloadScripts() { sys.changeScript(sys.getFileContent("scripts.js")); }

tests.session = {
    run: function (src, chan) {
        tests.session.state[1](src, chan);
    },
    state: {
        '1': function (src, chan) {
            print("TestSESSION: State #1");
            if (SESSION.hasUser(src) && SESSION.hasChannel(chan)) {
                SESSION.users(src).messageSent = true;
                SESSION.global().messageSent = true;
                tests.session.state[2](src, chan);
            } else {
                fail(1);
            }
        },
        '2': function (src, chan) {
            reloadScripts();
            // Using the same script id shouldn't reset objects
            print("TestSESSION: State #2");
            if (SESSION.hasUser(src) && SESSION.hasChannel(chan) && SESSION.users(src).messageSent === true) {
                tests.session.state[3](src, chan);
            } else {
                fail(2);
            }
        },
        '3': function (src, chan) {
            reloadScripts();
            // Objects should be reset if the script id is changed
            print("TestSESSION: State #3");
            if (SESSION.hasUser(src) && SESSION.hasChannel(chan) && SESSION.users(src).messageSent === false && SESSION.global().messageSent === false) {
                success();
            } else {
                fail(3);
            }
        }
    }
};

({
afterChatMessage: function (src, message, chan) {
    if (message.substr(0, 6) === "Test: ") {
        tests[message.substr(6)].run(src, chan);
        return;
    }

    if (message.substr(0,6) === "eval: ") {
        sys.sendAll(sys.eval(message.substr(6)), 0);
        return;
    }
},
afterReconnection: function (src) {
    sys.sendAll(sys.name(src) + " reconnected.", 0);
}
})
