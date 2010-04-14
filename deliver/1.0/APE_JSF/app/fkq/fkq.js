/**
 * 社区访客群类
 */

function Fkq(ape, debug) {
    this.initialize = function(opts) {
        ape.hostUin = opts.hostUin || 0;
        ape.fkqPipeName = "fangkequnpipe" + ape.hostUin || "";
        ape.fkqPipe = null;
        
        ape.onRaw("fkqdata", this.rawFkqData);
        ape.addEvent("userJoin", this.createUser);
        ape.addEvent("userLeft", this.deleteUser);
        ape.addEvent("multiPipeCreate", this.pipeCreate);

        // FKQ_JOIN command should carry with session after raw_login,
        // so, can't bind with 'load' event, trigger it after 'ident'
        ape.onRaw("ident", this.start);
    };

    this.start = function() {
        ape.request.send("FKQ_JOIN", {'hostUin': ape.hostUin+""}, true);
    };

    this.pipeCreate = function(pipe, options) {
        if (pipe.properties.name.toLowerCase() == ape.fkqPipeName) {
            ape.fkqPipe = pipe;
        }
    };

    this.createUser = function(user, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.fkqPipeName) {
            hn.fkq.userVisit(ape, user.properties.uin);
        }
    };

    this.deleteUser = function(user, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.fkqPipeName) {
            hn.fkq.userAway(user.properties.uin);
        }
    };
    
    this.rawFkqData = function(raw, pipe) {
        if (pipe.properties.name.toLowerCase() == ape.fkqPipeName) {
            var msg = unescape(raw.data.msg);
            var uin = raw.data.from.properties.uin;
            var tm = Date(eval(raw.time)).match(/\d{1,2}:\d{1,2}:\d{1,2}/)[0];
            hn.fkq.userSaid(ape, uin, msg, tm);
        }
    };
}