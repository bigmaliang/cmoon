;(function($) {
    $.fn.mntips = function(rows, opts) {
        opts = $.extend({
			// 'top', 'bottom', 'right', 'left', 'center'
			position: ['bottom', 'center'], 
			offset: [0, 0],
			relative: false,

            tipClass: 'mntip',

            rowspec: null,      // ['id',...]

            clickRow: function(row, evt) {return false;}
        }, opts||{});

	    /* calculate tip position relative to the trigger */  	
	    function getPosition(trigger, tip) {
		    // get origin top/left position 
		    var conf = opts,
            top = conf.relative ? trigger.position().top : trigger.offset().top, 
			left = conf.relative ? trigger.position().left : trigger.offset().left,
			pos = conf.position[0];

		    top  -= tip.outerHeight() - conf.offset[0];
		    left += trigger.outerWidth() + conf.offset[1];
		    
		    // iPad position fix
		    if (/iPad/i.test(navigator.userAgent)) {
			    top -= $(window).scrollTop();
		    }
		    
		    // adjust Y		
		    var height = tip.outerHeight() + trigger.outerHeight();
		    if (pos == 'center') 	{ top += height / 2; }
		    if (pos == 'bottom') 	{ top += height; }
		    
		    
		    // adjust X
		    pos = conf.position[1]; 	
		    var width = tip.outerWidth() + trigger.outerWidth();
		    if (pos == 'center') 	{ left -= width / 2; }
		    if (pos == 'left')   	{ left -= width; }	 
		    
		    return {top: top, left: left};
	    }		

        $(this).each(function() {
            var me = $(this),
            tipdiv = $('<ul class="'+opts.tipClass+'">');

            if (me.data('mntip')) {
                 me.data('mntip').remove();
            }
            
            $.each(rows, function(i, row) {
                var item = $("<li>").appendTo(tipdiv).click(function(evt) {
                    tipdiv.hide();
                    opts.clickRow(row, evt);
                });
                if (bmoon.utl.type(opts.rowspec) == 'Array') {
                    $.each(opts.rowspec, function(i, col) {
                        $("<span class='tipscol"+i+"'>"+row[col]+"</span>").appendTo(item);
                    });
                } else {
                    $('<span>'+row+'</span>').appendTo(item);
                }
            });

            tipdiv.appendTo(document.body).hide();

            var pos = getPosition(me, tipdiv);

            tipdiv.css({position: 'absolute', left: pos.left, top: pos.top}).show();

            me.data('mntip', tipdiv);
        });
    };
})(jQuery);
