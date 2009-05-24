$(document).ready(function() {
	var opts_add = {
		dataType: "json",
		success: sucCmtAdd,
		error: errCmtAdd,
		clearForm: true,
//		flowerCover: "#comments",
		validateForm: true,
		timeout: 5000
	};
	$("form").FormValidate().ajaxForm(opts_add);
	function sucCmtAdd(data) {
		if (data.success == "1") {
			mycomment.makeComment(data.comment);
			$("#comment"+data.comment[0].id).seekAttention({
				pulseSpeed: 800
			});
		} else {
			alert(data.errmsg || "发布失败, 请稍后再试!");
		}
		$(document).trigger('close.facebox');
		return true;
	}
	function errCmtAdd() {
		alert("操作失败!");
	}

	var opts_cmt = {
		bindTo: "#comments",
		replyEnable: true
	};
	function showComment(pgsn) {
		$.ajax({
			type: "GET",
			url: "/cgi-bin/comment",
			data: "op=show&pgsn="+pgsn,
			dataType: "json",
			success: function(data) {
				if (type(data.comment) != "Array") {
					//alert(data.errmsg || "获取留言失败!");
					return false;
				}
				if (typeof mycomment == "undefined") {
					mycomment = $(document).mncomment(data.comment, opts_cmt);
				}
				return true;
			},
			error: function() {
				alert("获取留言失败");
			}
		});
	}

	$("a[rel=facebox]").facebox({opacity: 0.4});
	$(document).bind('reveal.facebox', function() {
		$("form").FormValidate().ajaxForm(opts_add);
		$("#pid").val(0);
		$("#zid").val(0);
	});
	//showComment(1);
});
