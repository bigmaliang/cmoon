//regist.js
//=========
	$("#submit").click(function(event) {
		$("#unalert").fadeOut("fast");
		if ($("#uname").val() == "") {
			$("#unalert").fadeIn("slow");
			return;
		}
		$.getJSON("/sc/regist?uname="+$("#uname").val()+"&male="+$("input[name='male']:checked").val(),
			  function(data) {
				  $("#step1").fadeOut("fast");
				  $("#step2").fadeIn("slow");
				  if (data.uin != null) {
					  $("#uin").attr("value", data.uin);
				  } else if (data.tired != null && data.tired != "0") {
					  $("#tired").toggle();
					  $("#period").html(data.during);
				  } else {
					  $("#failure").toggle();
					  $("#errmsg").html(" ("+data.errmsg+") ");
				  }
			  })
	});

//util.js
		//var exp  = new Date();
		//exp.setTime(exp.getTime() + 365*24*60*60*1000);
		//document.cookie = "ClientName" + "="+ encodeURIComponent(cn) + ";expires=" + exp.toGMTString() + ";domain=" + g_site_domain;
