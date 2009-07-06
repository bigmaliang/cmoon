$(document).ready(function()
{
	var heads = {
		uin: "用户ID",
		uname: "分配名",
		status: "帐号类型",
		uptime: "操作日期"
	};

	function showAccount(page)
	{
		$.ajax({
			type: "GET",
			url: "/admin/account",
			cache: false,
			data: "pg="+page,
			dataType: "json",
			success: function(data)	{
				if (type(data.accounts) != "Array") {
					if (data.errmsg == "敏感操作, 请先登录") {
						document.loginopts = {
							title: data.errmsg,
							rurl: "admin/account.html"
						};
						$.facebox({ajax: '/member/login.html'});
					} else {
						alert(data.errmsg || "获取组列表失败");
					}
					return;
				}
				if (typeof myaccount != "undefined") {
					myaccount.remove();
				}
				myaccount = $(document).mntable(heads, data.accounts,
				opts_mntable).appendTo($("#accounts"));
				$("#pagenav").mnpagenav({ttnum: data.ttnum, callback: showAccount});
			},
			error: function() {
				alter("获取组列表失败");
			}
		});
	}

	showAccount(1);

	function beforeAllocSerial(obj, opts) {
		if ($("#usnalloc").val().length != 0)
			$("#usnalloc").attr("value", $.md5($.md5($("#usnalloc").val())) );
	}
	function errAlloc() {
		$("#step1").fadeOut("fast");
		$("#step2").fadeOut("fast");
		$("#tiredalloc").fadeOut("fast");
		$("#failurealloc").fadeIn("slow");
	}
	function sucAlloc(data) {
		$("#step1").fadeOut("fast");
		$("#step2").fadeOut("fast");
		$("div.hide").fadeOut("fast");
		if (data.success == "1" && data.uin != null) {
			$("#step2").fadeIn("slow", function() {
				$("#accountalloc").text(data.uin);
			});
			loginCheck();
		} else if (data.tired != null && data.tired != "0") {
			$("#tiredalloc").fadeIn("slow", function() {
				$("#periodalloc").html(data.during);
			});
		} else {
			$("#failurealloc").fadeIn("slow", function() {
				$("#errmsgalloc").html(data.errmsg);
			});
		}
	}

	var opt_alloc = {
		dataType: 'json',
		error: errAlloc,
		clearForm: true,
		validateForm: true,
		success: sucAlloc,
		beforeSerialize: beforeAllocSerial,
		timeout: 5000
	};
	$("#formalloc").FormValidate().ajaxForm(opt_alloc);
});

