
    delay: function(time, callback) {
        // Empty function:
        jQuery.fx.step.delay = function(){};
        // Return meaningless animation, (will be added to queue)
        return this.animate({delay:1}, time, callback);
    },


function getCookie(name)
{
	var arr = document.cookie.match(new RegExp("(^| )"+name+"=([^;]*)(;|$)"));
	if(arr != null)
		return decodeURIComponent(arr[2]);
	return null;
}
function delCookie(name)
{
	var exp = new Date();
	exp.setTime(exp.getTime() - 1);
	var cval=getCookie(name);
	if(cval!=null) document.cookie = name + "="+cval+";expires="+exp.toGMTString();
}

