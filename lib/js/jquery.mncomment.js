(function($) {
    $.fn.mncomment = function(cmts, opts) {
        opts = $.extend({
            bindTo: document,
            replyEnable: false
        }, opts||{});

        var retobj = $("<div id='comment0'><div id='cmt_content_0'></div></div>").appendTo($(opts.bindTo));
        retobj.num = 0;

        function makeComment(cmts) {
            $.each(cmts, function(i, cmt) {
                var id = parseInt(cmt.id);
                var pid = parseInt(cmt.pid);
                var zid = parseInt(cmt.zid);
                var serialnum = "";
                var title = "<span class='cmtUser'>"+cmt.uname+"</span>";
                var content = cmt.content;
                var intime = cmt.intime;

                var cls = "class='cmtChild'"; if (pid == 0) {
                    cls = "class='cmtLevel0'";
                    serialnum = "<span class='cmtSerial'>"+cmt.sn+"</span>";
                }

                var e_item = $("<div id='comment"+id+"' "+cls+"></div>").appendTo($("#cmt_content_"+pid));

                var e_title = $("<div id='cmt_title_"+id+"' class='cmtTitle'>"+serialnum+title+"</div>").appendTo(e_item);
                $("<span class='cmtTime'>发表于 "+intime+"</span>").appendTo(e_title);
                if (opts.replyEnable) {
                    var rep = $("<a href='#comment_add' class='cmtReply'>[回复]</a>").click(function () {
                        $("#pid").val(id);
                        if (zid == 0) {
                            $("#zid").val(id);
                        } else {
                            $("#zid").val(zid);
                        }
                    }).appendTo(e_title);
                    rep.facebox();
                }
                $("<a href='javascript:void(0);' class='cmtToggle'>&lt;&lt;</a>").click(function() {$("#cmt_content_"+id).toggle("slow");}).appendTo(e_title);

                var e_content = $("<div id='cmt_content_"+id+"' class='cmtContent'><pre>"+content+"</pre></div>").appendTo(e_item);

                retobj.num++;
            });
        }

        retobj.makeComment = makeComment;
        makeComment(cmts);
        return retobj;
    };
})(jQuery);
