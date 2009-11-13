;hn.fkq = {

    __init: function(ape) {
        // fkq: { userbox: {fk_120, fk_222}, msgbox: {msgs...}, msginp, sbmbtn}
        if (!$("#fkq").length) {
            var con = $("<div id='fkq'></div>").appendTo($("#navbar"));
            $("<div id='fkq-usrbox'></div>").appendTo(con);
            $("<div id='fkq-msgbox'></div>").appendTo(con);
            $("<input id='fkq-msginp' type='text' />").appendTo(con);
            var sbmbtn = $("<input id='fkq-sbmbtn' type='button' value='提交' />");
            sbmbtn.appendTo(con);
            sbmbtn.click(function() {
                var msg = $("#fkq-msginp").val();
                if (msg.length) {
                    ape.fkqPipe.request.send("FKQ_SEND", {'msg': msg}, true);
                    var tm = Date().match(/\d{1,2}:\d{1,2}:\d{1,2}/)[0];
                    $("<div>"+ape.uin+"("+tm+"): "+msg+"</div>").appendTo($("#fkq-msgbox"));
                    $("#fkq-msginp").val("");
                }
            });
        }
    },
        
    userVisit: function(ape, uins) {
        hn.fkq.__init(ape);
        
        var usrbox = $("#fkq-usrbox");

        if (typeof uins == "Array") {
            $.each(uins, function(key, uin) {
                $("<div id='fkq-fk_"+uin+"'>" +uin + "</div>").appendTo(usrbox);
            });
        } else {
            $("<div id='fkq-fk_"+uins+"'>"+uins+"</div>").appendTo(usrbox);
        }
    },

    userSaid: function(ape, uin, msg, tm) {
        hn.fkq.__init(ape);
        
        $("<div>"+uin+"("+tm+"): "+msg+"</div>").appendTo($("#fkq-msgbox"));
    },

    userAway: function(uins) {
        if (typeof uins == "Array") {
            $.each(uins, function(key, uin) {
                $("#fkq-fk_"+uin).remove();
            });
        } else {
            $("#fkq-fk_"+uins).remove();
        }
    }
};