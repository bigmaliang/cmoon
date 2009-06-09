$(document).ready(function() {
	function beforeRegSerial(obj, opts) {
		if ($("#usnreg").val().length != 0)
			$("#usnreg").attr("value", $.md5($.md5($("#usnreg").val())) );
	}
	function errRegist() {
		$("#step1").fadeOut("fast");
		$("#step2").fadeOut("fast");
		$("#tiredreg").fadeOut("fast");
		$("#failurereg").fadeIn("slow");
	}
	function sucRegist(data) {
		$("#step1").fadeOut("fast");
		$("#step2").fadeOut("fast");
		$("div.hide").fadeOut("fast");
		if (data.success == "1" && data.uin != null) {
			$("#step2").fadeIn("slow", function() {
				$("#accountreg").text(data.uin);
			});
			loginCheck();
		} else if (data.tired != null && data.tired != "0") {
			$("#tiredreg").fadeIn("slow", function() {
				$("#periodreg").html(data.during);
			});
		} else {
			$("#failurereg").fadeIn("slow", function() {
				$("#errmsgreg").html(data.errmsg);
			});
		}
	}

	var opt_regist = {
		dataType: 'json',
		error: errRegist,
		clearForm: true,
		validateForm: true,
		success: sucRegist,
		beforeSerialize: beforeRegSerial,
		timeout: 5000
	};
	$("#formregist").FormValidate().ajaxForm(opt_regist);
});

