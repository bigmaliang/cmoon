;(function($) {
    function tjt_atomadd(tabber, panner) {
        var obj = $("<div><a href='/uiplug/tjt/up.atomadd.html'>货物上架</a></div>");
        obj.appendTo(tabber);
    }
      
    $.fn.uiplugMole = function(name) {
        if (!$("#MOLE_TABS", "#UIPLUG_MOLE").length) {
            $("<div id='MOLE_TABS'></div>").appendTo($("#UIPLUG_MOLE"));
            $("<div id='MOLE_PANES'></div>").appendTo($("#UIPLUG_MOLE"));
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
