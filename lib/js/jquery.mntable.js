;(function($){
	$.fn.mntable = function(heads, rows, opts) {
		opts = $.extend({
			tbattr: "",
			tdattr: "",
			trCallback: function(tr, row) {return;}
		}, opts||{});
		var table = $("<table "+opts.tbattr+" ></table>");
		table.rown = 0;
		table.rowspec = [];
		table.tbody = $("<tbody></tbody>").appendTo(table);

		function makeHead(heads) {
			var thead = $("<thead></thead>").appendTo(table);
			var thr = $("<tr></tr>").appendTo(thead);
			$.each(heads, function(i, val) {
				if (typeof val == "string")
					$("<th>"+val+"</th>").appendTo(thr);
				else if (typeof val == "object" && val instanceof Array)
					$("<th>"+val[0]+"</th>").appendTo(thr);
			});
			if (typeof heads == "object" && !(heads instanceof Array)) {
				$.each(heads, function(key, val) {
					var colspec = key;
					if (typeof val == "object" && val instanceof Array) {
						colspec = [key, val[1]];
					}
					table.rowspec.push(colspec);
				});
			}
		}

		function makeSpecRow(row, tr) {
			$.each(table.rowspec, function(i, colspec) {
				if (typeof colspec == "string") {
					$("<td name="+colspec+" "+opts.tdattr+">"+row[colspec]+"</td>").
						appendTo(tr);
				} else if (typeof colspec == "object" && colspec instanceof Array) {
					colspec[1](row, colspec[0], row[colspec[0]], tr);
				}
			});
		}

		function makeRows(rows) {
			$.each(rows, function(i, row) {
				var tr = $("<tr></tr>").appendTo(table.tbody);
				opts.trCallback(tr, row);
				table.rown++;
				if (table.rowspec.length != 0) {
					makeSpecRow(row, tr);
				} else {
					$.each(row, function(i, col) {
						$("<td "+opts.tdattr+">"+col+"</td>").appendTo(tr);
					});
				}
			});
		}

		makeHead(heads);
		makeRows(rows);
		table.makeRows = makeRows;
		return table;
	};
})(jQuery);
