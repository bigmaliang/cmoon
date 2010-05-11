$(document).ready(function() {
	$("#submit").click(
	function() {
		if (!$(".VAL_NEWAPP").inputval()) {
			return;
		}

		var
		aname = $("#appname").val(),
		email = $("#email").val(),
		usn = $("#usn").val();

		$.getJSON("/app/new", {aname: aname, email: email, usn: usn},
		function(data){
			if (data.success != 1) {
				alert(data.errmsg || "操作失败， 请稍后再试");
			}
			// TODO
		});
	});
});
