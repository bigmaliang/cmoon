;(function($) {
    function tjt_atomadd(tabber, panner) {
        var obj = $("<li><a href='/uiplug/tjt/up.atomadd.html'>货物上架</a></li>");
        obj.appendTo(tabber);
    }
    function tjt_dcadmin(tabber, panner) {
        var obj = $("<li><a href='/uiplug/tjt/up.dcadmin.html'>送货点管理</a></li>");
        obj.appendTo(tabber);
    }

    $.fn.uiplugMole = function(name) {
        if (!$("#MOLE_TABS", "#UIPLUG_MOLE").length) {
            $("<ul id='MOLE_TABS' class='tabs'></ul>").appendTo($("#UIPLUG_MOLE"));
            $("<div id='MOLE_PANES' class='panes'><div style='display:block'></div></div>").appendTo($("#UIPLUG_MOLE"));
        }
        var tabber = $("#MOLE_TABS", "#UIPLUG_MOLE");
        var panner = $("#MOLE_PANES", "UIPLUG_MOLE");

        if (type(name) == "String") {
            var fun = eval(name);
            if (type(fun) == "Function") {
                fun(tabber, panner);
            }
        }
    };

    $.fn.uiplugAtom = function(name) {
    };
})(jQuery);
