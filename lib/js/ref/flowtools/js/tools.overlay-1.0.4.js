/**
 * tools.overlay 1.0.4 - Overlay HTML with eye candy.
 * 
 * Copyright (c) 2009 Tero Piirainen
 * http://flowplayer.org/tools/overlay.html
 *
 * Dual licensed under MIT and GPL 2+ licenses
 * http://www.opensource.org/licenses
 *
 * Launch  : March 2008
 * Date: 2009-06-12 11:02:45 +0000 (Fri, 12 Jun 2009)
 * Revision: 1911 
 */
(function($) { 

	// static constructs
	$.tools = $.tools || {version: {}};
	
	$.tools.version.overlay = '1.0.4';
	
	
	var instances = [];		

	
	function Overlay(el, opts) {		
		
		// private variables
		var self = this, w = $(window), closeButton, img, oWidth, oHeight, trigger, bg, exposeApi;
		var expose = opts.expose && $.tools.version.expose;
		
		// generic binding function
		function bind(name, fn) {
			$(self).bind(name, function(e, args)  {
				if (fn && fn.call(this) === false && args) {
					args.proceed = false;	
				}	
			});	 
			return self;
		}
		
		// bind all callbacks from configuration
		$.each(opts, function(name, fn) {
			if ($.isFunction(fn)) { bind(name, fn); }
		});   
		
		
		// get trigger and overlayed element
		var jq = opts.target || el.attr("rel");
		var o = jq ? $(jq) : null;

		if (!o) { o = el; }	
		else { trigger = el; }
		
		
		// external CSS properties are accessible only on window.onLoad (safari / chrome)
		w.load(function() {  
		
			// get growing image
			bg = o.attr("overlay");
		
			// growing image is required (on this version)	
			if (!bg) { 
				bg = o.css("backgroundImage");
				
				if (!bg) { 
					throw "background-image CSS property not set for overlay element: " + jq; 
				}
				
				// url("bg.jpg") --> bg.jpg
				bg = bg.substring(bg.indexOf("(") + 1, bg.indexOf(")")).replace(/\"/g, "");
				o.css("backgroundImage", "none");
				o.attr("overlay", bg);							
			}			
			
			// set initial growing image properties
			oWidth = o.outerWidth({margin:true});
			oHeight = o.outerHeight({margin:true});
			
			// setup growing image
			img = $('<img src="' + bg + '"/>');		
			img.css({border:0,position:'absolute',display:'none'}).width(oWidth).attr("overlay", true);   
			$('body').append(img);   
			
	
			// if trigger is given - assign it's click event
			if (trigger) {
				trigger.bind("click.overlay", function(e) {
					self.load(e.pageY - w.scrollTop(), e.pageX - w.scrollLeft());
					return e.preventDefault();
				});
			}   		
					
			// close button
			opts.close = opts.close || ".close";
			
			if (!o.find(opts.close).length) {				
				o.prepend('<div class="close"></div>');
			} 
			
			closeButton = o.find(opts.close);
			
			closeButton.bind("click.overlay", function() { 
				self.close();  
			});			
			
			// automatic preloading of the image
			if (opts.preload) {
				setTimeout(function() {
					var img = new Image();
					img.src = bg;					
				}, 2000);
			} 
			
		});

				
		// API methods  
		$.extend(self, {

			load: function(top, left) {
				
				// lazy loading if things are not setup yet
				if (!img) { 
					w.load(function()  {
						self.load(top, left);		
					});
					return self;
				}
				
				// one instance visible at once
				if (self.isOpened()) {
					return self;	
				}
				
				if (opts.oneInstance) {
					$.each(instances, function() {
						this.close();
					});
				}
				
				// onBeforeLoad
				var p = {proceed: true};
				$(self).trigger("onBeforeLoad", p);				
				if (!p.proceed) { return self; }						

				// exposing effect
				if (expose) {
					img.expose(opts.expose);
					exposeApi = img.expose().load();
				}				
				
				// start position			
				top = top   || opts.start.top; 					
				left = left || opts.start.left;				
				
				// finish position 
				var toTop = opts.finish.top;
				var toLeft = opts.finish.left;
				
				if (toTop == 'center') { toTop = Math.max((w.height() - oHeight) / 2, 0); }
				if (toLeft == 'center') { toLeft = Math.max((w.width() - oWidth) / 2, 0); }
				
				// adjust positioning relative to scrolling position
				if (!opts.start.absolute)  {
					top += w.scrollTop();
					left += w.scrollLeft();
				}
				
				if (!opts.finish.absolute)  {
					toTop += w.scrollTop();
					toLeft += w.scrollLeft();
				}
				
				// initialize background image  
				img.css({top:top, left:left, width: opts.start.width, zIndex: opts.zIndex}).show();
				
				
				// begin growing
				img.animate({top:toTop, left:toLeft, width: oWidth}, opts.speed, function() { 
		
					// set content on top of the image
					o.css({position:'absolute', top:toTop, left:toLeft}); 
					var z = img.css("zIndex");
					closeButton.add(o).css("zIndex", ++z);
					
					o.fadeIn(opts.fadeInSpeed, function() {  
						$(self).trigger("onLoad"); 	 
					});
					
				});		 
				
				return self; 
			}, 
			
			close: function() {
				
				if (!self.isOpened()) { return self; }
				
				var p = {proceed: true};
				$(self).trigger("onBeforeClose", p);				
				if (!p.proceed) { return self; }
				
				// close exposing effect
				if (exposeApi) { exposeApi.close(); }
				
				if (img.is(":visible")) {
					
					// hide overlayed content
					o.hide();
					
					// calculate triggers position
					var top = opts.start.top; 
					var left = opts.start.left;
					
					if (trigger) {
						p = trigger.offset();
						top = p.top + trigger.height() / 2;
						left = p.left + trigger.width() / 2;
					}					
					
					// shrink image
					img.animate({top: top, left: left, width:0 }, opts.closeSpeed, function()  {
						$(self).trigger("onClose", p);		
					});					
				}
				
				return self;
			},
			
			
			getBackgroundImage: function() {
				return img;	
			},
			
			getContent: function() {
				return o;	
			}, 
			
			getTrigger: function() {
				return trigger;	
			},

			isOpened: function()  {
				return o.is(":visible")	;
			},
			
			// manipulate start, finish and speeds
			getConf: function() {
				return opts;	
			},

			// callback functions
			onBeforeLoad: function(fn) {
				return bind("onBeforeLoad", fn); 		
			},
			
			onLoad: function(fn) {
				return bind("onLoad", fn); 		
			},
			
			onBeforeClose: function(fn) {
				return bind("onBeforeClose", fn); 		
			},
			 
			onClose: function(fn) {
				return bind("onClose", fn); 		
			} 			
			
		});  
				
		
		// keyboard::escape
		$(document).keydown(function(evt) {
			if (evt.keyCode == 27) {				
				self.close();	
			}
		});		

		
		// when window is clicked outside overlay, we close
		if (opts.closeOnClick) {					
			$(document).bind("click.overlay", function(evt) {
				if (!o.is(":visible, :animated")) { return; }
				var target = $(evt.target);
				if (target.attr("overlay")) { return; }
				if (target.parents("[overlay]").length) { return; }
				self.close(); 
			});						
		}		
		
	}
	
	// jQuery plugin initialization
	$.fn.overlay = function(conf) {   
		
		// already constructed --> return API
		var el = this.eq(typeof conf == 'number' ? conf : 0).data("overlay");
		if (el) { return el; }	
		
		var w = $(window);  		
		
		var opts = { 
		
			/* 
			 - onBeforeLoad 
			 - onLoad
			 - onBeforeClose 
			 - onClose 
			*/			
			
			start: {
				// by default: button position || window center
				top: Math.round(w.height() / 2), 
				left: Math.round(w.width() / 2),				
				width: 0,
				absolute: false
			},
			
			finish: {
				top: 80, 
				left: 'center',
				absolute: false
			},   
			
			speed: 'normal',
			fadeInSpeed: 'fast',
			closeSpeed: 'fast',
			
			close: null,	
			oneInstance: true,
			closeOnClick: true, 
			preload: true, 
			zIndex: 9999,
			api: false,
			expose: null,
			
			// target element to be overlayed. by default taken from [rel]
			target: null
		};
		
		if ($.isFunction(conf)) {
			conf = {onBeforeLoad: conf};	
		}
		
		$.extend(true, opts, conf);  
		
		
		this.each(function() {			
			el = new Overlay($(this), opts);
			instances.push(el);
			$(this).data("overlay", el);	
		});
		
		return opts.api ? el: this;		
	}; 
	
})(jQuery);

