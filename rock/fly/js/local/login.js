$(document).ready(function() {
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
			$("#loggedin").delay(500, function() {
				$(document).trigger('close.facebox');
				loginCheck();
			});
			if (type(data.rurl) == "String" && data.rurl != "") {
				window.location.href = data.rurl;
			}
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

	$("a[rel=facebox]").facebox({opacity: 0.4, closeImage: "/js/pub/ref/facebox/closelabeld.jpg"});
	$(document).bind("reveal.facebox", function() {
		$("#formlogin").FormValidate().ajaxForm(opt_login);
		var uin = $.cookie("uin");
		if (uin != null) {
			$("#uinlg").val(uin);
			$("#usnlogin").focus();
		} else {
			$("#uinlg").focus();
		}
	});
});

function modifyLogin(opts)
{
	if (opts.title) {
		$("#titlelg").html(opts.title);
	}
	if (opts.rurl) {
		$("#rurllg").text(opts.rurl);
	}
};
