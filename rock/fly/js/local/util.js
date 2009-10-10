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

function modifyTitle()
{
	var lopt = document.loginopts;
	if (type(lopt) == "Object") {
		if (lopt.title != "") {
    		$("#titlelg").text(lopt.title);
			lopt.title = "登录 EOL";
		}
		if (lopt.rurl != "") {
    		$("#rurllg").text(lopt.rurl);
		}
	}
}

function jumpToRurl()
{
	var lopt = document.loginopts;
	if (type(lopt) == "Object") {
		if (type(lopt.rurl) == "String" && lopt.rurl != "") {
			window.location.href = lopt.rurl;
			lopt.rurl = "";
		}
	}
}

function jsonCbkSuc(data, opts)
{
    opts = $.extend(
    {
        suctip: true,
        errmsg: "操作失败",
        rurl: "/index.html"
    }, opts||{});
    
	if (data.success != "1") {
		if (data.errmsg == "敏感操作, 请先登录") {
			document.loginopts = {title: data.errmsg, rurl: opts.rurl};
			overlay_login.load();
		} else {
			alert(data.errmsg || opts.errmsg);
		}
		return false;
	}

    if (opts.suctip)
        alert("操作成功!");
    return true;
}

function jsonCbkErr(XMLHttpRequest, textStatus, errorThrown)
{
    alert(textStatus || "操作失败!");
}
