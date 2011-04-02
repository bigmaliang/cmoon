/*
 * o.nav.mnnav({ntt: mgd.ntt, npp: mgd.npp, npg: mgd.npg});
 */
;(function($){

	function gotoPage(pn, tpg, cbk, url) {
		var o = $('#_nav-inp');
		pn = parseInt(pn);

		if (!pn || pn < 0 || pn > tpg) {
			o.addClass('invalid')
			return;
		}
		o.removeClass('invalid');
		document._cpg = pn;
		o.val(pn);
		if (url.length) {
			var regu = /\?/;
			if (regu.test(url)) url = url + '&npg=' + pn;
			else url = url + '?npg=' + pn;
			window.location = url;
		} else cbk(pn);
	}

	function navLeft(cpg, tpg, cbk, url) {
		var html = '<span class="left">第 <input type="text" id="_nav-inp" value="'+
			cpg+'" />'+' <span class="confirm">&nbsp;</span> 页/共 ' +
			tpg + ' 页</span>',
		r = $(html),
		i = $('#_nav-inp', r),
		c = $('.confirm', r);

		i.bind('keydown', 'return', function(){
			gotoPage(i.val(), tpg, cbk, url);
		});

		c.click(function() {
			gotoPage(i.val(), tpg, cbk, url);
		});
		
		return r;
	}

	function navCenter(tpg, cbk, url) {
		var html = [
			'<span class="center">',
			  '<a href="javascript:void(0);" rel="1">首页</a> ',
			  '<a href="javascript:void(0);" rel="', tpg, '">末页</a>',
			'</span>'
		].join(''),
		r = $(html),
		a = $('a', r);

		a.click(function() {
			gotoPage($(this).attr('rel'), tpg, cbk, url);
		});
		
		return r;
	}

	function navRight(cpg, tpg, cbk, url) {
		var html = '<span class="right">';
		if (cpg > 1) html += '<span class="nav-prev" rel="'+(parseInt(cpg)-1)+'">&nbsp;</span> ';
		if (cpg < tpg) html += '<span class="nav-next" rel="'+(parseInt(cpg)+1)+'">&nbsp;</span>';
		html += '</span>';

		var r = $(html),
		a = $('span', r);

		a.click(function() {
			gotoPage($(this).attr('rel'), tpg, cbk, url);
		});
		
		return r;
	}
	
	$.fn.mnnav = function(opts) {
		opts = $.extend({
			npp: 15,
			npg: 1,
			ntt: 0,
			url: "",			// (url) means no ajax mode, need offer pg meanwhile
			cbk: function(pg) {alert("goto page " + pg); return true;}
		}, opts||{});

		opts.npp = parseInt(opts.npp);
		opts.npg = parseInt(opts.npg);
		opts.ntt = parseInt(opts.ntt);

		var cpg = bmoon.utl.type(document._cpg) == 'Number'? document._cpg: opts.npg,
		tpg = parseInt((opts.ntt+opts.npp-1) / opts.npp);

		$(this).each(function(k, obj){
			var o = $(obj);

			o.empty();

			navLeft(cpg, tpg, opts.cbk, opts.url).appendTo(o);
			navCenter(tpg, opts.cbk, opts.url).appendTo(o);
			navRight(cpg, tpg, opts.cbk, opts.url).appendTo(o);
		});
	};
})(jQuery);
