;var mgd = mgd || {}, bmoon = bmoon || {};
bmoon.utl = {
	exetime: 0,
	
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

    reload: function(url) {
        var href = (url && url.length > 0) ? url: location.href;
        location.href = href;
    },

    url: location.href,

    urlclean: location.href.match(/^[^\?]*/)[0],

    urlparam: location.href.match(/\?.*$/) ? location.href.match(/\?.*$/)[0] : '',

    refer: document.referrer,
    
    title: $('head title').html(),

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
		//return string.replace(/&/g,'&amp;').replace(/>/g,'&gt;').replace(/</g,'&lt;').replace(/\"/g,'&quot;');
        return string.replace(/>/g,'&gt;').replace(/</g,'&lt;').replace(/\"/g,'&quot;');
	},

    clotheHTML: function(input) {
        var e = document.createElement('div');
        e.innerHTML = input;
        return e.childNodes.length === 0 ? "" : e.childNodes[0].nodeValue;
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
	},

    // ----------------------------------------------------------
    // If you're not in IE (or IE version is less than 5) then:
    //     ie === undefined
    // If you're in IE (>5) then you can determine which version:
    //     ie === 7; // IE7
    // Thus, to detect IE:
    //     if (ie) {}
    // And to detect the version:
    //     ie === 6 // IE6
    //     ie> 7 // IE8, IE9 ...
    //     ie <9 // Anything less than IE9
    // ----------------------------------------------------------
    // http://james.padolsey.com/javascript/detect-ie-in-js-using-conditional-comments/
	ie: function() {
        var
        undef,
        v = 3,
        div = document.createElement('div');

        while(
            div.innerHTML = '<!--[if gt IE '+ (++v) +']><i></i><![endif]-->',
            div.getElementsByTagName('i')[0]
        );
        return v>4 ? v : undef;
	},

	after: function(func, until, sec) {
		// avoid system resource leak
		sec = sec || 10;
		if (parseInt(bmoon.utl.exetime * (100 / 1000)) > sec) {
			console.log(until + 'can not satisfied after ' + sec + ' seconds pasted');
			bmoon.utl.exetime = 0;
			return;
		}
		
		// through exp: we can't use if (until !== undefined), seems js will cache variable's val
		if (eval(until)) {
			bmoon.utl.exetime = 0;
			func();
		} else {
			bmoon.utl.exetime++;
			setTimeout(function() {
				bmoon.utl.after(func, until, sec);
			}, 100);
		}
	},

    objToString: function(o) {
        return JSON.stringify(o);
    },

    stringToObj: function(s) {
        var o = {};
        
        try {
            o = $.parseJSON(s);
        } catch (err) {
            return null;
        }
        
        return o;
    },

    loadJS: function(url) {
        var e = document.createElement("script");
        e.src = url;
        e.type="text/javascript";
        document.getElementsByTagName("head")[0].appendChild(e);
    },

    earthDis: function(pointa, pointb) {
        var R = 6371,
        dc = 1.852,
        //dc[1] = 1.852,              // km
        //dc[2] = 185200.0/160934.40, // 1.150779448 sm
        //dc[3] = 185200.0/30.48, // 6076.11549 ft
        
        lat1 = (Math.PI/180)*pointa[0],
        lat2 = (Math.PI/180)*pointb[0],
        lon1 = (Math.PI/180)*pointa[1],
        lon2 = (Math.PI/180)*pointb[1];

        with (Math) {
            var cd = acos(sin(lat1)*sin(lat2)+cos(lat1)*cos(lat2)*cos(lon1-lon2));
        }
        
        //return Math.round( cd*(180/Math.PI)*60*dc );
        return (cd*(180/Math.PI)*60*dc).toFixed(2);
    }
};
