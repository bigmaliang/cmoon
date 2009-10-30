$(document).ready(function()
{
	function beforeLoginSerial(obj, opts) {
		if ($("#usnlogin").val().length != 0)
			$("#usnlogin").attr("value", $.md5($.md5($("#usnlogin").val())) );
	}
	function errLogin() {
		$("#login").fadeOut("fast");
		$("div.hide").fadeOut("fast");
		$("#unlogged").fadeIn("slow");
	}
	function sucLogin(data) {
		$("#login").fadeOut("fast");
		$("#tiredlogin").fadeOut("fast");
		$("#failurelogin").fadeOut("fast");
		if (data.uname != null && data.success == "1") {
			$("#loggedin").fadeIn("slow", function() {
				$("#unamelogin").text(data.uname);
			});
			$("#loggedin").delay(500, function()
			{
				overlay_login.close();
				jumpToRurl();
				if (typeof document.loginopts == "undefined" ||
					document.loginopts.rurl == "") {
					loginCheck();
				}
			});
		} else if (data.tired != null && data.tired != "0") {
			$("#tiredlogin").fadeIn("slow", function() {
				$("#periodlogin").html(data.during);
			});
		} else {
			$("#failurelogin").fadeIn("slow", function() {
				$("#errmsglogin").html(data.errmsg);
			});
		}
	}

	var opt_login = {
		success: sucLogin,
		dataType: 'json',
		error: errLogin,
		clearForm: true,
		beforeSerialize: beforeLoginSerial,
		validateForm: true,
		timeout: 5000
	};

	var overlay_login = $("a[rel=#loginoverlay]").overlay(
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
			modifyTitle();
			$("#formlogin").FormValidate().ajaxForm(opt_login);
			var uin = $.cookie("uin");
			if (uin != null) {
				$("#uinlg").val(uin);
				$("#usnlogin").focus();
			} else {
				$("#uinlg").focus();
			}
		}
	});
});
