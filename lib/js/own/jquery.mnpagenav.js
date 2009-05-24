;(function($){
	$.fn.mnpagenav = function(opts) {
		opts = $.extend({
			numperpage: 15,
			pg: 1,
			ttnum: 0,
			callback: function(pg) {return true;}
		}, opts||{});

		var currpage = 1;
		if (typeof document.pagenav_pg != "undefined") {
			currpage = parseInt(document.pagenav_pg);
		} else {
			currpage = opts.pg;
		}
		var tmppg = parseInt(opts.ttnum)+parseInt(opts.numperpage)-1;
		var totalpage = parseInt(tmppg / opts.numperpage);

		$(this).each(function(i, obj)
		{
			var wrapper = obj;
			$(wrapper).empty();
			//$("<span>共 "+totalpage+" 页</span>").appendTo(wrapper);
			if (currpage != 1) {
				$("<a href='javascript:void(0);'>上一页</a>").appendTo(wrapper).click(function() {
					document.pagenav_pg = currpage-1;
					opts.callback(currpage-1);
				});
			}

			$("<span>第 "+currpage+"/"+totalpage+" 页</span>").appendTo(wrapper);

			if (currpage != totalpage && totalpage != 0) {
				$("<a href='javascript:void(0);'>下一页</a>").appendTo(wrapper).click(function() {
					document.pagenav_pg = currpage+1;
					opts.callback(currpage+1);
				});
			}

			$("<span>转到第 <input type='text' id='pagenav_goto' class='short bordless' /> 页</span>").appendTo(wrapper);
			$("<span><a href='javascript:void(0);'>确定</a></span>").appendTo(wrapper).click(function() {
				var pg = $("#pagenav_goto").val();
				var regd = /^[0-9]+$/;
				if (!regd.test(pg)) {
					$("#pagenav_goto").addClass("invalid");
					$("#pagenav_goto").removeClass("valie");
					return;
				}
				if (pg < 1 || pg > totalpage) {
					$("#pagenav_goto").addClass("invalid");
					$("#pagenav_goto").removeClass("valie");
					return;
				}
				$("#pagenav_goto").addClass("valid");
				$("#pagenav_goto").removeClass("invalie");
				document.pagenav_pg = pg;
				opts.callback(pg);
			});
		});
	};
})(jQuery);
