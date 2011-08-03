;(function($){
    function assemUrl(url, page) {
        var regu = /\?/;
        if(regu.test(url)) {
            return url + "&pg=" + page;
        } else {
            return url + "?pg=" + page;
        }
    }
    function pagePrev(url, callback, currpage, wrapper) {
        if (url) {
            var furl = assemUrl(url, currpage-1);
            $("<a href='"+furl+"'>上一页</a>").appendTo(wrapper);
        } else {
            var obj = $("<a href='javascript:void(0);'>上一页</a>").appendTo(wrapper);
            obj.click(function() {
                document.pagenav_pg = currpage-1;
                callback(currpage-1);
            });
        }
    }
    function pageNext(url, callback, currpage, wrapper) {
        if (url) {
            var furl = assemUrl(url, currpage+1);
            $("<a href='"+furl+"'>下一页</a>").appendTo(wrapper);
        } else {
            var obj = $("<a href='javascript:void(0);'>下一页</a>").appendTo(wrapper);
            obj.click(function() {
                document.pagenav_pg = currpage+1;
                callback(currpage+1);
            });
        }
    }
    function pageJump(url, callback, currpage, totalpage, wrapper) {
        $("<span>转到第 <input type='text' id='pagenav_goto' class='short bordless' /> 页</span>").appendTo(wrapper);
        var obj = $("<span><a href='javascript:void(0);'>确定</a></span>");
        obj.appendTo(wrapper);
        obj.click(function()
        {
            var pg = $("#pagenav_goto").val();
            var regd = /^[0-9]+$/;
            if (!regd.test(pg)) {
                $("#pagenav_goto").addClass("invalid");
                $("#pagenav_goto").removeClass("valie");
                return;
            }
            if (pg < 1 || pg > totalpage) {
                $("#pagenav_goto").addClass("invalid");
                $("#pagenav_goto").removeClass("valie");
                return;
            }
            $("#pagenav_goto").addClass("valid");
            $("#pagenav_goto").removeClass("invalie");
            if (url) {
                var furl = assemUrl(url, pg);
                self.location.href = furl;
            } else {
                document.pagenav_pg = pg;
                callback(pg);
            }
        });
    }

    $.fn.mnpagenav = function(opts) {
        opts = $.extend({
            numperpage: 15,
            pg: 1,
            ttnum: 0,
            url: "",            // (url) means no ajax mode, need offer pg meanwhile
            callback: function(pg) {return true;}
        }, opts||{});

        var currpage = 1;
        if (typeof document.pagenav_pg != "undefined") {
            currpage = parseInt(document.pagenav_pg);
        } else {
            currpage = parseInt(opts.pg);
        }
        var tmppg = parseInt(opts.ttnum)+parseInt(opts.numperpage)-1;
        var totalpage = parseInt(tmppg / opts.numperpage);

        $(this).each(function(i, obj)
        {
            var wrapper = obj;
            $(wrapper).empty();
            //$("<span>共 "+totalpage+" 页</span>").appendTo(wrapper);
            if (currpage != 1)
                pagePrev(opts.url, opts.callback, currpage, wrapper);

            $("<span>第 "+currpage+"/"+totalpage+" 页</span>").appendTo(wrapper);

            if (currpage != totalpage && totalpage != 0)
                pageNext(opts.url, opts.callback, currpage, wrapper);

            pageJump(opts.url, opts.callback, currpage, totalpage, wrapper);
        });
    };
})(jQuery);
