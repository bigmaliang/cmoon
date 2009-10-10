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
                if (jsonCbkSuc(data, {rurl: "/admin/group.html"}))
			        optm.callBack(data);
			},
			error: jsonCbkErr
		});
	}

	function addMember(group)
	{
		$("#gid").val(group.gid);
		if (group.amadmin != "1") {
			$("#mode_admin").removeClass("show");
			$("#mode_admin").addClass("hide");
			$("#mode_admin select").val(0);
		} else {
			$("#mode_admin").removeClass("hide");
			$("#mode_admin").addClass("show");
		}
		$("#formgroupadd").FormValidate().ajaxForm(
		{
			success: function(data) {
                if (jsonCbkSuc(data, {rurl: "/admin/group.html"}))
	                overlay_group.close();
            },
			error: jsonCbkErr,
			dataType: 'json',
			validateForm: true,
			timeout: 5000
		});
		overlay_group.load();
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
                if (jsonCbkSuc(data, {errmsg: "获取组列表失败", rurl: "/admin/group.html"})) {
					if (typeof mygroup != "undefined") {
						mygroup.remove();
					}
					mygroup = $(document).mnlist(data.groups, opt_list).appendTo($("#groups"));
					$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showGroup});
	            }
			},
			error: jsonCbkErr
		});
	}

	showGroup(1);
	var overlay_group = $("input[rel=#memberadd]").overlay(
	{
		api:true,
		closeOnClick: false,
		expose: {
			closeOnClick: false
		}
	});
});
