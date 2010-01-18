;hn.fkq = {
    kwds: ["DOM", "TEXT", "ATTR", "EVENT"],
    els: {
        efun: {
            EVENT: {mouseover: function(ape, opts) {
                        var tg = function() {
                            $("#efun").addClass("hover");
                        };
                        ape.hnTimeoutTG = setTimeout(tg, 500);
                    },
                    mouseout: function(ape, opts) {
                        $("#efun").removeClass("hover");
                        clearTimeout(ape.hnTimeoutTG);
                    }
            },
            efun_: {
                ATTR: {"class": "efun"},
                efun_tg_: {
                    TEXT: "访客群",
                    ATTR: {"class": "efun_tg"},
                    EVENT: {click: "hn.fkq.evtToggleIns"}
                },
                efun_tip_: 1
            }
        },
        eins: {
            eins_: {
                ATTR: {"class": "eins"},
                eins_msgbox_: {
                    ATTR: {"class": "eins_msgbox"}
                },
                eins_usrbox_: {
                    ATTR: {"class": "eins_usrbox"},
                    eins_usrbox_usr_: 1
                },
                eins_msginp_: {
                    DOM: "<input type='text' />",
                    ATTR: {"class": "eins_msginp"},
                    EVENT: {keyup: function(ape, opts, e) {
                                if(e.which == 13) {
                                    hn.fkq.evtSendMsg(ape, opts);
                                }
                            }
                    }
                },
                eins_msgsnd_: {
                    DOM: "<input type='button' value='提交' />",
                    ATTR: {"class": "eins_msgsnd"},
                    EVENT: {click: "hn.fkq.evtSendMsg"}
                },
                eins_talkto_: 1
            }
        }
    },
    
    evtSendMsg: function(ape, opts) {
        var msginp = $("#eins_msginp_"+opts.fill);
        var msgbox = $("#eins_msgbox_"+opts.fill);
        
        var msg = msginp.val();
        if (msg.length) {
            ape.fkqPipe.request.send("FKQ_SEND", {'msg': msg}, true);
            var tm = Date().match(/\d{1,2}:\d{1,2}:\d{1,2}/)[0];
            $("<div>"+ape.uin+"("+tm+"): "+msg+"</div>").appendTo(msgbox);
            msginp.val("");
        }
    },

    evtToggleIns: function(ape, opts) {
        $("#eins_"+opts.fill).toggle();
        if (ape.hnIntervalNewmsg) {
            clearInterval(ape.hnIntervalNewmsg);
        }
    },

    __init: function(ape) {
        if (!$("#fkq").length) {
            var con = $("<div id='fkq'></div>").appendTo($("#navbar"));
            hn.fkq.fkqUiCreate(hn.fkq.els, con, ape);
        }
    },

    // obj: UI spec with obj format, pls refer hn.fkq.els
    fkqUiCreate: function(obj, parent, ape, opts) {
        var createDOM = function (id, spec, ape, opts) {
            if (typeof (id) != "string") {
                return null;
            }
            var o = $("#"+id);
            if (!o.length) {
                var dom = "<div></div>";
                if (spec.DOM) {
                    dom = spec.DOM;
                }
                o = $(dom).attr("id", id);

                if (spec.TEXT) {
                    o.text(spec.TEXT);
                }

                if (spec.ATTR) {
                    $.each(spec.ATTR, function(key, val){
                        o.attr(key, val);
                    });
                }
                
                if (spec.EVENT) {
                    $.each(spec.EVENT, function(key, val){
                        o.bind(key, function(e){
                            var fun = val;
                            if (typeof(fun) != "function") {
                                fun = eval(val);
                            }
                            if (typeof(fun) == "function") {
                                fun(ape, opts, e);
                            }
                        });
                    });
                }
            }
            return o;
        };

        var p = parent;
        if (typeof (parent) == "string") {
            p = $("#"+parent);
            if (!p.length) {
                return;
            }
        }

        opts = $.extend({
            fill: "",
            text: ""
        }, opts||{});
        
        $.each(obj, function(key, val) {
            for (kwd in hn.fkq.kwds) {
                if (key == hn.fkq.kwds[kwd]) {
                    return true;
                }
            }
            if (opts.fill && typeof (opts.fill) == "string") {
                key = key+opts.fill;
            }
            var o = createDOM(key, val, ape, opts);
            o.appendTo(p);
            if (opts.text && typeof (opts.text) == "string") {
                o.text(opts.text);
            } else if (opts.text && typeof (opts.text) == "function") {
                o.text(opts.text());
            }
        });
    },
        
    userVisit: function(ape, uins) {
        var f = hn.fkq;
        var e = f.els;
        f.__init(ape);

        f.fkqUiCreate(e.efun, "efun", ape, {fill:"a"});
        f.fkqUiCreate(e.eins, "eins", ape, {fill:"a"});
        f.fkqUiCreate(e.efun.efun_, "efun_a", ape, {fill:"a"});
        f.fkqUiCreate(e.eins.eins_, "eins_a", ape, {fill:"a"});
        
        if (typeof (uins) == "object") {
            $.each(uins, function(key, uin) {
                f.fkqUiCreate(e.eins.eins_.eins_usrbox_, "eins_usrbox_a", ape,
                {fill: "a_"+uin, text: uin});
            });
        } else if (typeof (uins) == "string") {
            f.fkqUiCreate(e.eins.eins_.eins_usrbox_, "eins_usrbox_a", ape,
            {fill: "a_"+uins, text: uins});
        }
    },

    userSaid: function(ape, uin, msg, tm) {
        var f = hn.fkq;
        var e = f.els;
        f.__init(ape);
        
        $("<div>"+uin+"("+tm+"): "+msg+"</div>").appendTo($("#eins_msgbox_a"));
        var disp = $("#eins_a").css("display");
        if (disp == "none") {
            if (!ape.hnIntervalNewmsg) {
                ape.hnIntervalNewmsg = setInterval(function() {
                                                       $("#efun_a").toggle();
                                                   }, 500);
            }
        }
    },

    userAway: function(uins) {
        if (typeof uins == "object") {
            $.each(uins, function(key, uin) {
                $("#eins_usrbox_usr_a_"+uin).remove();
            });
        } else {
            $("#eins_usrbox_usr_a_"+uins).remove();
        }
    }
};