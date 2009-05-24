$(document).ready(function() {
	var cmts = [[1, 0, 1, "马亮", "初次评论, 请见谅"],
				[2, 0, 2, "马亮", "第二次, 不同了"],
				[3, 1, 0, "马明", "见谅撒, 严肃点"],
				[4, 3, 0, "马亮", "不会吧, 欺负生人啊"],
				[5, 0, 13, "马明", "无聊, 再发一个"],
				[6, 1, 0, "小左", "打酱油路过"]];
	var opts = {
		bindTo: "#comments",
		replyEnable: true
	};
	if (typeof mycomment == "undefined") {
		mycomment = $(document).mncomment(cmts, opts);
	}

	$("a[rel=facebox]").facebox();
});
