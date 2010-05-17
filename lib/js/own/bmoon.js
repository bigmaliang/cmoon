; var bmoon = {};
bmoon.utl = {
	randomWord: function(n) {
		var baseStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		for (var i = 0, r =""; i < n; i++) r += baseStr.charAt(Math.floor(Math.random() * 62));
		return r;
	},

	//type(101);          // returns 'Number'
	//type('hello');      // returns 'String'
	//type({});           // returns 'Object'
	//type([]);           // returns 'Array'
	//type(function(){}); // returns 'Function'
	//type(new Date());   // returns 'Date'
	//type(document);     // returns 'HTMLDocument'
	//if( type([1,2,3,4,5]) === 'Array' ) { }
	type: function(o) {
		return !!o && Object.prototype.toString.call(o).match(/(\w+)\]/)[1];
	},

	stripHTML: function(string) {
		return string.replace(/&/g,'&amp;').replace(/>/g,'&gt;').replace(/</g,'&lt;').replace(/\"/g,'&quot;');
	},

	getQueryString: function(name) {
		var reg = new RegExp("(^|\\?|&)"+name+"=([^&]*)(\\s|&|$)", "i");
		//if (reg.test(location.href)) return unescape(RegExp.$2.replace(/\+/g, " ")); return "";
		if (reg.test(location.href)) return decodeURIComponent(RegExp.$2.replace(/\+/g, " ")); return "";
	}
};

$.fn.delay = function(time, callback) {
    // Empty function:
    jQuery.fx.step.delay = function(){};
    // Return meaningless animation, (will be added to queue)
    return this.animate({delay:1}, time, callback);
};
