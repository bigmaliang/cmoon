;(function($) {
    $.fn.mnscrollpage = function(opts) {
        opts = $.extend({
            trigpos: 'bottom',
            scpage: -1,            // scroll how many pages? -1 forever
            ppage: 0,            // how many page already loaded? I'll load page start this
            callback: function(pg) {console.log(pg);}
        }, opts || {});

        $(this).each(function(i, obj) {
            var
            c = $(obj);
            ppage = opts.ppage,
            dh = th = ch = new Number;
            
            c.scroll(function() {
                if (ppage == opts.scpage) {
                    c.unbind('scroll');
                }

                if (opts.trigpos == 'top') {
                    if (c.scrollTop() == 0) {
                        ppage += 1;
                        opts.callback(ppage);
                    }
                } else {
                    dh = c[0] == document ? c.height(): c.attr('scrollHeight');
                    
                    th = c.scrollTop();
                    ch = c[0] == document ? document.documentElement.clientHeight: c.height();

                    if (dh-th-ch == 0) {
                        ppage += 1;
                        opts.callback(ppage);
                    }
                }
            });
        });
    };
})(jQuery);
