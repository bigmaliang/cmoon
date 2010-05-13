;(function($) {
    function atom_firsttab(tabber, panner) {
        var obj = $("<li class='folder'><a href='/null.html'>管理菜单</a></li>");
        obj.appendTo(tabber);
    }
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
            var tabs = $("<ul id='MOLE_TABS' class='tabs'></ul>").appendTo($("#UIPLUG_MOLE"));
            var panes = $("<div id='MOLE_PANES' class='panes'><div class='pane'></div></div>").appendTo($("#UIPLUG_MOLE"));
            atom_firsttab(tabs, panes);
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
