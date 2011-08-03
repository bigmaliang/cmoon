(function($) {
    $.fn.kwtips = function(rows, opts) {
        opts = $.extend({
            showbelow: null,
            tipsdiv: "#dttips",
            rowspec: [0],
            clickRow: function(){return false;}
        }, opts||{});
//        return this.each(function() {
            var tipdiv = $(opts.tipsdiv);
            var pos = $(opts.showbelow).offset();
            tipdiv.empty();
            $("<a class='closetips'><img src='http://pic.x.soso.com/images/research/smart_end.gif' /></a>").appendTo(tipdiv).click(function() {
                tipdiv.hide();
            });
            $.each(rows, function(i, row) {
                var item = $("<a class='tipsrow'></a>").appendTo(tipdiv).click(function(evt) {
                    tipdiv.hide();
                    opts.clickRow(evt, row);
                });
                $.each(opts.rowspec, function(i, col) {
                    $("<span class='tipscol"+i+"'>"+row[col]+"</span>").appendTo(item);
                });
            });
            tipdiv.css({
                display: "block",
                left: pos.left + "px",
                top: (pos.top + $(opts.showbelow).outerHeight()) + "px"
            });
//        });
    };
})(jQuery);
