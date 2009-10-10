$(document).ready(function()
{
	var opts_mntable = {
		tbattr: "width='100%' border='0' cellspacing='1' cellpadding='0' class='list'",
		tdattr: "class='td_border' align='center'",
		trCallback: function(tr, row) {
			$(tr).attr("id", "mntrow"+row.id);
		}
	};
	var heads = {
		id: "ID",
		pid: "父ID",
		uri: "uri",
		remark: "名称",
		uname: "创建者",
		//_allp: ["所有用户权限", displayMode],
		//_memberp: ["注册用户权限", displayMode],
		//_joinp: ["组用户权限", displayMode],
		intime: "加入时间",
		_delete: ["删除", function(row, col, val, tr) {
			$("<td "+opts_mntable.tdattr+"><input type='button' class='submitbtn' value='删除' /></td>").appendTo(tr).click(function() {
				if (confirm("确认删除?")) {
					var id = $("[name='id']", tr).text();
					manageRow("delete", {id: id, callBack: function(){tr.remove();}});
				}
			});
		}]
	};
	function displayMode(row, col, mode, tr) {
		var id = $("[name='id']", tr).text();
		var optm = {id: id, area: col};
		function checkB(file) {
			if ((mode & file) != 0)
				return "<input type='checkbox' checked='checked' />";
			else
				return "<input type='checkbox' />";
		}
		var td = $("<td "+opts_mntable.tdattr+"></td>").appendTo(tr);
		$(checkB(0x01)).appendTo(td).click(function() {
			optm.unit = 1;
			optm.enable = $(this).attr("checked");
			manageRow("modify", optm);
		});
		$(checkB(0x02)).appendTo(td).click(function() {
			optm.unit = 2;
			optm.enable = $(this).attr("checked");
			manageRow("modify", optm);
		});
		$(checkB(0x04)).appendTo(td).click(function() {
			optm.unit = 4;
			optm.enable = $(this).attr("checked");
			manageRow("modify", optm);
		});
		$(checkB(0x08)).appendTo(td).click(function() {
			optm.unit = 8;
			optm.enable = $(this).attr("checked");
			manageRow("modify", optm);
		});
	}
	function manageRow(op, optm) {
		var ptype = "POST";
		var tmpop = "";
		optm = $.extend({
			id: "",
			area: "",
			unit: "",
			enable: false,
			callBack: function() {return true;}
		}, optm||{});
		if (op == "delete") {
			//ptype = "DELETE";
			ptype = "POST";
			tmpop = "del";
		} else if (op == "modify") {
			ptype = "POST";
		} else if (op == "add") {
			ptype = "PUT";
		}
		$.ajax({
			type: ptype,
			url: "/admin/file",
			cache: false,
			data: "id="+optm.id+"&area="+optm.area+"&unit="+optm.unit+"&enable="+optm.enable+"&op="+tmpop,
			dataType: "json",
			success: function(data, textStatus) {
                if (jsonCbkSuc(data, {rurl: "/admin/file.html"}))
			        optm.callBack(data);
			},
			error: jsonCbkErr
		});
	}
	function showFile(page) {
		$.ajax({
			type: "GET",
			url: "/admin/file",
			cache: false,
			data: "pg="+page+"&pid="+$("#querypid").val(),
			dataType: "json",
			success: function(data, textStatus) {
                if (jsonCbkSuc(data, {errmsg: "获取文件列表失败!", rurl: "/admin/file.html"})) {
					if (typeof myfile != "undefined") {
						myfile.remove();
					}
					myfile = $(document).mntable(heads, data.files, opts_mntable).appendTo($("#files"));
					$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showFile});
                }
			},
			error: jsonCbkErr
		});
	}

	showFile(1);
	$("#requery").click(function()
	{
		showFile(1);
	});


	function beforeFileaddSerial() {
		var mode = parseInt($("#modetype").val());
		$(".ckmode", "#formfileadd").each(function(i, obj) {
			if ($(obj).attr("checked") == true) {
				mode = mode | 1<<(i+4);
			}
		});
		$("#addmode").val(mode);
	}
	var opt_fileadd = {
		success: function (data) {
            if (jsonCbkSuc(data, {rurl: "/admin/file.html"})) {
				myfile.makeRows(data.files);
				$("#mntrow"+data.files[0].id).seekAttention({
					pulseSpeed: 800
				});
				overlay_file.close();
            }
        },
		dataType: 'json',
		error: jsonCbkErr,
		//clearForm: true,
		beforeSerialize: beforeFileaddSerial,
		validateForm: true,
		timeout: 5000
	};

	var overlay_file = $("a[rel=#fileoverlay]").overlay(
	{
		api: true,
		closeOnClick: false,
		onBeforeLoad: function() {
			var wrap = this.getContent().find("div.wrap");
			if (wrap.is(":empty")) {
				wrap.load(this.getTrigger().attr("href"));
			}
		},
		onLoad: function() {
			$("#formfileadd").FormValidate().ajaxForm(opt_fileadd);
			$("#remark").focus();
		}
	});
});
