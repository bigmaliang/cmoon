$(document).ready(function()
{
	var opts_table = {
		tbattr: "width='100%' border='0' cellspacing='1' cellpadding='0' " +
		" class='list'",
		tdattr: "class='td_border' align='center'"
	};
	var heads = {
		uid: "用户ID",
		mode: ["权限属性", function(row, col, val, tr)
			   {
				   var smode = "";
				   switch(val) {
				   case "0":
					   smode = "普通组员";
					   break;
				   case "1":
					   smode = "初级会员";
					   break;
				   case "2":
					   smode = "高级会员";
					   break;
				   default:
					   smode = "组管理员";
					   break;
				   }
				   $("<td "+opts_table.tdattr+">"+smode+"</td>").appendTo(tr);
			   }],
		status: ["状态", function(row, col, val, tr)
				 {
					 var sstatus = "";
					 switch(val) {
					 case "0":
						 sstatus = "申请中";
						 break;
					 case "1":
						 sstatus = "被驳回";
						 break;
					 case "2":
						 sstatus = "被邀请";
						 break;
					 case "3":
						 sstatus = "已拒绝";
						 break;
					 default:
						 sstatus = "正常";
						 break;
					 }
					 $("<td "+opts_table.tdattr+">"+sstatus+"</td>").appendTo(tr);
				 }],
		_delete: ["删除", function(row, col, val, tr)
				  {
					  $("<td "+opts_table.tdattr+"><input type='button' class='submitbtn' value='删除' /></td>").appendTo(tr).click(function()
					  {
						  if (confirm("确认删除?")) {
							  manageRow("del", {uid: row[uid], gid: row[gid], callBack: function(){tr.remove();}});
						  }
					  });
				  }]
	};

	function manageRow(op, optm)
	{
		optm = $.extend(
		{
			uid: -1,
			gid: -1,
			callBack: function() {return;}
		}, optm||{});
		$.ajax(
		{
			type: "POST",
			url: "/admin/group",
			cache: false,
			data: "uid="+optm.uid+"&gid="+optm.gid,
			dataType: "json",
			success: function(data) {
				if (data.success != "1") {
					if (data.errmsg == "敏感操作, 请先登录") {
						document.loginopts = {title: data.errmsg, rurl: "/admin/file.html"};
						$.facebox({ajax: '/member/login.html'});
					} else {
						alert(data.errmsg || "操作失败!");
					}
					return;
				}
				alert("操作成功!");
				optm.callBack(data);
			},
			error: function(){alert("操作失败");}
		});
	}

	function sucMemberadd(data)
	{
		if (data.success != "1") {
			alert(data.errmsg || "操作失败, 请稍后再试");
			return;
		}
		alert("操作成功");
		$(document).trigger('close.facebox');
	}

	function errMemberadd() {
		alert("操作失败");
	}

	function addMember(group)
	{
		$("#gid").val(group.gid);
		if (group.amadmin != "1") {
			$(".mode_normal").removeAttr("disabled");
			$(".mode_admin").removeClass("show");
			$(".mode_admin").addClass("hide");
		} else {
			$(".mode_normal").attr("disabled", "disabled");
			$(".mode_admin").removeClass("hide");
			$(".mode_admin").addClass("show");
		}
		$.facebox({div: '#memberadd'});
		$(".infoform").FormValidate().ajaxForm(
		{
			success: sucMemberadd,
			error: errMemberadd,
			dataType: 'json',
			validateForm: true,
			timeout: 5000
		});
	}

	function rendRow(row, obj)
	{
		var content = $(document).mntable(heads, row.groupinfo.members, opts_table);
		content.appendTo(obj);
		return content;
	}

	var opt_list = {
		title: "uri",
		time: "intime",
		titleClickName: "添加用户",
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
				mygroup = $(document).mnlist(data.groups, opt_list).appendTo($("#groups"));
				$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showGroup});
			},
			error: function() {
				alert("获取组列表失败");
			}
		});
	}

	showGroup(1);
});
