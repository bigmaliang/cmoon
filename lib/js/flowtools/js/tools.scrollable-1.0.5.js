/**
 * jquery.scrollable 1.0.5 - Scroll your HTML with eye candy.
 * 
 * Copyright (c) 2009 Tero Piirainen
 * http://flowplayer.org/tools/scrollable.html
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
	
	$.tools.version.scrollable = '1.0.5';
				
	var current = null;		

	
	// constructor
	function Scrollable(root, conf) {   

		// current instance
		var self = this;  
		if (!current) { current = self; }		
		
		// generic binding function
		function bind(name, fn) {
			$(self).bind(name, function(e, args)  {
				if (fn && fn.call(this, args.index) === false && args) {
					args.proceed = false;	
				}	
			});	
			
			return self;
		}
		
		// bind all callbacks from configuration
		$.each(conf, function(name, fn) {
			if ($.isFunction(fn)) { bind(name, fn); }
		});   
		
		
		// horizontal flag
		var horizontal = !conf.vertical;				
		
		// wrap (root elements for items)
		var wrap = $(conf.items, root);				
		
		// current index
		var index = 0;		
		
		function find(query, ctx) {
			return query.indexOf("#") != -1 ? $(query).eq(0) : ctx.siblings(query).eq(0);	
		}		
		
		// get handle to navigational elements
		var navi = find(conf.navi, root);
		var prev = find(conf.prev, root);
		var next = find(conf.next, root);
		var prevPage = find(conf.prevPage, root);
		var nextPage = find(conf.nextPage, root);
		
		
		// methods
		$.extend(self, {
			
			getIndex: function() {
				return index;	
			},
	
			getConf: function() {
				return conf;	
			},
			
			getSize: function() {
				return self.getItems().size();	
			},
	
			getPageAmount: function() {
				return Math.ceil(this.getSize() / conf.size); 	
			},
			
			getPageIndex: function() {
				return Math.ceil(index / conf.size);	
			},

			getRoot: function() {
				return root;	
			},
			
			getItemWrap: function() {
				return wrap;	
			},
			
			getItems: function() {
				return wrap.children();	
			},
			
			getVisibleItems: function() {
				return self.getItems().slice(index, index + conf.size);	
			},
			
			/* all seeking functions depend on this */		
			seekTo: function(i, time, fn) {
				
				// default speed
				if (time === undefined) { time = conf.speed; }
				
				// function given as second argument
				if ($.isFunction(time)) {
					fn = time;
					time = conf.speed;
				}
								
				if (i < 0) { i = 0; }				
				if (i > self.getSize() - conf.size) { return self; } 				

				var item = self.getItems().eq(i);					
				if (!item.length) { return self; }				
				
				
				// onBeforeSeek
				var p = {index: i, proceed: true};
				$(self).trigger("onBeforeSeek", p);				
				if (!p.proceed) { return self; }
									
				
				if (horizontal) {
 					var left = -item.position().left;					
					wrap.animate({left: left}, time, conf.easing, fn ? function() { fn.call(self); } : null);
					
				} else {
					var top = -item.position().top;										
					wrap.animate({top: top}, time, conf.easing, fn ? function() { fn.call(self); } : null);							
				}	
				
				
				// navi status update
				if (navi.length) {
					var klass = conf.activeClass;
					var page = Math.ceil(i / conf.size);
					page = Math.min(page, navi.children().length - 1);
					navi.children().removeClass(klass).eq(page).addClass(klass);
				} 
				
				// prev buttons disabled flag
				if (i === 0) {
					prev.add(prevPage).addClass(conf.disabledClass);					
				} else {
					prev.add(prevPage).removeClass(conf.disabledClass);
				}
								
				// next buttons disabled flag
				if (i >= self.getSize() - conf.size) {
					next.add(nextPage).addClass(conf.disabledClass);
				} else {
					next.add(nextPage).removeClass(conf.disabledClass);
				}				
				
				current = self;
				index = i;				
				
				// onSeek after index being updated
				$(self).trigger("onSeek", {index: i});				
				return self; 
			},			
			
				
			move: function(offset, time, fn) {
				var to = index + offset;
				if (conf.loop && to > (self.getSize() - conf.size)) {
					to = 0;	
				}
				return this.seekTo(to, time, fn);
			},
			
			next: function(time, fn) {
				return this.move(1, time, fn);	
			},
			
			prev: function(time, fn) {
				return this.move(-1, time, fn);	
			},
			
			movePage: function(offset, time, fn) {
				return this.move(conf.size * offset, time, fn);		
			},
			
			setPage: function(page, time, fn) {
				var size = conf.size;
				var index = size * page;
				var lastPage = index + size >= this.getSize(); 
				if (lastPage) {
					index = this.getSize() - conf.size;
				}
				return this.seekTo(index, time, fn);
			},
			
			prevPage: function(time, fn) {
				return this.setPage(this.getPageIndex() - 1, time, fn);
			},  
	
			nextPage: function(time, fn) {
				return this.setPage(this.getPageIndex() + 1, time, fn);
			}, 
			
			begin: function(time, fn) {
				return this.seekTo(0, time, fn);	
			},
			
			end: function(time, fn) {
				return this.seekTo(this.getSize() - conf.size, time, fn);	
			},
			
			reload: function() {
				return load();	
			},
			
			click: function(index, time, fn) {
				
				var item = self.getItems().eq(index);
				var klass = conf.activeClass;			
				
				// check that index is sane
				if (index < 0 || index >= this.getSize()) { return self; }
					
				
				// special case with two items
				if (conf.size == 2) {
					if (index == self.getIndex()) { index--; }
					self.getItems().removeClass(klass);
					item.addClass(klass);					
					return this.seekTo(index, time, fn);
				}
				

				if (!item.hasClass(klass)) {				
					self.getItems().removeClass(klass);
					item.addClass(klass);
					var delta = Math.floor(conf.size / 2);
					var to = index - delta;

					// next to last item must work
					if (to > self.getSize() - conf.size) { 
						to = self.getSize() - conf.size; 
					}
					
					if (to !== index) {
						return this.seekTo(to, time, fn);		
					}				 
				}
				
				return self;
			},

			// callback functions
			onBeforeSeek: function(fn) {
				return bind("onBeforeSeek", fn); 		
			},
			
			onSeek: function(fn) {
				return bind("onSeek", fn); 		
			}
			
		});
	
		
		// mousewheel
		if ($.isFunction($.fn.mousewheel)) { 
			root.bind("mousewheel.scrollable", function(e, delta)  {
				// opera goes to opposite direction
				var step = $.browser.opera ? 1 : -1;
				
				self.move(delta > 0 ? step : -step, 50);
				return false;
			});
		}  
		
		// prev button		
		prev.addClass(conf.disabledClass).click(function() { 
			self.prev(); 
		});
		

		// next button
		next.click(function() { 
			self.next(); 
		});
		
		// prev page button
		nextPage.click(function() { 
			self.nextPage(); 
		});
		

		// next page button
		prevPage.addClass(conf.disabledClass).click(function() { 
			self.prevPage(); 
		});		

		
		// keyboard
		if (conf.keyboard) {			

			// keyboard works on one instance at the time. thus we need to unbind first
			$(document).unbind("keydown.scrollable").bind("keydown.scrollable", function(evt) {
				
				var el = current;	
				if (!el || evt.altKey || evt.ctrlKey) { return; }
					
				if (horizontal && (evt.keyCode == 37 || evt.keyCode == 39)) {					
					el.move(evt.keyCode == 37 ? -1 : 1);
					return evt.preventDefault();
				}	
				
				if (!horizontal && (evt.keyCode == 38 || evt.keyCode == 40)) {
					el.move(evt.keyCode == 38 ? -1 : 1);
					return evt.preventDefault();
				}
				
				return true;
				
			});	 
		}

		// navi 			
		function load() {			
	
			// generate new entries
			if (navi.is(":empty") || navi.data("me") == self) {
				
				navi.empty();
				navi.data("me", self);
				
				for (var i = 0; i < self.getPageAmount(); i++) {		
					
					var item = $("<" + conf.naviItem + "/>").attr("href", i).click(function(e) {							
						var el = $(this);
						el.parent().children().removeClass(conf.activeClass);
						el.addClass(conf.activeClass);
						self.setPage(el.attr("href"));
						return e.preventDefault();
					});
					
					if (i === 0) { item.addClass(conf.activeClass); }
					navi.append(item);					
				}
				
			// assign onClick events to existing entries
			} else {
				
				// find a entries first -> syntaxically correct
				var els = navi.children(); 
				
				els.each(function(i)  {
					var item = $(this);
					item.attr("href", i);
					if (i === 0) { item.addClass(conf.activeClass); }
					
					item.click(function() {
						navi.find("." + conf.activeClass).removeClass(conf.activeClass);
						item.addClass(conf.activeClass);
						self.setPage(item.attr("href"));
					});
					
				});
			}
			
			
			// item.click()
			if (conf.clickable) {
				self.getItems().each(function(index, arg) {
					var el = $(this);
					if (!el.data("set")) {
						el.bind("click.scrollable", function() {
							self.click(index);		
						});
						el.data("set", true);
					}
				});				
			}
			
			
			// hover
			if (conf.hoverClass) {
				self.getItems().hover(function()  {
					$(this).addClass(conf.hoverClass);		
				}, function() {
					$(this).removeClass(conf.hoverClass);	
				});
			}			
			
			return self;
		}
		
		load();
		
		
		// interval stuff
		var timer = null;

		function setTimer() {

			// do not start additional timer if already exists
			if (timer) { return; }
			
			// construct new timer
			timer = setInterval(function()  {
					
				// check if interval is being changed dynamically during runtime
				if (conf.interval === 0) {					
					clearInterval(timer);
					timer = 0;
					return;
				}			
				
				self.next();				
			}, conf.interval);
		}	
		
		if (conf.interval > 0) {			
			
			// when mouse enters, autoscroll stops
			root.hover(function() {			
				clearInterval(timer);		
				timer = 0;
				
			}, function() {		
				setTimer();	
			});
			
			setTimer();	
		}
		
	} 

		
	// jQuery plugin implementation
	$.fn.scrollable = function(conf) { 
			
		// already constructed --> return API
		var el = this.eq(typeof conf == 'number' ? conf : 0).data("scrollable");
		if (el) { return el; }		
		
 
		var opts = {
			
			// basics
			size: 5,
			vertical:false,			
			clickable: true,
			loop: false,
			interval: 0,			
			speed: 400,
			keyboard: true,			
			
			// other
			activeClass:'active',
			disabledClass: 'disabled',
			hoverClass: null,			
			easing: 'swing',
			
			// navigational elements
			items: '.items',
			prev: '.prev',
			next: '.next',
			prevPage: '.prevPage',
			nextPage: '.nextPage',			
			navi: '.navi',
			naviItem: 'a',
			api:false,

			
			// callbacks
			onBeforeSeek: null,
			onSeek: null
			
		}; 
		
		
		$.extend(opts, conf);		
		
		this.each(function() {			
			el = new Scrollable($(this), opts);
			$(this).data("scrollable", el);	
		});
		
		return opts.api ? el: this; 
		
	};
			
	
})(jQuery);
