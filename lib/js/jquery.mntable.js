/*
 *    var heads = {
 *        id: "ID",
 *        remark: "名称",
 *        _joinp: ["组用户权限", displayMode],
 *        _delete: ["删除", function(row, col, val, tr) {
 *            var content = $("<input type='button' class='submitbtn' value='删除' />");
 *            content.click(function() {
 *                if (confirm("确认删除?")) {
 *                    var id = $("[name='id']", tr).text();
 *                    manageRow("delete", {id: id, callBack: function(){tr.remove();}});
 *                }
 *            });
 *            return content;
 *        }],
 *        intime: "加入时间"
 *    };
 *
 *  var rows = [{id: 1, remark: '文件', intime: '2009-10-10'}, {id: 2, remark: '组', intime: '2009-10-11'}];
 *
 *    var opts = {
 *        tbattr: 'class="td_border" align="center"',
 *        tdattr: ...,
 *        trCallback: function(tr, row) {
 *            $(tr).attr("id", "mntrow"+row.id);
 *        }
 *    };
 *
 * OR
 *
 * $(document).mntable(['标题', '个数'], o.stdata.url).appendTo(o.sttable);
 */

;(function($){
    $.fn.mntable = function(heads, rows, opts) {
        opts = $.extend({
            tbattr: '',
            tdattr: '',
            trCallback: function(tr, row) {return;}
        }, opts||{});
        var table = $('<table '+opts.tbattr+' ></table>');
        table.rown = 0;
        table.rowspec = [];
        table.thead = $('<thead></thead>').appendTo(table);
        table.tbody = $('<tbody></tbody>').appendTo(table);

        function makeHead(heads) {
            var thr = $('<tr></tr>').appendTo(table.thead);
            $.each(heads, function(i, val) {
                if (bmoon.utl.type(val) == 'String')
                    $('<th>'+val+'</th>').appendTo(thr);
                else if (bmoon.utl.type(val) == 'Array')
                    $('<th>'+val[0]+'</th>').appendTo(thr);
            });
            if (bmoon.utl.type(heads) == 'Object') {
                $.each(heads, function(key, val) {
                    var colspec = key;
                    if (bmoon.utl.type(val) == 'Array') {
                        colspec = [key, val[1]];
                    }
                    table.rowspec.push(colspec);
                });
            }
        }

        function makeSpecRow(row, tr) {
            $.each(table.rowspec, function(i, colspec) {
                if (bmoon.utl.type(colspec) == 'String') {
                    var td = $('<td name='+colspec+' '+opts.tdattr+'></td>');
                    td.html(row[colspec]).appendTo(tr);
                } else if (bmoon.utl.type(colspec) == 'Array') {
                    var td = $('<td name='+colspec[0]+' '+opts.tdattr+'></td>');
                    var res = colspec[1](row, colspec[0], row[colspec[0]], tr, td);
                    if (res) res.appendTo(td);
                    td.appendTo(tr);
                }
            });
        }

        function makeRows(rows) {
            $.each(rows, function(i, row) {
                var tr = $('<tr></tr>').appendTo(table.tbody);
                opts.trCallback(tr, row);
                table.rown++;
                if (table.rowspec.length != 0) {
                    makeSpecRow(row, tr);
                } else {
                    $.each(row, function(i, col) {
                        $('<td '+opts.tdattr+'>'+col+'</td>').appendTo(tr);
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
