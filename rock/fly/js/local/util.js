var g_site_domain = "eol.com";

function sayHi()
{
	var cn = $.cookie("ClientName");
	if (cn == null) {
		cn = randomWord(8);
		$.cookie("ClientName", cn, {expires: 365, path: '/', domain: g_site_domain});
	}
}

function loginCheck()
{
	var uname = $.cookie("uname");
	var uin = $.cookie("uin");
	if (uname != null && uin != null) {
		$("#nav-guest").hide();
		$("#unameLayout").text(uname);
		$("#uinLayout").text(uin);
		$("#userActions").mnsuperfish({
			ajax:true,
			url:"/service/action",
			data:"uin="+uin,
			dataType: "json"});
		$("#nav-member").show();
	} else {
		$("#nav-member").hide();
		$("#userActions").hide();
		$("#nav-guest").show();
	}
}
function logoutEol()
{
	$.cookie("uname", null, {path: '/', domain: g_site_domain});
	$.cookie("usn", null, {path: '/', domain: g_site_domain});
	$.cookie("musn", null, {path: '/', domain: g_site_domain});
	loginCheck();
	$.ajax({
		type: "GET",
		url: "/member/logout",
		cache: false
	});
}
