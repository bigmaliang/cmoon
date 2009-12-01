/*
Script: Core.js
	MooTools - My Object Oriented JavaScript Tools.

License:
	MIT-style license.

Copyright:
	Copyright (c) 2006-2008 [Valerio Proietti](http://mad4milk.net/).

Code & Documentation:
	[The MooTools production team](http://mootools.net/developers/).

Inspiration:
	- Class implementation inspired by [Base.js](http://dean.edwards.name/weblog/2006/03/base/) Copyright (c) 2006 Dean Edwards, [GNU Lesser General Public License](http://opensource.org/licenses/lgpl-license.php)
	- Some functionality inspired by [Prototype.js](http://prototypejs.org) Copyright (c) 2005-2007 Sam Stephenson, [MIT License](http://opensource.org/licenses/mit-license.php)
*/

var MooTools = {
	'version': '1.2.2',
	'build': 'f0491d62fbb7e906789aa3733d6a67d43e5af7c9'
};

var Native = function(options){
	options = options || {};
	var name = options.name;
	var legacy = options.legacy;
	var protect = options.protect;
	var methods = options.implement;
	var generics = options.generics;
	var initialize = options.initialize;
	var afterImplement = options.afterImplement || function(){};
	var object = initialize || legacy;
	generics = generics !== false;

	object.constructor = Native;
	object.$family = {name: 'native'};
	if (legacy && initialize) object.prototype = legacy.prototype;
	object.prototype.constructor = object;

	if (name){
		var family = name.toLowerCase();
		object.prototype.$family = {name: family};
		Native.typize(object, family);
	}

	var add = function(obj, name, method, force){
		if (!protect || force || !obj.prototype[name]) obj.prototype[name] = method;
		if (generics) Native.genericize(obj, name, protect);
		afterImplement.call(obj, name, method);
		return obj;
	};

	object.alias = function(a1, a2, a3){
		if (typeof a1 == 'string'){
			if ((a1 = this.prototype[a1])) return add(this, a2, a1, a3);
		}
		for (var a in a1) this.alias(a, a1[a], a2);
		return this;
	};

	object.implement = function(a1, a2, a3){
		if (typeof a1 == 'string') return add(this, a1, a2, a3);
		for (var p in a1) add(this, p, a1[p], a2);
		return this;
	};

	if (methods) object.implement(methods);

	return object;
};

Native.genericize = function(object, property, check){
	if ((!check || !object[property]) && typeof object.prototype[property] == 'function') object[property] = function(){
		var args = Array.prototype.slice.call(arguments);
		return object.prototype[property].apply(args.shift(), args);
	};
};

Native.implement = function(objects, properties){
	for (var i = 0, l = objects.length; i < l; i++) objects[i].implement(properties);
};

Native.typize = function(object, family){
	if (!object.type) object.type = function(item){
		return ($type(item) === family);
	};
};

(function(){
	var natives = {'Array': Array, 'Date': Date, 'Function': Function, 'Number': Number, 'RegExp': RegExp, 'String': String};
	for (var n in natives) new Native({name: n, initialize: natives[n], protect: true});

	var types = {'boolean': Boolean, 'native': Native, 'object': Object};
	for (var t in types) Native.typize(types[t], t);

	var generics = {
		'Array': ["concat", "indexOf", "join", "lastIndexOf", "pop", "push", "reverse", "shift", "slice", "sort", "splice", "toString", "unshift", "valueOf"],
		'String': ["charAt", "charCodeAt", "concat", "indexOf", "lastIndexOf", "match", "replace", "search", "slice", "split", "substr", "substring", "toLowerCase", "toUpperCase", "valueOf"]
	};
	for (var g in generics){
		for (var i = generics[g].length; i--;) Native.genericize(window[g], generics[g][i], true);
	}
})();

var Hash = new Native({

	name: 'Hash',

	initialize: function(object){
		if ($type(object) == 'hash') object = $unlink(object.getClean());
		for (var key in object) this[key] = object[key];
		return this;
	}

});

Hash.implement({

	forEach: function(fn, bind){
		for (var key in this){
			if (this.hasOwnProperty(key)) fn.call(bind, this[key], key, this);
		}
	},

	getClean: function(){
		var clean = {};
		for (var key in this){
			if (this.hasOwnProperty(key)) clean[key] = this[key];
		}
		return clean;
	},

	getLength: function(){
		var length = 0;
		for (var key in this){
			if (this.hasOwnProperty(key)) length++;
		}
		return length;
	}

});

Hash.alias('forEach', 'each');

Array.implement({

	forEach: function(fn, bind){
		for (var i = 0, l = this.length; i < l; i++) fn.call(bind, this[i], i, this);
	}

});

Array.alias('forEach', 'each');

function $A(iterable){
	if (iterable.item){
		var l = iterable.length, array = new Array(l);
		while (l--) array[l] = iterable[l];
		return array;
	}
	return Array.prototype.slice.call(iterable);
};

function $arguments(i){
	return function(){
		return arguments[i];
	};
};

function $chk(obj){
	return !!(obj || obj === 0);
};

function $clear(timer){
	clearTimeout(timer);
	clearInterval(timer);
	return null;
};

function $defined(obj){
	return (obj != undefined);
};

function $each(iterable, fn, bind){
	var type = $type(iterable);
	((type == 'arguments' || type == 'collection' || type == 'array') ? Array : Hash).each(iterable, fn, bind);
};

function $empty(){};

function $extend(original, extended){
	for (var key in (extended || {})) original[key] = extended[key];
	return original;
};

function $H(object){
	return new Hash(object);
};

function $lambda(value){
	return (typeof value == 'function') ? value : function(){
		return value;
	};
};

function $merge(){
	var args = Array.slice(arguments);
	args.unshift({});
	return $mixin.apply(null, args);
};

function $mixin(mix){
	for (var i = 1, l = arguments.length; i < l; i++){
		var object = arguments[i];
		if ($type(object) != 'object') continue;
		for (var key in object){
			var op = object[key], mp = mix[key];
			mix[key] = (mp && $type(op) == 'object' && $type(mp) == 'object') ? $mixin(mp, op) : $unlink(op);
		}
	}
	return mix;
};

function $pick(){
	for (var i = 0, l = arguments.length; i < l; i++){
		if (arguments[i] != undefined) return arguments[i];
	}
	return null;
};

function $random(min, max){
	return Math.floor(Math.random() * (max - min + 1) + min);
};

function $splat(obj){
	var type = $type(obj);
	return (type) ? ((type != 'array' && type != 'arguments') ? [obj] : obj) : [];
};

var $time = Date.now || function(){
	return +new Date;
};

function $try(){
	for (var i = 0, l = arguments.length; i < l; i++){
		try {
			return arguments[i]();
		} catch(e){}
	}
	return null;
};

function $type(obj){
	if (obj == undefined) return false;
	if (obj.$family) return (obj.$family.name == 'number' && !isFinite(obj)) ? false : obj.$family.name;
	if (obj.nodeName){
		switch (obj.nodeType){
			case 1: return 'element';
			case 3: return (/\S/).test(obj.nodeValue) ? 'textnode' : 'whitespace';
		}
	} else if (typeof obj.length == 'number'){
		if (obj.callee) return 'arguments';
		else if (obj.item) return 'collection';
	}
	return typeof obj;
};

function $unlink(object){
	var unlinked;
	switch ($type(object)){
		case 'object':
			unlinked = {};
			for (var p in object) unlinked[p] = $unlink(object[p]);
		break;
		case 'hash':
			unlinked = new Hash(object);
		break;
		case 'array':
			unlinked = [];
			for (var i = 0, l = object.length; i < l; i++) unlinked[i] = $unlink(object[i]);
		break;
		default: return object;
	}
	return unlinked;
};


/*
Script: Browser.js
	The Browser Core. Contains Browser initialization, Window and Document, and the Browser Hash.

License:
	MIT-style license.
*/

var Browser = $merge({

	Engine: {name: 'unknown', version: 0},

	Platform: {name: (window.orientation != undefined) ? 'ipod' : (navigator.platform.match(/mac|win|linux/i) || ['other'])[0].toLowerCase()},

	Features: {xpath: !!(document.evaluate), air: !!(window.runtime), query: !!(document.querySelector)},

	Plugins: {},

	Engines: {

		presto: function(){
			return (!window.opera) ? false : ((arguments.callee.caller) ? 960 : ((document.getElementsByClassName) ? 950 : 925));
		},

		trident: function(){
			return (!window.ActiveXObject) ? false : ((window.XMLHttpRequest) ? 5 : 4);
		},

		webkit: function(){
			return (navigator.taintEnabled) ? false : ((Browser.Features.xpath) ? ((Browser.Features.query) ? 525 : 420) : 419);
		},

		gecko: function(){
			return (document.getBoxObjectFor == undefined) ? false : ((document.getElementsByClassName) ? 19 : 18);
		}

	}

}, Browser || {});

Browser.Platform[Browser.Platform.name] = true;

Browser.detect = function(){

	for (var engine in this.Engines){
		var version = this.Engines[engine]();
		if (version){
			this.Engine = {name: engine, version: version};
			this.Engine[engine] = this.Engine[engine + version] = true;
			break;
		}
	}

	return {name: engine, version: version};

};

Browser.detect();

Browser.Request = function(){
	return $try(function(){
		return new XMLHttpRequest();
	}, function(){
		return new ActiveXObject('MSXML2.XMLHTTP');
	});
};

Browser.Features.xhr = !!(Browser.Request());

Browser.Plugins.Flash = (function(){
	var version = ($try(function(){
		return navigator.plugins['Shockwave Flash'].description;
	}, function(){
		return new ActiveXObject('ShockwaveFlash.ShockwaveFlash').GetVariable('$version');
	}) || '0 r0').match(/\d+/g);
	return {version: parseInt(version[0] || 0 + '.' + version[1], 10) || 0, build: parseInt(version[2], 10) || 0};
})();

function $exec(text){
	if (!text) return text;
	if (window.execScript){
		window.execScript(text);
	} else {
		var script = document.createElement('script');
		script.setAttribute('type', 'text/javascript');
		script[(Browser.Engine.webkit && Browser.Engine.version < 420) ? 'innerText' : 'text'] = text;
		document.head.appendChild(script);
		document.head.removeChild(script);
	}
	return text;
};

Native.UID = 1;

var $uid = (Browser.Engine.trident) ? function(item){
	return (item.uid || (item.uid = [Native.UID++]))[0];
} : function(item){
	return item.uid || (item.uid = Native.UID++);
};

var Window = new Native({

	name: 'Window',

	legacy: (Browser.Engine.trident) ? null: window.Window,

	initialize: function(win){
		$uid(win);
		if (!win.Element){
			win.Element = $empty;
			if (Browser.Engine.webkit) win.document.createElement("iframe"); //fixes safari 2
			win.Element.prototype = (Browser.Engine.webkit) ? window["[[DOMElement.prototype]]"] : {};
		}
		win.document.window = win;
		return $extend(win, Window.Prototype);
	},

	afterImplement: function(property, value){
		window[property] = Window.Prototype[property] = value;
	}

});

Window.Prototype = {$family: {name: 'window'}};

new Window(window);

var Document = new Native({

	name: 'Document',

	legacy: (Browser.Engine.trident) ? null: window.Document,

	initialize: function(doc){
		$uid(doc);
		doc.head = doc.getElementsByTagName('head')[0];
		doc.html = doc.getElementsByTagName('html')[0];
		if (Browser.Engine.trident && Browser.Engine.version <= 4) $try(function(){
			doc.execCommand("BackgroundImageCache", false, true);
		});
		if (Browser.Engine.trident) doc.window.attachEvent('onunload', function() {
			doc.window.detachEvent('onunload', arguments.callee);
			doc.head = doc.html = doc.window = null;
		});
		return $extend(doc, Document.Prototype);
	},

	afterImplement: function(property, value){
		document[property] = Document.Prototype[property] = value;
	}

});

Document.Prototype = {$family: {name: 'document'}};

new Document(document);


/*
Script: Array.js
	Contains Array Prototypes like each, contains, and erase.

License:
	MIT-style license.
*/

Array.implement({

	every: function(fn, bind){
		for (var i = 0, l = this.length; i < l; i++){
			if (!fn.call(bind, this[i], i, this)) return false;
		}
		return true;
	},

	filter: function(fn, bind){
		var results = [];
		for (var i = 0, l = this.length; i < l; i++){
			if (fn.call(bind, this[i], i, this)) results.push(this[i]);
		}
		return results;
	},

	clean: function() {
		return this.filter($defined);
	},

	indexOf: function(item, from){
		var len = this.length;
		for (var i = (from < 0) ? Math.max(0, len + from) : from || 0; i < len; i++){
			if (this[i] === item) return i;
		}
		return -1;
	},

	map: function(fn, bind){
		var results = [];
		for (var i = 0, l = this.length; i < l; i++) results[i] = fn.call(bind, this[i], i, this);
		return results;
	},

	some: function(fn, bind){
		for (var i = 0, l = this.length; i < l; i++){
			if (fn.call(bind, this[i], i, this)) return true;
		}
		return false;
	},

	associate: function(keys){
		var obj = {}, length = Math.min(this.length, keys.length);
		for (var i = 0; i < length; i++) obj[keys[i]] = this[i];
		return obj;
	},

	link: function(object){
		var result = {};
		for (var i = 0, l = this.length; i < l; i++){
			for (var key in object){
				if (object[key](this[i])){
					result[key] = this[i];
					delete object[key];
					break;
				}
			}
		}
		return result;
	},

	contains: function(item, from){
		return this.indexOf(item, from) != -1;
	},

	extend: function(array){
		for (var i = 0, j = array.length; i < j; i++) this.push(array[i]);
		return this;
	},
	
	getLast: function(){
		return (this.length) ? this[this.length - 1] : null;
	},

	getRandom: function(){
		return (this.length) ? this[$random(0, this.length - 1)] : null;
	},

	include: function(item){
		if (!this.contains(item)) this.push(item);
		return this;
	},

	combine: function(array){
		for (var i = 0, l = array.length; i < l; i++) this.include(array[i]);
		return this;
	},

	erase: function(item){
		for (var i = this.length; i--; i){
			if (this[i] === item) this.splice(i, 1);
		}
		return this;
	},

	empty: function(){
		this.length = 0;
		return this;
	},

	flatten: function(){
		var array = [];
		for (var i = 0, l = this.length; i < l; i++){
			var type = $type(this[i]);
			if (!type) continue;
			array = array.concat((type == 'array' || type == 'collection' || type == 'arguments') ? Array.flatten(this[i]) : this[i]);
		}
		return array;
	},

	hexToRgb: function(array){
		if (this.length != 3) return null;
		var rgb = this.map(function(value){
			if (value.length == 1) value += value;
			return value.toInt(16);
		});
		return (array) ? rgb : 'rgb(' + rgb + ')';
	},

	rgbToHex: function(array){
		if (this.length < 3) return null;
		if (this.length == 4 && this[3] == 0 && !array) return 'transparent';
		var hex = [];
		for (var i = 0; i < 3; i++){
			var bit = (this[i] - 0).toString(16);
			hex.push((bit.length == 1) ? '0' + bit : bit);
		}
		return (array) ? hex : '#' + hex.join('');
	}

});


/*
Script: Function.js
	Contains Function Prototypes like create, bind, pass, and delay.

License:
	MIT-style license.
*/

Function.implement({

	extend: function(properties){
		for (var property in properties) this[property] = properties[property];
		return this;
	},

	create: function(options){
		var self = this;
		options = options || {};
		return function(event){
			var args = options.arguments;
			args = (args != undefined) ? $splat(args) : Array.slice(arguments, (options.event) ? 1 : 0);
			if (options.event) args = [event || window.event].extend(args);
			var returns = function(){
				return self.apply(options.bind || null, args);
			};
			if (options.delay) return setTimeout(returns, options.delay);
			if (options.periodical) return setInterval(returns, options.periodical);
			if (options.attempt) return $try(returns);
			return returns();
		};
	},

	run: function(args, bind){
		return this.apply(bind, $splat(args));
	},

	pass: function(args, bind){
		return this.create({bind: bind, arguments: args});
	},

	bind: function(bind, args){
		return this.create({bind: bind, arguments: args});
	},

	bindWithEvent: function(bind, args){
		return this.create({bind: bind, arguments: args, event: true});
	},

	attempt: function(args, bind){
		return this.create({bind: bind, arguments: args, attempt: true})();
	},

	delay: function(delay, bind, args){
		return this.create({bind: bind, arguments: args, delay: delay})();
	},

	periodical: function(periodical, bind, args){
		return this.create({bind: bind, arguments: args, periodical: periodical})();
	}

});


/*
Script: Number.js
	Contains Number Prototypes like limit, round, times, and ceil.

License:
	MIT-style license.
*/

Number.implement({

	limit: function(min, max){
		return Math.min(max, Math.max(min, this));
	},

	round: function(precision){
		precision = Math.pow(10, precision || 0);
		return Math.round(this * precision) / precision;
	},

	times: function(fn, bind){
		for (var i = 0; i < this; i++) fn.call(bind, i, this);
	},

	toFloat: function(){
		return parseFloat(this);
	},

	toInt: function(base){
		return parseInt(this, base || 10);
	}

});

Number.alias('times', 'each');

(function(math){
	var methods = {};
	math.each(function(name){
		if (!Number[name]) methods[name] = function(){
			return Math[name].apply(null, [this].concat($A(arguments)));
		};
	});
	Number.implement(methods);
})(['abs', 'acos', 'asin', 'atan', 'atan2', 'ceil', 'cos', 'exp', 'floor', 'log', 'max', 'min', 'pow', 'sin', 'sqrt', 'tan']);


/*
Script: String.js
	Contains String Prototypes like camelCase, capitalize, test, and toInt.

License:
	MIT-style license.
*/

String.implement({

	test: function(regex, params){
		return ((typeof regex == 'string') ? new RegExp(regex, params) : regex).test(this);
	},

	contains: function(string, separator){
		return (separator) ? (separator + this + separator).indexOf(separator + string + separator) > -1 : this.indexOf(string) > -1;
	},

	trim: function(){
		return this.replace(/^\s+|\s+$/g, '');
	},

	clean: function(){
		return this.replace(/\s+/g, ' ').trim();
	},

	camelCase: function(){
		return this.replace(/-\D/g, function(match){
			return match.charAt(1).toUpperCase();
		});
	},

	hyphenate: function(){
		return this.replace(/[A-Z]/g, function(match){
			return ('-' + match.charAt(0).toLowerCase());
		});
	},

	capitalize: function(){
		return this.replace(/\b[a-z]/g, function(match){
			return match.toUpperCase();
		});
	},

	escapeRegExp: function(){
		return this.replace(/([-.*+?^${}()|[\]\/\\])/g, '\\$1');
	},

	toInt: function(base){
		return parseInt(this, base || 10);
	},

	toFloat: function(){
		return parseFloat(this);
	},

	hexToRgb: function(array){
		var hex = this.match(/^#?(\w{1,2})(\w{1,2})(\w{1,2})$/);
		return (hex) ? hex.slice(1).hexToRgb(array) : null;
	},

	rgbToHex: function(array){
		var rgb = this.match(/\d{1,3}/g);
		return (rgb) ? rgb.rgbToHex(array) : null;
	},

	stripScripts: function(option){
		var scripts = '';
		var text = this.replace(/<script[^>]*>([\s\S]*?)<\/script>/gi, function(){
			scripts += arguments[1] + '\n';
			return '';
		});
		if (option === true) $exec(scripts);
		else if ($type(option) == 'function') option(scripts, text);
		return text;
	},

	substitute: function(object, regexp){
		return this.replace(regexp || (/\\?\{([^{}]+)\}/g), function(match, name){
			if (match.charAt(0) == '\\') return match.slice(1);
			return (object[name] != undefined) ? object[name] : '';
		});
	}

});


/*
Script: Hash.js
	Contains Hash Prototypes. Provides a means for overcoming the JavaScript practical impossibility of extending native Objects.

License:
	MIT-style license.
*/

Hash.implement({

	has: Object.prototype.hasOwnProperty,

	keyOf: function(value){
		for (var key in this){
			if (this.hasOwnProperty(key) && this[key] === value) return key;
		}
		return null;
	},

	hasValue: function(value){
		return (Hash.keyOf(this, value) !== null);
	},

	extend: function(properties){
		Hash.each(properties, function(value, key){
			Hash.set(this, key, value);
		}, this);
		return this;
	},

	combine: function(properties){
		Hash.each(properties, function(value, key){
			Hash.include(this, key, value);
		}, this);
		return this;
	},

	erase: function(key){
		if (this.hasOwnProperty(key)) delete this[key];
		return this;
	},

	get: function(key){
		return (this.hasOwnProperty(key)) ? this[key] : null;
	},

	set: function(key, value){
		if (!this[key] || this.hasOwnProperty(key)) this[key] = value;
		return this;
	},

	empty: function(){
		Hash.each(this, function(value, key){
			delete this[key];
		}, this);
		return this;
	},

	include: function(key, value){
		if (this[key] == undefined) this[key] = value;
		return this;
	},

	map: function(fn, bind){
		var results = new Hash;
		Hash.each(this, function(value, key){
			results.set(key, fn.call(bind, value, key, this));
		}, this);
		return results;
	},

	filter: function(fn, bind){
		var results = new Hash;
		Hash.each(this, function(value, key){
			if (fn.call(bind, value, key, this)) results.set(key, value);
		}, this);
		return results;
	},

	every: function(fn, bind){
		for (var key in this){
			if (this.hasOwnProperty(key) && !fn.call(bind, this[key], key)) return false;
		}
		return true;
	},

	some: function(fn, bind){
		for (var key in this){
			if (this.hasOwnProperty(key) && fn.call(bind, this[key], key)) return true;
		}
		return false;
	},

	getKeys: function(){
		var keys = [];
		Hash.each(this, function(value, key){
			keys.push(key);
		});
		return keys;
	},

	getValues: function(){
		var values = [];
		Hash.each(this, function(value){
			values.push(value);
		});
		return values;
	},

	toQueryString: function(base){
		var queryString = [];
		Hash.each(this, function(value, key){
			if (base) key = base + '[' + key + ']';
			var result;
			switch ($type(value)){
				case 'object': result = Hash.toQueryString(value, key); break;
				case 'array':
					var qs = {};
					value.each(function(val, i){
						qs[i] = val;
					});
					result = Hash.toQueryString(qs, key);
				break;
				default: result = key + '=' + encodeURIComponent(value);
			}
			if (value != undefined) queryString.push(result);
		});

		return queryString.join('&');
	}

});

Hash.alias({keyOf: 'indexOf', hasValue: 'contains'});


/*
Script: Event.js
	Contains the Event Native, to make the event object completely crossbrowser.

License:
	MIT-style license.
*/

var Event = new Native({

	name: 'Event',

	initialize: function(event, win){
		win = win || window;
		var doc = win.document;
		event = event || win.event;
		if (event.$extended) return event;
		this.$extended = true;
		var type = event.type;
		var target = event.target || event.srcElement;
		while (target && target.nodeType == 3) target = target.parentNode;

		if (type.test(/key/)){
			var code = event.which || event.keyCode;
			var key = Event.Keys.keyOf(code);
			if (type == 'keydown'){
				var fKey = code - 111;
				if (fKey > 0 && fKey < 13) key = 'f' + fKey;
			}
			key = key || String.fromCharCode(code).toLowerCase();
		} else if (type.match(/(click|mouse|menu)/i)){
			doc = (!doc.compatMode || doc.compatMode == 'CSS1Compat') ? doc.html : doc.body;
			var page = {
				x: event.pageX || event.clientX + doc.scrollLeft,
				y: event.pageY || event.clientY + doc.scrollTop
			};
			var client = {
				x: (event.pageX) ? event.pageX - win.pageXOffset : event.clientX,
				y: (event.pageY) ? event.pageY - win.pageYOffset : event.clientY
			};
			if (type.match(/DOMMouseScroll|mousewheel/)){
				var wheel = (event.wheelDelta) ? event.wheelDelta / 120 : -(event.detail || 0) / 3;
			}
			var rightClick = (event.which == 3) || (event.button == 2);
			var related = null;
			if (type.match(/over|out/)){
				switch (type){
					case 'mouseover': related = event.relatedTarget || event.fromElement; break;
					case 'mouseout': related = event.relatedTarget || event.toElement;
				}
				if (!(function(){
					while (related && related.nodeType == 3) related = related.parentNode;
					return true;
				}).create({attempt: Browser.Engine.gecko})()) related = false;
			}
		}

		return $extend(this, {
			event: event,
			type: type,

			page: page,
			client: client,
			rightClick: rightClick,

			wheel: wheel,

			relatedTarget: related,
			target: target,

			code: code,
			key: key,

			shift: event.shiftKey,
			control: event.ctrlKey,
			alt: event.altKey,
			meta: event.metaKey
		});
	}

});

Event.Keys = new Hash({
	'enter': 13,
	'up': 38,
	'down': 40,
	'left': 37,
	'right': 39,
	'esc': 27,
	'space': 32,
	'backspace': 8,
	'tab': 9,
	'delete': 46
});

Event.implement({

	stop: function(){
		return this.stopPropagation().preventDefault();
	},

	stopPropagation: function(){
		if (this.event.stopPropagation) this.event.stopPropagation();
		else this.event.cancelBubble = true;
		return this;
	},

	preventDefault: function(){
		if (this.event.preventDefault) this.event.preventDefault();
		else this.event.returnValue = false;
		return this;
	}

});


/*
Script: Class.js
	Contains the Class Function for easily creating, extending, and implementing reusable Classes.

License:
	MIT-style license.
*/

function Class(params){
	
	if (params instanceof Function) params = {initialize: params};
	
	var newClass = function(){
		Object.reset(this);
		if (newClass._prototyping) return this;
		this._current = $empty;
		var value = (this.initialize) ? this.initialize.apply(this, arguments) : this;
		delete this._current; delete this.caller;
		return value;
	}.extend(this);
	
	newClass.implement(params);
	
	newClass.constructor = Class;
	newClass.prototype.constructor = newClass;

	return newClass;

};

Function.prototype.protect = function(){
	this._protected = true;
	return this;
};

Object.reset = function(object, key){
		
	if (key == null){
		for (var p in object) Object.reset(object, p);
		return object;
	}
	
	delete object[key];
	
	switch ($type(object[key])){
		case 'object':
			var F = function(){};
			F.prototype = object[key];
			var i = new F;
			object[key] = Object.reset(i);
		break;
		case 'array': object[key] = $unlink(object[key]); break;
	}
	
	return object;
	
};

new Native({name: 'Class', initialize: Class}).extend({

	instantiate: function(F){
		F._prototyping = true;
		var proto = new F;
		delete F._prototyping;
		return proto;
	},
	
	wrap: function(self, key, method){
		if (method._origin) method = method._origin;
		
		return function(){
			if (method._protected && this._current == null) throw new Error('The method "' + key + '" cannot be called.');
			var caller = this.caller, current = this._current;
			this.caller = current; this._current = arguments.callee;
			var result = method.apply(this, arguments);
			this._current = current; this.caller = caller;
			return result;
		}.extend({_owner: self, _origin: method, _name: key});

	}
	
});

Class.implement({
	
	implement: function(key, value){
		
		if ($type(key) == 'object'){
			for (var p in key) this.implement(p, key[p]);
			return this;
		}
		
		var mutator = Class.Mutators[key];
		
		if (mutator){
			value = mutator.call(this, value);
			if (value == null) return this;
		}
		
		var proto = this.prototype;

		switch ($type(value)){
			
			case 'function':
				if (value._hidden) return this;
				proto[key] = Class.wrap(this, key, value);
			break;
			
			case 'object':
				var previous = proto[key];
				if ($type(previous) == 'object') $mixin(previous, value);
				else proto[key] = $unlink(value);
			break;
			
			case 'array':
				proto[key] = $unlink(value);
			break;
			
			default: proto[key] = value;

		}
		
		return this;

	}
	
});

Class.Mutators = {
	
	Extends: function(parent){

		this.parent = parent;
		this.prototype = Class.instantiate(parent);

		this.implement('parent', function(){
			var name = this.caller._name, previous = this.caller._owner.parent.prototype[name];
			if (!previous) throw new Error('The method "' + name + '" has no parent.');
			return previous.apply(this, arguments);
		}.protect());

	},

	Implements: function(items){
		$splat(items).each(function(item){
			if (item instanceof Function) item = Class.instantiate(item);
			this.implement(item);
		}, this);

	}
	
};


/*
Script: Class.Extras.js
	Contains Utility Classes that can be implemented into your own Classes to ease the execution of many common tasks.

License:
	MIT-style license.
*/

var Chain = new Class({

	$chain: [],

	chain: function(){
		this.$chain.extend(Array.flatten(arguments));
		return this;
	},

	callChain: function(){
		return (this.$chain.length) ? this.$chain.shift().apply(this, arguments) : false;
	},

	clearChain: function(){
		this.$chain.empty();
		return this;
	}

});

var Events = new Class({

	$events: {},

	addEvent: function(type, fn, internal){
		type = Events.removeOn(type);
		if (fn != $empty){
			this.$events[type] = this.$events[type] || [];
			this.$events[type].include(fn);
			if (internal) fn.internal = true;
		}
		return this;
	},

	addEvents: function(events){
		for (var type in events) this.addEvent(type, events[type]);
		return this;
	},

	fireEvent: function(type, args, delay){
		type = Events.removeOn(type);
		if (!this.$events || !this.$events[type]) return this;
		this.$events[type].each(function(fn){
			fn.create({'bind': this, 'delay': delay, 'arguments': args})();
		}, this);
		return this;
	},

	removeEvent: function(type, fn){
		type = Events.removeOn(type);
		if (!this.$events[type]) return this;
		if (!fn.internal) this.$events[type].erase(fn);
		return this;
	},

	removeEvents: function(events){
		var type;
		if ($type(events) == 'object'){
			for (type in events) this.removeEvent(type, events[type]);
			return this;
		}
		if (events) events = Events.removeOn(events);
		for (type in this.$events){
			if (events && events != type) continue;
			var fns = this.$events[type];
			for (var i = fns.length; i--; i) this.removeEvent(type, fns[i]);
		}
		return this;
	}

});

Events.removeOn = function(string){
	return string.replace(/^on([A-Z])/, function(full, first) {
		return first.toLowerCase();
	});
};

var Options = new Class({

	setOptions: function(){
		this.options = $merge.run([this.options].extend(arguments));
		if (!this.addEvent) return this;
		for (var option in this.options){
			if ($type(this.options[option]) != 'function' || !(/^on[A-Z]/).test(option)) continue;
			this.addEvent(option, this.options[option]);
			delete this.options[option];
		}
		return this;
	}

});


/*
Script: JSON.js
	JSON encoder and decoder.

License:
	MIT-style license.

See Also:
	<http://www.json.org/>
*/

var JSON = new Hash({

	$specialChars: {'\b': '\\b', '\t': '\\t', '\n': '\\n', '\f': '\\f', '\r': '\\r', '"' : '\\"', '\\': '\\\\'},

	$replaceChars: function(chr){
		return JSON.$specialChars[chr] || '\\u00' + Math.floor(chr.charCodeAt() / 16).toString(16) + (chr.charCodeAt() % 16).toString(16);
	},

	encode: function(obj){
		switch ($type(obj)){
			case 'string':
				return '"' + obj.replace(/[\x00-\x1f\\"]/g, JSON.$replaceChars) + '"';
			case 'array':
				return '[' + String(obj.map(JSON.encode).filter($defined)) + ']';
			case 'object': case 'hash':
				var string = [];
				Hash.each(obj, function(value, key){
					var json = JSON.encode(value);
					if (json) string.push(JSON.encode(key) + ':' + json);
				});
				return '{' + string + '}';
			case 'number': case 'boolean': return String(obj);
			case false: return 'null';
		}
		return null;
	},

	decode: function(string, secure){
		if ($type(string) != 'string' || !string.length) return null;
		if (secure && !(/^[,:{}\[\]0-9.\-+Eaeflnr-u \n\r\t]*$/).test(string.replace(/\\./g, '@').replace(/"[^"\\\n\r]*"/g, ''))) return null;
		return eval('(' + string + ')');
	}

});

Native.implement([Hash, Array, String, Number], {

	toJSON: function(){
		return JSON.encode(this);
	}

});

/*
Script: Cookie.js
	Class for creating, loading, and saving browser Cookies.

License:
	MIT-style license.

Credits:
	Based on the functions by Peter-Paul Koch (http://quirksmode.org).
*/

var Cookie = new Class({

	Implements: Options,

	options: {
		path: false,
		domain: false,
		duration: false,
		secure: false,
		document: document
	},

	initialize: function(key, options){
		this.key = key;
		this.setOptions(options);
	},

	write: function(value){
		value = encodeURIComponent(value);
		if (this.options.domain) value += '; domain=' + this.options.domain;
		if (this.options.path) value += '; path=' + this.options.path;
		if (this.options.duration){
			var date = new Date();
			date.setTime(date.getTime() + this.options.duration * 24 * 60 * 60 * 1000);
			value += '; expires=' + date.toGMTString();
		}
		if (this.options.secure) value += '; secure';
		this.options.document.cookie = this.key + '=' + value;
		return this;
	},

	read: function(){
		var value = this.options.document.cookie.match('(?:^|;)\\s*' + this.key.escapeRegExp() + '=([^;]*)');
		return (value) ? decodeURIComponent(value[1]) : null;
	},

	dispose: function(){
		new Cookie(this.key, $merge(this.options, {duration: -1})).write('');
		return this;
	}

});

Cookie.write = function(key, value, options){
	return new Cookie(key, options).write(value);
};

Cookie.read = function(key){
	return new Cookie(key).read();
};

Cookie.dispose = function(key, options){
	return new Cookie(key, options).dispose();
};



/*
Script: Request.js
	Powerful all purpose Request Class. Uses XMLHTTPRequest.

License:
	MIT-style license.
*/

var Request = new Class({

	Implements: [Chain, Events, Options],

	options: {/*
		onRequest: $empty,
		onComplete: $empty,
		onCancel: $empty,
		onSuccess: $empty,
		onFailure: $empty,
		onException: $empty,*/
		url: '',
		data: '',
		headers: {
			'X-Requested-With': 'XMLHttpRequest',
			'Accept': 'text/javascript, text/html, application/xml, text/xml, */*'
		},
		async: true,
		format: false,
		method: 'post',
		link: 'ignore',
		isSuccess: null,
		emulation: true,
		urlEncoded: true,
		encoding: 'utf-8',
		evalScripts: false,
		evalResponse: false,
		noCache: false
	},

	initialize: function(options){
		this.xhr = new Browser.Request();
		this.setOptions(options);
		this.options.isSuccess = this.options.isSuccess || this.isSuccess;
		this.headers = new Hash(this.options.headers);
	},

	onStateChange: function(){
		if (this.xhr.readyState != 4 || !this.running) return;
		this.running = false;
		this.status = 0;
		$try(function(){
			this.status = this.xhr.status;
		}.bind(this));
		if (this.options.isSuccess.call(this, this.status)){
			this.response = {text: this.xhr.responseText, xml: this.xhr.responseXML};
			this.success(this.response.text, this.response.xml);
		} else {
			this.response = {text: null, xml: null};
			this.failure();
		}
		this.xhr.onreadystatechange = $empty;
	},

	isSuccess: function(){
		return ((this.status >= 200) && (this.status < 300));
	},

	processScripts: function(text){
		if (this.options.evalResponse || (/(ecma|java)script/).test(this.getHeader('Content-type'))) return $exec(text);
		return text.stripScripts(this.options.evalScripts);
	},

	success: function(text, xml){
		this.onSuccess(this.processScripts(text), xml);
	},

	onSuccess: function(){
		this.fireEvent('complete', arguments).fireEvent('success', arguments).callChain();
	},

	failure: function(){
		this.onFailure();
	},

	onFailure: function(){
		this.fireEvent('complete').fireEvent('failure', this.xhr);
	},

	setHeader: function(name, value){
		this.headers.set(name, value);
		return this;
	},

	getHeader: function(name){
		return $try(function(){
			return this.xhr.getResponseHeader(name);
		}.bind(this));
	},

	check: function(){
		if (!this.running) return true;
		switch (this.options.link){
			case 'cancel': this.cancel(); return true;
			case 'chain': this.chain(this.caller.bind(this, arguments)); return false;
		}
		return false;
	},

	send: function(options){
		if (!this.check(options)) return this;
		this.running = true;

		var type = $type(options);
		if (type == 'string' || type == 'element') options = {data: options};

		var old = this.options;
		options = $extend({data: old.data, url: old.url, method: old.method}, options);
		var data = options.data, url = options.url, method = options.method;

		switch ($type(data)){
			case 'element': data = $(data).toQueryString(); break;
			case 'object': case 'hash': data = Hash.toQueryString(data);
		}

		if (this.options.format){
			var format = 'format=' + this.options.format;
			data = (data) ? format + '&' + data : format;
		}

		if (this.options.emulation && ['put', 'delete'].contains(method)){
			var _method = '_method=' + method;
			data = (data) ? _method + '&' + data : _method;
			method = 'post';
		}

		if (this.options.urlEncoded && method == 'post'){
			var encoding = (this.options.encoding) ? '; charset=' + this.options.encoding : '';
			this.headers.set('Content-type', 'application/x-www-form-urlencoded' + encoding);
		}

		if(this.options.noCache) {
			var noCache = "noCache=" + new Date().getTime();
			data = (data) ? noCache + '&' + data : noCache;
		}


		if (data && method == 'get'){
			url = url + (url.contains('?') ? '&' : '?') + data;
			data = null;
		}


		this.xhr.open(method.toUpperCase(), url, this.options.async);

		this.xhr.onreadystatechange = this.onStateChange.bind(this);

		this.headers.each(function(value, key){
			try {
				this.xhr.setRequestHeader(key, value);
			} catch (e){
				this.fireEvent('exception', [key, value]);
			}
		}, this);

		this.fireEvent('request');
		this.xhr.send(data);
		if (!this.options.async) this.onStateChange();
		return this;
	},

	cancel: function(){
		if (!this.running) return this;
		this.running = false;
		this.xhr.abort();
		this.xhr.onreadystatechange = $empty;
		this.xhr = new Browser.Request();
		this.fireEvent('cancel');
		return this;
	}

});

(function(){

var methods = {};
['get', 'post', 'put', 'delete', 'GET', 'POST', 'PUT', 'DELETE'].each(function(method){
	methods[method] = function(){
		var params = Array.link(arguments, {url: String.type, data: $defined});
		return this.send($extend(params, {method: method.toLowerCase()}));
	};
});

Request.implement(methods);

})();
var APE = {
	'version': '1.0b1',
	'Request': {},
	'Transport': {}
};
APE.Events = new Class({
	
	Extends: Events,
		
	onRaw: function(type, fn, internal) {
		return this.addEvent('raw_' + type.toLowerCase(), fn, internal);
	},
	
	onCmd: function(type, fn, internal) {                                  
		return this.addEvent('cmd_' + type.toLowerCase(), fn, internal);
	},
	
	onError: function(type, fn, internal) {                                
		return this.addEvent('error_' + type, fn, internal);
	}
	
});
/*
  Copyright (C) 2008-2009 Weelya <contact@weelya.com> 
  This file is part of APE Client.
  APE is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  APE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with APE ; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  
*/

/***							    ________________________________________________________
 *                 __------__      /										   	  		    \
 *               /~          ~\   | APE, the Ajax Push Engine made with heart (and MooTools) |
 *              |    //^\\//^\|   |    http://www.weelya.net - http://www.ape-project.org    |
 *            /~~\  ||  o| |o|:~\  \ _______________________________________________________/
 *           | |6   ||___|_|_||:|  /
 *            \__.  /      o  \/' / 
 *             |   (       O   )_/
 *    /~~~~\    `\  \         /
 *   | |~~\ |     )  ~------~`\
 *  /' |  | |   /     ____ /~~~)\
 * (_/'   | | |     /'    |    ( |
 *        | | |     \    /   __)/ \
 *        \  \ \      \/    /' \   `\
 *          \  \|\        /   | |\___|
 *            \ |  \____/     | |
 *            /^~>  \        _/ <
 *           |  |         \       \
 *           |  | \        \        \
 *           -^-\  \       |        )
 *                `\_______/^\______/
 */
APE.Core = new Class({

	Implements: [APE.Events, Options],

	$originalEvents: {},

	options:{
		server: '', // APE server URL
		pollTime: 25000, // Max time for a request
		identifier: 'ape', // Identifier is used by cookie to differentiate ape instance
		transport: 0, // Transport 0: long polling, 1 : XHRStreaming, 2: JSONP, 3 SSE / JSONP, 4 : SSE / XHR
		frequency: 0, // Frequency identifier
		cycledStackTime: 350 //Time before send request of cycledStack
	},

	initialize: function(options){
		window.Ape = this;
		this.setOptions(options);

		this.selectTransport();
		this.request = new APE.Request(this);

		this.pipes = new $H; 
		this.users = new $H;

		this.sessid = null;
		this.pubid = null;

		this.timer = null;
		this.status = 0; // 0 = APE is not initialized, 1 = connected, -1 = Disconnected by timeout, -2 = Disconnected by request failure
		this.failCounter = 0;
		this.pollerObserver = null;
		this.requestDisabled = false;

		this.onRaw('login', this.rawLogin);
		this.onRaw('err', this.rawErr);
		this.onRaw('ident', this.rawIdent);

		this.onError('003', this.clearSession);
		this.onError('004', this.clearSession);

		//Set core var for APE.Client instance
		if (options.init) options.init.apply(null, [this]);

		//Execute complete function of APE.Client instance
		if (options.complete) options.complete.apply(null, [this]);
		this.fireEvent('load', this);

		if (this.options.connectOptions) this.start(this.options.connectOptions);
	},

	selectTransport: function() {
		var transports = [APE.Transport.longPolling, APE.Transport.XHRStreaming, APE.Transport.JSONP];
		var transport = this.options.transport;
		var support;

		while (support != true) {
			support = transports[transport].browserSupport();//Test if browser support transport	

			if (support) {
				this.options.transport = transport;
				this.transport = new transports[transport](this);
			}
			else transport = support;//Browser do not support transport, next loop will test with fallback transport returned by browserSupport();
		}
	},
	poller: function() {
		if (this.pollerActive) this.check();
	},

	startPoller: function() {
		this.pollerActive = true;
	},

	stopPoller: function() {
		$clear(this.pollerObserver);
		this.pollerActive = false;
	},
	
	stopRequest: function() {
		this.cancelRequest();
		if (this.transport.streamRequest) this.transport.streamRequest.cancel();
		this.requestDisabled = true;
	},

	parseParam: function(param) {
		return ($type(param) == 'object') ? Hash.getValues(param) : $splat(param);
	},

	cancelRequest: function() {
		this.transport.cancel();
	},

	/***
	 * Function called when a request fail or timeout
	 */
	requestFail: function(args, failStatus, request) {
		var reSendData = false;

		if (request.request && !request.request.dataSent) reSendData = true;
		if (this.status > 0) {//APE is connected but request failed
			this.status = failStatus;
			this.cancelRequest();
			this.stopPoller();
			this.fireEvent('apeDisconnect');
		} 

		if (this.failCounter < 6) this.failCounter++;

		//Cancel last request
		this.cancelRequest();

		var delay = (this.failCounter*1000);
		if (reSendData) {
			this.request.send.delay(delay, this.request, args);
		} else {
			this.check.delay(delay, this);
		}
	},

	/***
	 * Parse received data from Server
	 */
	parseResponse: function(raws, callback){
		if (raws) {
			if (this.status < 0 ) {
				this.failCounter = 0;
				this.status = 1;
				this.startPoller();
				this.fireEvent('apeReconnect');
			}
		}

		var check = false;
		var chlCallback;//Callback on challenge

		if (raws) {
			raws = JSON.decode(raws, true);//TODO replace me by eval for performance purpose
			if (!raws){ // Something went wrong, json decode failed
				this.check();
				return;
			}

			for (var i = 0; i < raws.length; i++){ //Read all raw
				var raw = raws[i];

				if (callback && $type(callback) == 'function') {
					callback.run(raw);
				}

				this.callRaw(raw);

				//Last request is finished and it's not an error
				if (!this.transport.running()) {
					if (!raw.data.code || (raw.data.code!='005' && raw.data.code!= '001' && raw.data.code != '004' && raw.data.code != '003')) {
						check = true;
					}
				} else {
					//Invalidate check if something went wrong with other raw or a new request have been launched
					check = false;
				}
			}
		}
		if (check) this.check();
	},

	/***
	 * Fire raw event. If received raw is on a non-existing pipe, create new pipe
	 */
	callRaw: function(raw) {
		var args;
		if (raw.data.pipe) {
			var pipeId = raw.data.pipe.pubid, pipe;
			if (!this.pipes.has(pipeId)) {
				pipe = this.newPipe(raw.data.pipe.casttype, raw.data);
			} else {
				pipe = this.pipes.get(pipeId);
				//Update pipe properties
				pipe.properties = raw.data.pipe.properties;
			}
			if (pipe) {
				args = [raw, pipe];
				pipe.fireEvent('raw_' + raw.raw.toLowerCase(), args);
			}
		} else {
			args = raw;
		}
		if (raw.data.chl) {//Execute callback on challenge
			var chlCallback = this.request.callbackChl.get(raw.data.chl);
			if (chlCallback) {
				chlCallback.run(raw);
				this.request.callbackChl.erase(raw.data.chl);
			}
		}
		this.fireEvent('raw_' + raw.raw.toLowerCase(), args);
	},
	
	newPipe: function(type, options){
		if (options && options.pipe.pubid) {
			var pipe = this.pipes.get(options.pipe.pubid)
			if (pipe) return pipe;
		} 

		if(type == 'uni') return new APE.PipeSingle(this, options);
		if(type == 'multi') return new APE.PipeMulti(this, options);
		if(type == 'proxy') return new APE.PipeProxy(this, options);
	},

	/***
	 * Add a pipe to the core pipes hash
	 */
	addPipe: function(pubid, pipe){
		return this.pipes.set(pubid, pipe); 
	},

	getPipe: function(pubid){
		var pipe = this.pipes.get(pubid);
		if (!pipe) {
			pipe = this.users.get(pubid);
			if (pipe) pipe = this.newPipe('uni', {'pipe': pipe});
		}
		return pipe;
	},

	/***
	 * Remove a pipe from the pipe hash and fire event 'pipeDelete'
	 */
	delPipe: function(pubid){
		var pipe = this.pipes.get(pubid);
		this.pipes.erase(pubid);
		this.fireEvent(pipe.type+'PipeDelete', [pipe.type, pipe]);
		return pipe;
	},
	
	check: function(){
		this.request.send('CHECK', null, true);
	},

	start: function(options, sendStack){
		this.connect(options, sendStack); 
	},

	connect: function(options, sendStack){
		this.request.stack.add('CONNECT', options, false, false);
		if (this.options.channel) { 
			var channel;
			if ($type(this.options.channel) == 'object') {
				channel = [];
				$each(this.options.channel, function(c) {
						channel.push(c);
				});
			} else channel = this.options.channel
			this.request.stack.add('JOIN', {"channels": channel}, false);
		}
		if (sendStack !== false) this.request.stack.send();
	},

	join: function(channel){
		this.request.send('JOIN', {"channels":channel});
	},

	left: function(pubid){
		this.request.send('LEFT', {"channel":this.pipes.get(pubid).name});
		this.delPipe(pubid);
	},

	quit: function(){
		this.request.send('QUIT');
		this.clearSession();
	},

	getPubid: function(){
		return this.pubid;
	},

	getSessid:function(){
		return this.sessid;
	},

	setSession: function(obj, options) {
		if (this.restoring) return;
		if (options) this.request.setOptions(options);

		this.request.send('SESSION', {'action': 'set', 'values': obj}, true);
	},

	getSession: function(key, callback){
		var options = {};

		if (callback) {
			options.callback = function(resp) { 
				if (resp.raw == 'SESSIONS') this.run(arguments) 
			}.bind(callback)
		}

		this.request.send('SESSION', {
				'action':'get', 
				'values': (($type(key) == 'array') ? key : [key])
			}, true, options );
	},
	
	rawIdent: function(raw){
		this.user = raw.data.user;
		this.pubid = raw.data.user.pubid;
	},

	rawLogin: function(param){
		this.sessid = param.data.sessid;

		this.status = 1;
		this.startPoller();
		this.fireEvent('ready');
		this.fireEvent('init');
	},

	rawErr: function(err){
		this.fireEvent('error_' + err.data.code, err);
	},
	
	/***
	 * Clear the sessions, clean timer, remove cookies, remove events
	 */
	clearSession:function(){
		//Clear all APE var
		this.sessid = null;
		this.pubid = null;
		this.$events = {}; 
		this.request.chl = 1;
		this.status = 0;
		this.options.restore = false;
		
		this.fireEvent('clearSession');
		this.stopPoller();
		this.cancelRequest();
	}
});

var Ape;  

window.onload = function(){
	var config = window.APEConfig;
	if (!config) {
		var identifier = window.frameElement.id;
		config = window.parent.APE.Config[identifier.substring(4, identifier.length)];
	}
	//Delay of 1ms allow browser to do not show a loading message
	(function() {
		new APE.Core(config);
	}).delay(1);
};
APE.Pipe  = new Class({

	Implements: APE.Events,

	initialize: function(ape, options){
		this.pipe = options.pipe;
		this.properties = options.pipe.properties;

		this.ape = ape;
		
		this.initRequestMethod();

		this.ape.addPipe(this.pipe.pubid, this);
	},

	initRequestMethod: function() {
		this.request = {};
		this.request = {
			send: function() {
				var args = this.parsePipeCmd.apply(this, arguments);
				this.ape.request.send.apply(this.ape.request, args);
			}.bind(this),
			cycledStack: {
				add: function() {
					var args = this.parsePipeCmd.apply(this, arguments);
					this.ape.request.cycledStack.add.apply(this.ape.request.cycledStack, args);
				}.bind(this),
				send: this.ape.request.send,
				setTime: this.ape.request.cycledStack.setTime.bind(this.ape.request.cycledStack)
			},
			stack :  {
				add: function() {
					var args = this.parsePipeCmd.apply(this, arguments);
					this.ape.request.stack.add.apply(this.ape.request.stack, args);
				}.bind(this),
				send: this.ape.request.stack.send.bind(this.ape.request.stack)
			}
		}
	},

	parsePipeCmd: function() {
			if ($type(arguments[0]) == 'array') {
				var args = arguments;
				for (var i = 0; i < args[0].length; i++) {
					if (!args[0][i].params) args[0][i].params = {};
					if (this.pipe) args[0][i].pipe = this.pipe.pubid;
				}
			} else {
				var args = arguments;
				if (!args[1]) args[1] = {};
				if (this.pipe) args[1].pipe = this.pipe.pubid;
			}
			return args;
	},

	send: function(data){
		this.request.send('SEND', {'msg': data});
	},

	left: function() {
		this.ape.left(this.pipe.pubid);
	},

	getPubid: function(){
		return this.pipe.pubid;
	},

	fireGlobalEvent: function(type, fn, internal) {
		this.fireEvent(type, fn, internal);
		this.ape.fireEvent(type, fn, internal);
	},

	fireInTheHall: this.fireGlobalEvent

});
APE.PipeProxy = new Class({

	Extends: APE.Pipe,

	initialize: function(core, options){
		this.core = core || window.Ape;
		this.ape = this.core;

		this.initRequestMethod();
		this.type = 'proxy';

		if (options) {
			this.init(options);
		}
	},

	init: function(options){
		this.pipe = options.pipe;

		this.core.addPipe(this.getPubid(), this);

		this.onRaw('proxy_event', this.rawProxyEvent );
		this.ape.fireEvent('proxyPipeCreate', [this, options]);
	},

	reset: function() {
		//close connection
	},

	close: function() {
		//close connection
	},

	open: function(hostname, port){
		if (this.core.status == 0) this.core.start(null, false);
		//Adding a callback to request response to create a new pipe if this.pipe haven't been init
		this.request.stack.add('PROXY_CONNECT', {'host':hostname, 'port':port}, true, this.pipe ? {} : {'callback':this.callback.bind(this)});
		this.request.stack.send();
	},

	send: function(data){
	      this.request.send('SEND', {'msg':B64.encode(data)});
	},

	rawProxyEvent: function(resp, pipe){
		if(!this.pipe) this.init(resp.data);
		switch (resp.data.event) {
			case 'read':
				var data = B64.decode(resp.data.data);
				this.fireGlobalEvent('proxyRead', data)
				if (this.onread) this.onread(data);
				break;
			case 'connect':
				this.fireGlobalEvent('proxyConnect');
				if (this.onopen) this.onopen();
				break;
			case 'close':
				this.fireGlobalEvent('proxyClose');
				if (this.onclose) this.onclose();
				break;
		}
	},

	callback: function(raw){
		this.init(raw.data);
	}
});

APE.Core = new Class({

	Extends: APE.Core,

	/***
	 * This allow ape to be compatible with TCPSocket
	 */
	TCPSocket: APE.PipeProxy
});
APE.PipeMulti = new Class({

	Extends: APE.Pipe,

	initialize: function(core, options) {
		this.parent(core, options);

		this.type = 'multi';
		this.name = options.pipe.properties.name;

		//Test if pipe have users before sending event
		//because this.users need to be defined
		if (options.users) {
			this.users = new $H;
			var users = options.users;
		}

		this.ape.fireEvent('multiPipeCreate', [this, options]);

		if (options.users) {
			var l = users.length;
			for (var i=0; i < l; i++) {
				this.addUser(users[i].pubid, users[i]);
			}
			this.onRaw('left', this.rawLeft);
			this.onRaw('join', this.rawJoin);
		}
	},

	rawJoin: function(raw, pipe) {
		this.addUser(raw.data.user.pubid, raw.data.user);
	},

	rawLeft: function(raw, pipe) {
		this.delUser(raw.data.user.pubid);
	},

	addUser: function(pubid, user) {
		if (!this.ape.users.has(user.pubid)) {
			user.pipes = new $H;
			this.ape.users.set(pubid, user);
		} else {
			user = this.ape.users.get(pubid);
		}
		user.pipes.set(this.pipe.pubid, this);
		var u = {'pipes':user.pipes ,'casttype': user.casttype, 'pubid': user.pubid, 'properties': user.properties};
		this.users.set(pubid, u);
		this.fireGlobalEvent('userJoin', [u, this]);
		return u;
	},

	delUser: function(pubid) {
		var u = this.users.get(pubid);
		this.users.erase(pubid);
		u.pipes.erase(this.pipe.pubid)
		if (u.pipes.getLength() == 0) {
			this.ape.users.erase(u.pubid);
		}
		this.fireGlobalEvent('userLeft', [u, this]);
		return u;
	},

	getUser: function(pubid) {
		return this.users.get(pubid);
	},
	
	getUserPipe: function(user) {
		if (typeof user == 'string') user = this.users.get(users.pubid);
		return this.ape.newPipe('uni', {'pipe':user});
	}
});
APE.PipeSingle = new Class({

	Extends: APE.Pipe,

	initialize: function(core, options){
		this.parent(core, options);
		this.type = 'uni';
		this.ape.fireEvent('uniPipeCreate',[this, options]);
	}
});
APE.Request = new Class({
	initialize: function(ape) {
		this.ape = ape;
		this.stack = new APE.Request.Stack(ape);
		this.cycledStack = new APE.Request.CycledStack(ape);
		this.options = {};
		this.chl = 1;
		this.callbackChl = new $H;

		//Fix presto bug (see send method)
		if (Browser.Engine.presto){
			this.requestVar = {
				updated: false,
				args: []
			};
			this.requestObserver.periodical(10, this);
		}
	},

	setOptions: function(options) {
		this.options = $merge(options, this.options);
	},

	send: function(cmd, params, sessid, options, noWatch) {
		if (this.ape.requestDisabled) return;
		//Opera dirty fix
		if (Browser.Engine.presto && !noWatch) {
			this.requestVar.updated = true;
			this.requestVar.args.push([cmd, params, sessid, options]);
			return;
		}

		this.options = $merge({
			event: true,
			callback: null
		}, this.options);

		var ret = this.ape.transport.send(this.parseCmd(cmd, params, sessid, options), this.options, noWatch);

		$clear(this.ape.pollerObserver);
		this.ape.pollerObserver = this.ape.poller.delay(this.ape.options.pollTime, this.ape);

		this.options = {};//Reset options

		return ret;
	},

	parseCmd: function(cmd, params, sessid, options) {
		var queryString = '';
		var a = [];
		var o = {};
		if ($type(cmd) == 'array') {
			var tmp, evParams;
			for (var i = 0; i < cmd.length; i++) {
				tmp = cmd[i];

				o = {};
				o.cmd = tmp.cmd;
				o.chl = this.chl++;

				tmp.params ? o.params = tmp.params : null;
				evParams = $extend({}, o.params);

				this.escapeParams(o.params);

				if (!$defined(tmp.sessid) || tmp.sessid !== false) o.sessid = this.ape.getSessid();
				a.push(o);

				var ev = 'cmd_' + tmp.cmd.toLowerCase();
				if (tmp.options && tmp.options.callback) this.callbackChl.set(o.chl, tmp.options.callback);
				if (this.options.event) {
					//Request is on a pipe, fire the event on the core & on the pipe
					if (o.params && o.params.pipe) {
						var pipe = this.ape.getPipe(o.params.pipe);
						evParams = [pipe, evParams];
						pipe.fireEvent(ev, evParams);
					}
					this.ape.fireEvent(ev, evParams);
				}
			}
		} else {
			o.cmd = cmd;
			o.chl = this.chl++;

			params ? o.params = params : null;
			var evParams = $extend({}, params);

			this.escapeParams(params);

			if (!$defined(sessid) || sessid !== false) o.sessid = this.ape.getSessid();
			a.push(o);
			
			var ev = 'cmd_' + cmd.toLowerCase();
			if (options && options.callback) this.callbackChl.set(o.chl, options.callback);

			if (this.options.event) {
				//Request is on a pipe, fire the event on the pipe
				if (params && params.pipe) { 
					var pipe = this.ape.getPipe(params.pipe);
					if (pipe) {
						evParams = [pipe, evParams];
						pipe.fireEvent(ev, evParams);
					}
				}
				this.ape.fireEvent(ev, evParams);
			}
		}
		return JSON.encode(a);
	},

	escapeParams: function(params) {
		for (var i in params) {
			if (params.hasOwnProperty(i)) {
				if (typeof params[i] == 'string') {
					params[i] = escape(params[i]);
					if (this.ape.options.transport) params[i] = escape(params[i]); //In case of JSONP data have to be escaped two times
				}
				else this.escapeParams(params[i]);
			}
		}
	},

	/****
	 * This method is only used by opera.
	 * Opera have a bug, when request are sent trought user action (ex : a click), opera throw a security violation when trying to make a XHR.
	 * The only way is to set a class var and watch when this var change
	 */
	requestObserver: function(){
		if (this.requestVar.updated) {
			var args = this.requestVar.args.shift();
			this.requestVar.updated = (this.requestVar.args.length>0) ? true : false;
			args[3] = true; //Set noWatch argument to true
			this.send.run(args, this);
		}
	}
});
APE.Request.Stack = new Class({
	initialize: function(ape) {
		this.ape = ape;
		this.stack =[];
	},
	add: function(cmd, params, sessid, options) {
		this.stack.push({'cmd':cmd, 'params':params, 'sessid':sessid, 'options': options});
	},
	send: function() {
		this.ape.request.send(this.stack);
		this.stack = [];
	}
});
APE.Request.CycledStack = new Class({
	initialize: function(ape) {
		this.ape = ape;

		this.timer = this.send.periodical(this.ape.options.cycledStackTime, this);

		this.stack = [];
		this.reajustTime = false;
	},

	add: function(cmd, params, sessid) {
		this.stack.push({'cmd':cmd, 'params':params, 'sessid':sessid});
	},

	setTime: function(time, now) {
		if (now) {
			this.send();
			$clear(this.timer);
			this.timer = this.send.periodical(time, this);
			this.reajustTime = false;
		}
		else this.reajustTime = time;
	},

	send: function() {
		if (this.stack.length > 0) {
			this.ape.request.send(this.stack);
			this.stack = [];
			if (this.reajustTime) {
				this.setTime(this.reajustTime, true);
			}
		}
	}
});
Request = new Class({

	Extends: Request,

	onStateChange: function() {
		if (this.xhr.readyState == 1) this.dataSent = true;
		this.parent();
	}
});
APE.Transport.longPolling = new Class({

	initialize: function(ape) { 
		this.ape = ape;
		this.requestFailObserver = [];
	},

	send: function(queryString, options, args) {
		var request = new Request({
			url: 'http://' + this.ape.options.frequency + '.' + this.ape.options.server + '/'+this.ape.options.transport+'/?',
			onFailure: this.ape.requestFail.bind(this.ape, [args, -2, this]),
			onComplete: function(resp) {
				$clear(this.requestFailObserver.shift());
				this.ape.parseResponse(resp, options.callback);
			}.bind(this)
		}).send(queryString);

		this.request = request;

		this.requestFailObserver.push(this.ape.requestFail.delay(this.ape.options.pollTime + 10000, this.ape, [arguments, -1, request]));

		return request;
	},

	running: function() {
		return this.request ? this.request.running : false;
	},

	cancel: function() {
		if (this.request) this.request.cancel();
		$clear(this.requestFailObserver.shift());
	}
});

APE.Transport.longPolling.browserSupport = function() { return Browser.Features.xhr ? true : 2; };
/* Notice : This class in only intended to be use as an implemented class */
APE.Request.SSE = new Class({
	SSESupport: ((typeof window.addEventStream) == 'function'), 

	initSSE: function(queryString, options, readCallback) {
		var tmp = document.createElement('div');
		document.body.appendChild(tmp);
		tmp.innerHTML = '<event-source src="http://' + this.ape.options.frequency + '.' + this.ape.options.server + '/?' + queryString + '&' + $time() + '" id="APE_SSE">';
		this.eventSource = document.getElementById('APE_SSE');
		this.eventSource.addEventListener('ape-data', function(ev) { readCallback.run(ev.data) }, false);
	}
});
Request.XHRStreaming = new Class({

	Extends: Request,

	lastTextLength: 0,
	read: 0, //Contain the amout of data read

	onStateChange: function() {
		if (this.xhr.readyState == 1) this.dataSent = true;
		else if (this.xhr.readyState == 3) this.progress(this.xhr.responseText, this.xhr.responseXML);
		this.parent();
	},

	onProgress: function(){
		this.fireEvent('progress', arguments);
	},

	progress: function(text, xml){
		var length = text.length;
		this.read += length;
		text = text.substr(this.lastTextLength);
		this.lastTextLength = length;
		this.onProgress(this.processScripts(text), xml);
	}
});
APE.Transport.XHRStreaming = new Class({
	
	maxRequestSize: 100000,

	Implements: APE.Request.SSE,

	initialize: function(ape){ 
		this.ape = ape;
		this.requestFailObserver = [];

		//If browser support servent sent event, switch to SSE / XHR transport 
		if (this.SSESupport) this.ape.options.transport = 4;

		this.streamInfo = {
			timeoutObserver: null,
			cleanClose: false,
			forceClose: false,
			callback: null
		}
	},

	send: function(queryString, options, args) {
		if (this.SSESupport && !this.eventSource) {
			this.initSSE(queryString, options, this.readSSE.bind(this));
			if (options.callback) this.streamInfo.callback = options.callback;
		} else {
			if ((!this.streamRequest || !this.streamRequest.running) && !this.eventSource) { //Only one XHRstreaming request is allowed
				this.buffer = '';
				this.request = this.doRequest(queryString, options, args);

				if (options.callback) this.streamInfo.callback = options.callback;
			} else { //Simple XHR request
				var request = new Request({
					url: 'http://' + this.ape.options.frequency + '.' + this.ape.options.server + '/' + this.ape.options.transport + '/?',
					onFailure: this.ape.requestFail.bind(this.ape, [args, -2, this]),
					onComplete: function(resp) {
						$clear(this.requestFailObserver.shift());
						this.request.dataSent = true;//In the case of XHRStreaming. Request are imediatly close.
						this.ape.parseResponse(resp, options.callback);
					}.bind(this)
				}).send(queryString);
				request.id = $time();
				this.request = request;

				//set up an observer to detect request timeout
				this.requestFailObserver.push(this.ape.requestFail.delay(this.ape.options.pollTime + 10000, this.ape, [arguments, -1, request]));

			}

			return this.request;
		}
	},

	doRequest: function(queryString, options, args) {
		this.streamInfo.forceClose = false;

		var request = new Request.XHRStreaming({
			url: 'http://' + this.ape.options.frequency + '.' + this.ape.options.server + '/' + this.ape.options.transport + '/?',
			onProgress: this.readFragment.bindWithEvent(this),
			onFailure: this.ape.requestFail.bind(this.ape, [args, -2, this]),
			onComplete: function(resp) {
				$clear(this.streamInfo.timeoutObserver);
				if (this.ape.status > 0) {
					if (this.streamInfo.cleanClose) this.ape.check();
					else this.newStream();
					this.streamInfo.cleanClose = false;
				}
			}.bind(this)
		}).send(queryString);
		
		request.id = $time();
		this.streamRequest = request;
		
		//this should no longer exist
		//this.streamInfo.timeoutObserver = (function() {
		//	this.streamInfo.forceClose = true;
		//	//try to imediatly close stream
		//	if (this.checkStream()) this.newStream();
		//}).delay(1000*60, this);

		return request;
	},

	readSSE: function(data) {
		this.ape.parseResponse(data, this.streamInfo.callback);
		this.streamInfo.callback = null;
	},

	readFragment: function(text){
		this.streamInfo.canClose = false;

		if (text == '') {

			this.streamInfo.canClose = true;
			this.streamInfo.cleanClose = true;

			this.ape.parseResponse(text, this.streamInfo.callback);

			this.streamInfo.callback = null;
		} else {
			text = this.buffer + text;
			var group = text.split("\n\n");
			var length = group.length;
			
			// If group.length is gretter than 1 the fragment received complete last RAW or contains more than one RAW
			if (group.length > 1) { 	
				//Clear buffer
				this.buffer = '';
				
				if (group[length-1] !== '') { //Last group complete last received raw but it's not finish
					this.buffer += group[length-1];
				} else { //Received fragment is complete
					this.streamInfo.canClose = true;
					if (this.checkStream()) this.newStream();
				}


				for (var i = 0; i < length-1; i++) { 
					this.ape.parseResponse(group[i], this.streamInfo.callback);
				}
				//Delete callback
				this.streamInfo.callback = null;
			} else {//Fragement received is a part of a raw 
				this.buffer = text; 
			}
		}
	},
	
	running: function() {
		return (this.streamRequest && this.streamRequest.running) ? true : this.eventSource ? true : false;
	},	

	checkStream: function() {
		return (this.streamInfo.forceClose && this.streamInfo.canClose) || (this.streamRequest && this.streamRequest.read >= this.maxRequestSize && this.streamInfo.canClose);
	},

	newStream: function() {
//		this.ape.request.send('CLOSE');//This will close the stream request
		$clear(this.streamInfo.timeoutObserver);
		this.streamRequest.cancel();
		this.ape.check();
	},

	cancel: function(){
		if (this.request) this.request.cancel();

		$clear(this.streamInfo.timeoutObserver);
		$clear(this.requestFailObserver.shift());
	}
});
APE.Transport.XHRStreaming.browserSupport = function() {
	if (Browser.Features.xhr) {
		if (Browser.Engine.presto && ((typeof window.addEventStream) == 'function')) return true;
	//	else if (window.XDomainRequest) return true; //Not yet :p
		else return Browser.Engine.trident ? 0 : true;
	} else return 2;//No XHR Support, switch to JSONP
}

APE.Transport.JSONP = new Class({
	
	Implements: APE.Transport.SSE,

	initialize: function(ape) {
		this.ape = ape;
		this.requestFailObserver = [];
		this.requests = [];
		
		//If browser support servent sent event, switch to SSE / JSONP transport 
		if (this.SSESupport) this.ape.options.transport = 3;
		
		window.parent.onkeyup = function(ev) {
			if (ev.keyCode == 27) {
				this.cancel();//Escape key
				if (this.ape.status > 0) {
					if (!this.SSESupport) this.ape.request('CLOSE');
				}
			}
		}.bind(this);
	},

	send: function(queryString, options, args) {
		//Opera has some trouble with JSONP, so opera use mix of SSE & JSONP
		if (this.SSESupport && !this.eventSource) {
			this.initSSE(queryString, options, this.readSSE.bind(this));
		} else {
			this.callback = options.callback;

			var request = document.createElement('script');
			request.src = 'http://' + this.ape.options.frequency + '.' + this.ape.options.server + '/' + this.ape.options.transport +'/?' + queryString;
			document.head.appendChild(request);
			this.requests.push(request);
			//Detect timeout
			this.requestFailObserver.push(this.ape.requestFail.delay(this.ape.options.pollTime + 10000, this.ape, [arguments, -1, request]));

			if (Browser.Engine.gecko) {
				//Firefox hack to avoid status bar always show a loading message
				//Ok this hack is little bit weird but it works!
				(function() {
					var tmp = document.createElement('iframe');
					document.body.appendChild(tmp);
					document.body.removeChild(tmp);
				}).delay(200);
			}
		}
	},

	clearRequest: function(request) {
		request.parentNode.removeChild(request);
		//Avoid memory leaks
		if (request.clearAttributes) {
			request.clearAttributes();
		} else { 
			for (var prop in request) delete request[prop];
		}
		$clear(this.requestFailObserver.shift());
	},

	readSSE: function(data) {
		this.ape.parseResponse(data, this.callback);
		this.callback = null;
	},

	read: function(resp) {
		$clear(this.requestFailObserver.shift());
		this.clearRequest(this.requests.shift());
		this.ape.parseResponse(resp, this.callback);

		this.callback = null;
	},

	cancel: function() {
		if (this.requests.length > 0) {
			this.clearRequest(this.requests.shift());
		}
	},

	running: function() {
		if (this.SSESupport) {
			return this.eventSource ? true : false;
		} else {
			return this.requests.length > 0 ? true : false;
		}
	}

	
});

APE.Transport.JSONP.browserSupport = function() { return true };
String.implement({

	addSlashes: function(){
		return this.replace(/("|'|\\|\0)/g, '\\$1');
	},

	stripSlashes: function(){
		return this.replace(/\\("|'|\\|\0)/g, '$1');
	}
});

var B64 = new Hash({

	$p: '=',
	$tab: 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/',

	/***
	 * Base 64 encode / Base 64 decode
	 * Taken from orbited project - http://www.orbited.org
	 */
	encode: function(ba){
		//  Encode a string as a base64-encoded string
		var s = [], l = ba.length;
		var rm = l%3;
		var x = l - rm;
		var t;
		for (var i = 0; i < x;){
			t = ba.charCodeAt(i++)<<16|ba.charCodeAt(i++)<<8|ba.charCodeAt(i++);
			s.push(B64.$tab.charAt((t>>>18)&0x3f)); 
			s.push(B64.$tab.charAt((t>>>12)&0x3f));
			s.push(B64.$tab.charAt((t>>>6)&0x3f));
			s.push(B64.$tab.charAt(t&0x3f));
		}
		// deal with trailers, based on patch from Peter Wood.
		switch (rm){
			case 2:
				t = ba.charCodeAt(i++)<<16|ba.charCodeAt(i++)<<8;
				s.push(B64.$tab.charAt((t>>>18)&0x3f));
				s.push(B64.$tab.charAt((t>>>12)&0x3f));
				s.push(B64.$tab.charAt((t>>>6)&0x3f));
				s.push(B64.$p);
			break;
			case 1:
				t = ba.charCodeAt(i++)<<16;
				s.push(B64.$tab.charAt((t>>>18)&0x3f));
				s.push(B64.$tab.charAt((t>>>12)&0x3f));
				s.push(B64.$p);
				s.push(B64.$p);
			break;
		}

		return s.join(''); // string
	},

	decode: function(str){
		var s = str.split(''), out = [];
		var l = s.length;
		var tl = 0;
		while(s[--l] == B64.$p){ ++tl; } // strip off trailing padding
		for (var i = 0; i < l;){
			var t = B64.$tab.indexOf(s[i++])<<18;
			if(i <= l) t|=B64.$tab.indexOf(s[i++])<<12;
			if(i <= l) t|=B64.$tab.indexOf(s[i++])<<6;
			if(i <= l) t|=B64.$tab.indexOf(s[i++]);
			out.push(String.fromCharCode((t>>>16)&0xff));
			out.push(String.fromCharCode((t>>>8)&0xff));
			out.push(String.fromCharCode(t&0xff));
		}
		// strip off trailing padding
		while(tl--){ out.pop(); }
		return out.join(''); //  string
	}
});
//Override setInterval to be done outside the frame (there is some issue inside the frame with FF3 and WebKit)
if (!Browser.Engine.trident && !Browser.Engine.presto && !(Browser.Engine.gecko && Browser.Engine.version<=18)) {
	setInterval = function(fn,time) {
		return window.parent.setInterval(fn, time);
	};
	
	setTimeout = function(fn,time) {
		return window.parent.setTimeout(fn, time);
	};
	
	clearInterval = function(id) {
		return window.parent.clearInterval(id);
	};
	
	clearTimeout = function(id) {
		return window.parent.clearTimeout(id);
	};
}
APE.Core = new Class({

	Extends: APE.Core,

	initialize: function(options){
		if (this.getInstance(options.identifier).instance) options.restore = true;

		this.options.sessionVar = ['uniPipe']; 
		this.parent(options);

		//Init and save cookies
		if (options.restore) this.init();

		this.addEvent('uniPipeCreate',this.saveSessionPipe);
		this.addEvent('uniPipeDelete',this.saveSessionPipe);
	},
	
	saveSessionPipe:function(){
		var uniPipe = [];
		this.pipes.each(function(pipe) {
				if (pipe.type == 'uni') {
					uniPipe.push({'casttype':pipe.type, 'pubid':pipe.pipe.pubid, 'properties':pipe.properties});
				}
		});

		if (uniPipe.length > 0) this.setSession({'uniPipe': JSON.encode(uniPipe)});
	},

	restoreUniPipe: function(resp){
		var pipes = JSON.decode(unescape(resp.data.sessions.uniPipe));
		if (pipes) {
			for (var i = 0; i < pipes.length; i++){
				this.newPipe('uni',{'pipe': pipes[i]});
			}
		}
		this.fireEvent('restoreEnd');
		this.restoring = false;
	},

	init: function(){
		this.initCookie();
		this.createCookie();//Create cookie if needed
		this.saveCookie();//Save cookie
	},

	restoreCallback: function(resp){
		if (resp.raw!='ERR' && this.status == 0) { 
			this.fireEvent('init');
			this.status = 1;
		} else if (this.status == 0) {
			this.stopPoller();
		}
	},

	connect: function(options){
		var cookie = this.initCookie();
		if (!cookie) {//No cookie defined start a new connection
			this.parent(options);
			this.addEvent('init',this.init);
		} else {//Cookie or instance exist
			this.restoring = true;
			this.fireEvent('restoreStart');
			this.startPoller();
			this.request.setOptions({'callback': this.restoreCallback.bind(this)});
			this.getSession(this.options.sessionVar, this.restoreUniPipe.bind(this));
		}
	},

	/***
	 * Read the cookie APE_Cookie and try to find the application identifier
	 * @param	String	identifier, can be used to force the identifier to find ortherwhise identifier defined in the options will be used
	 * @return 	Boolean	false if application identifier isn't found or an object with the instance and the cookie
	 */
	getInstance: function(identifier) {
		var	tmp = Cookie.read('MPS');
		identifier = identifier || this.options.identifier;
		if (!tmp) return false;

		tmp = JSON.decode(tmp);
		
		//Cookie is corrupted or doest not contains instance
		if (!tmp || !tmp.instance) return false;
		//Get the instance of ape in cookie
		for(var i = 0; i < tmp.instance.length; i++){
			if(tmp.instance[i] && tmp.instance[i].identifier == identifier){
				return {instance: tmp.instance[i], cookie: tmp};
			}
		}
		
		//No instance found, just return the cookie
		return {cookie: tmp};
	},
	
	removeInstance: function(identifier){
		if (!this.cookie) return;

		for(var i = 0; i < this.cookie.instance.length; i++){
			if(this.cookie.instance[i].identifier == identifier){
				this.cookie.instance.splice(i,1);
				return;
			}
		}
	},

	/***
	 * Initialize cookie and some application variable is instance is found
	 * set this.cookie variable
	 * @return 	boolean	true if instance is found, else false
	 */
	initCookie: function(){
		var tmp = this.getInstance();
		if(tmp && tmp.instance){ //Cookie exist, application instance exist
			this.sessid = tmp.instance.sessid;
			this.pubid = tmp.instance.pubid;
			tmp.cookie.frequency = tmp.cookie.frequency.toInt() + 1;
			this.cookie = tmp.cookie;
			return true;
		} else if (tmp.cookie) { //Cookie exist, no application instance
			this.createInstance(tmp.cookie);
			tmp.cookie.frequency = tmp.cookie.frequency.toInt() + 1;
			this.cookie = tmp.cookie;
			return false;
		} else { //No cookie
			this.cookie = null;
			return false;
		}
	},

	/***
	 * Create a cookie instance (add to the instance array of the cookie the current application)
	 * @param	object	APE_Cookie
	 */
	createInstance: function(cookie) {
		cookie.instance.push({
			identifier: this.options.identifier,
			pubid: this.getPubid(),
			sessid: this.getSessid()
		});
	},

	/***
	 * Create ape cookie if needed (but do not write it)
	 */
	createCookie: function(){
		if(!this.cookie){
			//No Cookie or no ape instance in cookie, lets create the cookie
			var tmp = {
				frequency: 1,
				instance: []
			};
			this.createInstance(tmp);
			this.cookie = tmp;
		}
	},

	saveCookie: function(){
		Cookie.write('MPS', JSON.encode(this.cookie), {'path': '/', domain:this.options.domain});
	},

	clearSession: function(){
		this.parent();
		this.removeInstance(this.options.identifier);
		this.saveCookie();
	},

	removeCookie: function(){
		Cookie.dispose('MPS', {domain:this.options.domain});
	}
});
