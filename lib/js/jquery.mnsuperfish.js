;(function($) {
    $.fn.mnsuperfish = function(opts) {
        opts = $.extend({
            ajax: false,
            url: "",
            data: "",
            actions: []
        }, opts||{});

        function makemenu(actions) {
            var usermenu = $("<ul></ul>");
            $.each(actions, function(i, anchor) {
                $("<li><a href='"+anchor.href+"'>"+anchor.name+"</a></li>").appendTo(usermenu);
            });
            return usermenu;
        }

        $(this).each(function(i, obj) {
            var holderUL = obj;
            if (opts.ajax) {
                $.ajax({
                    type: "GET",
                    url: opts.url,
                    data: opts.data,
                    dataType: opts.dataType,
                    success: function(data) {
                        var actions = data.actions;
                        if (data.success == "1" && type(actions) == "Array") {
                            makemenu(actions).appendTo($("li.current", $(holderUL)));
                            $(holderUL).superfish().fadeIn();
                        } else {
                            alert("获取菜单失败");
                        }
                    },
                    error: function() {
                        alert("获取菜单数据失败");
                    }
                });
            } else if (type(opts.actions) == "Array") {
                makemenu(opts.actions).appendTo($("li.current", $(holderUL)));
                $(holderUL).superfish().fadeIn();
            }
        });
    };
})(jQuery);
