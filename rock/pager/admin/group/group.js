$(document).ready(function()
{
	var heads = {
		uid: "用户ID",
		mode: "组属性",
		status: "状态"
	};
	var opts_table = {
		tbattr: "width='100%' border='0' cellspacing='1' cellpadding='0' class='list'",
		tdattr: "class='td_border' align='center'"
	};

	function addMember(group)
	{
		$("#pid").val(group.id);
	}

	function rendRow(row, obj)
	{
		$(document).mntable(heads, row.groupinfo.members, opts_table).
		appendTo(obj);
	}

	var opt_list = {
		bindTo: "#grouplist",
		title: "uri",
		time: "intime",
		titleClickName: "添加用户",
		titleClickHref: "#memberadd",
		titleClickCbk: addMember,
		content: rendRow
	};

	function showGroup(page)
	{
		$.ajax({
			type: "GET",
			url: "/admin/group",
			cache: false,
			data: "pg="+page,
			dataType: "json",
			success: function(data)	{
				if (type(data.groups) != "Array") {
					if (data.errmsg == "敏感操作, 请先登录") {
						document.loginopts = {
							title: data.errmsg,
							rurl: "admin/group.html"
						};
						$.facebox({ajax: '/member/login.html'});
					} else {
						alert(data.errmsg || "获取组列表失败");
					}
					return;
				}
				if (typeof mygroup != "undefined") {
					mygroup.remove();
				}
				mygroup = $("<div id='grouplist'></div>").appendTo($("#groups"));
				$(document).mnlist(data.groups, opt_list);
				$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showGroup});
			},
			error: function() {
				alert("获取组列表失败");
			}
		});
	}

	showGroup(1);
});
