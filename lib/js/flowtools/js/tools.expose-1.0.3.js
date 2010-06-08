/**
 * tools.expose 1.0.3 - Make HTML elements stand out
 * 
 * Copyright (c) 2009 Tero Piirainen
 * http://flowplayer.org/tools/expose.html
 *
 * Dual licensed under MIT and GPL 2+ licenses
 * http://www.opensource.org/licenses
 *
 * Launch  : June 2008
 * Date: 2009-06-12 11:02:45 +0000 (Fri, 12 Jun 2009)
 * Revision: 1911 
 */
(function($) { 	

	// static constructs
	$.tools = $.tools || {version: {}};
	
	$.tools.version.expose = '1.0.3';
 	
	function getWidth() {
		
		var w = $(window).width();
		
		if ($.browser.mozilla) { return w; }
		
		var x;
		
		if (window.innerHeight && window.scrollMaxY) {
			x = window.innerWidth + window.scrollMaxX;			
			
		// all but Explorer Mac	
		} else if (document.body.scrollHeight > document.body.offsetHeight) { 
			x = document.body.scrollWidth;
			
		} else {
			x = document.body.offsetWidth;
		}
		
		return x < w ? x + 20 : w;		
	}
	
	function Expose(els, opts) { 
		
		// private variables
		var self = this, mask = null, loaded = false, origIndex = 0;		
		
		// generic binding function
		function bind(name, fn) {
			$(self).bind(name, function(e, args) {
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
		

		// adjust mask size when window is resized (or firebug is toggled)
		$(window).bind("resize.expose", function() {
			if (mask) {
				mask.css({ width: getWidth(), height: $(document).height()});
			}
		}); 
		
		
		// public methods
		$.extend(this, {
		
			getMask: function() {
				return mask;	
			},
			
			getExposed: function() {
				return els;	
			},
			
			getConf: function() {
				return opts;	
			},		
			
			isLoaded: function() {
				return loaded;	
			},
			
			load: function() { 
				
				// already loaded ?
				if (loaded) { return self;	}
	
				origIndex = els.eq(0).css("zIndex"); 				
				
				// find existing mask
				if (opts.maskId) { mask = $("#" + opts.maskId);	}
					
				if (!mask || !mask.length) {
					
					mask = $('<div/>').css({				
						position:'absolute', 
						top:0, 
						left:0,
						width: getWidth(),
						height: $(document).height(),
						display:'none',
						opacity: 0,					 		
						zIndex:opts.zIndex	
					});						
					
					// id
					if (opts.maskId) { mask.attr("id", opts.maskId); }					
					
					$("body").append(mask);	
					
					
					// background color 
					var bg = mask.css("backgroundColor");
					
					if (!bg || bg == 'transparent' || bg == 'rgba(0, 0, 0, 0)') {
						mask.css("backgroundColor", opts.color);	
					}   
					
					// esc button
					if (opts.closeOnEsc) {						
						$(document).bind("keydown.unexpose", function(evt) {							
							if (evt.keyCode == 27) {
								self.close();	
							}		
						});			
					}
					
					// mask click closes
					if (opts.closeOnClick) {
						mask.bind("click.unexpose", function()  {
							self.close();		
						});					
					}					
				}				
				
				// possibility to cancel click action
				var p = {proceed: true};
				$(self).trigger("onBeforeLoad", p);				
				if (!p.proceed) { return self; }
				
				
				// make sure element is positioned absolutely or relatively
				$.each(els, function() {
					var el = $(this);
					if (!/relative|absolute|fixed/i.test(el.css("position"))) {
						el.css("position", "relative");		
					}					
				});
			 
				// make elements sit on top of the mask
				els.css({zIndex:opts.zIndex + 1});				

				
				// reveal mask
				var h = mask.height();
				
				if (!this.isLoaded()) {					
					mask.css({opacity: 0, display: 'block'}).fadeTo(opts.loadSpeed, opts.opacity, function() {
							
						// sometimes IE6 misses the height property on fadeTo method
						if (mask.height() != h) { mask.css("height", h); }						
						$(self).trigger("onLoad");						
					});					
				}
				
				loaded = true;	
				return self;
			}, 
			
			
			close: function() {
				
				if (!loaded) { return self; }   
				
				var p = {proceed: true};
				$(self).trigger("onBeforeClose", p);					
				if (p.proceed === false) { return self; }
				
				mask.fadeOut(opts.closeSpeed, function() {
					$(self).trigger("onClose");
					els.css({zIndex: $.browser.msie ? origIndex : null});
				});        										
				
				loaded = false;
				return self; 
			},
			
			
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

	}
	
	
	// jQuery plugin implementation
	$.fn.expose = function(conf) {
		
		var el = this.eq(typeof conf == 'number' ? conf : 0).data("expose");
		if (el) { return el; }
		
		var opts = {
			/*
			 - onBeforeLoad 
			 - onLoad
			 - onBeforeClose 
			 - onClose 
			*/		

			// mask settings
			maskId: null,
			loadSpeed: 'slow',
			closeSpeed: 'fast',
			closeOnClick: true,
			closeOnEsc: true,
			
			// css settings
			zIndex: 9998,
			opacity: 0.8,
			color: '#456',
			api: false
		};
		
		if (typeof conf == 'string') {
			conf = {color: conf};
		}
		
		$.extend(opts, conf);		

		// construct exposes
		this.each(function() {
			el = new Expose($(this), opts);
			$(this).data("expose", el);	 
		});		
		
		return opts.api ? el: this;		
	};		


})(jQuery);
