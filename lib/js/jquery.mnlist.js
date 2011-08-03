;(function($) {
    $.fn.mnlist = function(rows, opts) {
        opts = $.extend(
        {
            title: "listtitle",
            time: "1970-01-01",
            titleClickName: "",
            titleClickHref: "",
            titleClickCbk: function() {return true;},
            toggleAble: true,
            content: "listcontent"
        }, opts||{});

        if (type(rows) != "Array") {
            return null;
        }

        var retobj = $("<ul id='mnlist'></ul>");
        retobj.num = 0;

        function makeList(rows)
        {
            $.each(rows, function(i, row)
            {
                var e_item = $("<li></li>").appendTo(retobj);
                var e_title = $("<div class='title'>"+row[opts.title]+"</div>").
                    appendTo(e_item);
                $("<span class='lstTime'> "+row[opts.time]+"</span>").
                    appendTo(e_title);
                if (opts.titleClickName) {
                    if (opts.titleClickHref) {
                        $("<a href='"+opts.titleClickHref+"'>"+
                          opts.titleClickName+"</a>").appendTo(e_title);
                    } else {
                        var ttc = $("<a href='javascript:void(0);'>"+
                                    opts.titleClickName+"</a>").appendTo(e_title);
                        ttc.click(function() {opts.titleClickCbk(row);});
                    }
                }
                if (opts.toggleAble) {
                    $("<a href='javascript:void(0);' class='lstToggle'>&lt;&lt;</a>").
                    click(function()
                    {
                        e_content.toggle("slow");
                    }).appendTo(e_title);
                }
                if (type(opts.content) == "String") {
                    var e_content = $("<li class='content'>"+row[opts.content]+"</li>").
                        appendTo(e_item);
                } else if (type (opts.content) == "Function"){
                    var e_content = opts.content(row, e_item);
                }
                retobj.num++;
            });
        }

        retobj.makeList = makeList;
        makeList(rows);
        return retobj;
     };
})(jQuery);
