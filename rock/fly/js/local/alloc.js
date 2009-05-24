$(document).ready(function() {
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

