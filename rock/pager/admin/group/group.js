$(document).ready(function()
{
	var heads = {
		uid: "用户ID",
		mode: "组属性",
		status: "状态"
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
				mygroup = $("<div></div>").appendTo($("#groups"));
				$.each(data.groups, function(i, group)
				{
					$("<div>"+group.uri+"</div>").appendTo(mygroup);
					$(document).mntable(heads, group.groupinfo.members,
					opts_mntable).appendTo(mygroup);
				});
				$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showGroup});
			},
			error: function() {
				alter("获取组列表失败");
			}
		});
	}

	showGroup(1);
});
