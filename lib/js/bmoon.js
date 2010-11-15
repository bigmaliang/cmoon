;var mgd = mgd || {}, bmoon = bmoon || {};
bmoon.utl = {
	randomWord: function(n) {
		var baseStr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
		for (var i = 0, r =""; i < n; i++) r += baseStr.charAt(Math.floor(Math.random() * 62));
		return r;
	},

	randomName: function() {
		var
		S = "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		//s = "abcdefghijklmnopqrstuvwxyz",
		s = "abcdefghjkmnpqrstuvwxyz",
		n = "0123456789",
		r = s.charAt(Math.floor(Math.random() * 23));

		r += s.charAt(Math.floor(Math.random() * 23)) + '_';
		//r += n.charAt(Math.floor(Math.random() * 10));

		for (var i = 0; i < 5; i++) r += S.charAt(Math.floor(Math.random() * 26));

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
	},

	// http://stackoverflow.com/questions/698301/is-there-a-native-jquery-function-to-switch-elements
	// http://www.doxdesk.com/
	// BUGFULL
	swapNodes: function(a, b) {
		var aparent = a.parent();
		var asibling = a.next() === b? a : a.next();
		b.parent().append(a);
		aparent.append(b);
	}
};

$.fn.delay = function(time, callback) {
    // Empty function:
    jQuery.fx.step.delay = function(){};
    // Return meaningless animation, (will be added to queue)
    return this.animate({delay:1}, time, callback);
};
