/*
 * 在线留言板
 */

function liveCS(ape, debug) {
    this.initialize = function(opts) {
		ape.lcsuin = opts.lcsuin;
		ape.lcspass = opts.lcspass;
        ape.appid = opts.appid;
        ape.lcsPipeName = "livecspipe_" + ape.appid;
        ape.lcsPipe = null;

        ape.onRaw("lcsdata", this.rawLcsData);
        ape.addEvent("userJoin", this.createUser);
        ape.addEvent("userLeft", this.deleteUser);
        ape.addEvent("multiPipeCreate", this.pipeCreate);

        ape.addEvent("load", this.start);
		ape.onRaw("ident", this.rawIdent);
    };

    this.start = function() {
		ape.start({'uin': ape.lcsuin});
    };

	this.rawIdent = function() {
        ape.request.send("LCS_JOIN_B", {'appid': ape.appid, 'uin': ape.lcsuin, 'pass': ape.lcspass}, true);
	};

    this.pipeCreate = function(pipe, options) {
        if (pipe.properties.name.toLowerCase() == ape.lcsPipeName) {
            ape.lcsPipe = pipe;
        }
    };

    this.createUser = function(user, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.lcsPipeName) {
			//bmoon.lcs.userVisit(ape, user.properties.uin);
        }
    };

    this.deleteUser = function(user, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.lcsPipeName) {
			//bmoon.lcs.userAway(user.properties.uin);
        }
    };

    this.rawLcsData = function(raw, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.lcsPipeName) {
            var msg = unescape(raw.data.msg);
            var uin = raw.data.from.properties.uin;
            var tm = Date(eval(raw.time)).match(/\d{1,2}:\d{1,2}:\d{1,2}/)[0];
            //bmoon.lcs.userSaid(ape, uin, msg, tm);
        }
    };
}
