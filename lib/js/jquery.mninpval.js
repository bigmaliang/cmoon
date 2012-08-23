;(function($) {
     $.fn.inputval = function() {
         var ret = true;

         function reportErr(inp, errmsg) {
             if (errmsg) {
                 alert(errmsg);
                 return false;  // prevent form submit
             }
             inp.removeClass("valid");
             inp.mtoggleClass({
                 classname: 'invalid',
                 leaveClass: true,
                 interval: 500,
                 time: 3
             });
         }
         
         $(this).each(function(i, obj)
         {
             var inp = $(obj);
             var msg = inp.attr("verrmsg");
             //var msg = inp.attr("verrmsg") ? inp.attr("verrmsg"): "输入有误";
             var val = inp.val();
             
             if (inp.hasClass('vrequire')) {
                 if (val.length <= 0) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             } else {
                 if (val.length == 0) {
                     inp.removeClass("invalid");
                     inp.addClass("valid");
                     return true;
                 }
             }

             if (inp.attr('vmax') != undefined) {
                 if (val.length > inp.attr('vmax')) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.attr('vmin') != undefined) {
                 if (val.length < inp.attr('vmin')) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vdate')) {
                 var Regex = /^([\d]|1[0,1,2]|0[1-9])(\-|\/|\.)([0-9]|[0,1,2][0-9]|3[0,1])(\-|\/|\.)\d{4}$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vemail')) {
                 var Regex =/^([a-zA-Z0-9_\.\-\+])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vint')) {
                 var Regex = /^\d{1,10}$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vphone')) {
                 //var Regex = /^\(?[2-9]\d{2}[ \-\)] ?\d{3}[\- ]?\d{4}$/;
                 var Regex = /^[0-9\-]+$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vname')) {
                 var Regex = /^[a-zA-Z0-9\.\-']*$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vurl')) {
                 var Regex = /^http:\/\/.*$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             if (inp.hasClass('vip')) {
                 var Regex = /^(\d{1,3}\.){3}\d{1,3}$/;
                 if (!Regex.test(val)) {
                     reportErr(inp, msg);
                     ret = false;
                     return false;
                 }
             }

             inp.removeClass("invalid");
             inp.addClass("valid");
             return true;
         });
         
         return ret;
     };
})(jQuery);
