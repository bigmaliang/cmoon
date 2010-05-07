/*
 * 在线留言板
 */

function liveCS(ape, debug) {
    this.initialize = function(opts) {
		ape.lcsuname = Cookie.read("lcs_uname");
		if (ape.lcsuname == null) {
			ape.lcsuname = bmoon.utl.randomWord(8);
			Cookie.write("lcs_uname", ape.lcsuname, {'path': '/', 'duration': 365});
		}
        ape.lcsaname = opts.aname || document.domain;
        ape.lcsPipeName = "livecspipe_" + ape.lcsaname;
		this.publicPipe = null;
		this.currentPipe = null;

		ape.onRaw("ident", this.rawLcsIdent);
        ape.onRaw("lcsdata", this.rawLcsData);

        ape.onError("110", ape.clearSession);
        ape.onError("111", ape.clearSession);
        ape.onError("112", ape.clearSession);
        ape.onError("113", ape.clearSession);

        ape.addEvent("userJoin", this.createUser);
        ape.addEvent("userLeft", this.deleteUser);
        ape.addEvent("multiPipeCreate", this.pipeCreate);

        ape.addEvent("load", this.start);
    };

    this.start = function() {
		var lcs = this;
		var opt = {'sendStack': false, 'request': 'stack'};
		ape.start({'uin': ape.lcsuname}, opt);
		if (ape.options.restore) {
			ape.getSession('currentPipe', function(resp) {
							   lcs.setCurrentPipe(resp.data.sessions.currentPipe);
						   }, opt);
		} else {
			//ape.request.send("LCS_JOIN", {'aname': ape.lcsaname}, opt);
			ape.request.stack.add("LCS_JOIN", {'aname': ape.lcsaname}, opt);
		}
		ape.request.stack.send();
    };

	this.setCurrentPipe = function(pubid) {
		this.currentPipe = ape.getPipe(pubid);
		ape.setSession({'currentPipe': pubid});
	};

	this.getCurrentPipe = function() {
		return this.currentPipe;
	};

    this.pipeCreate = function(pipe, options) {
        if (pipe.properties.name.toLowerCase() == ape.lcsPipeName) {
			this.publicPipe = pipe;
			if (!this.getCurrentPipe()) {
				this.setCurrentPipe(pipe.getPubid());
			}
		}
    };

    this.createUser = function(user, pipe) {
		if (pipe.properties.isadmin) {
			this.setCurrentPipe(pipe.getPubid());
		}
    };

    this.deleteUser = function(user, pipe) {
		if (pipe.properties.isadmin) {
			this.setCurrentPipe(this.publicPipe.getPubid());
		}
    };

	this.rawLcsIdent = function(raw, pipe) {
		var jid = parseInt(raw.data.user.properties.jid);
		// send LCS_VISIT only on session restore
		if (jid && ape.options.restore) {
			ape.request.send("LCS_VISIT", {'jid': jid, 'url': window.location.href, 'title': window.title});
		}
	};

    this.rawLcsData = function(raw, pipe) {
        if (pipe == this.getCurrentPipe()) {
            var msg = unescape(raw.data.msg);
            var uin = raw.data.from.properties.uin;
            var tm = Date(eval(raw.time)).match(/\d{1,2}:\d{1,2}:\d{1,2}/)[0];
            //bmoon.lcs.userSaid(ape, uin, msg, tm);
        }
    };
}
