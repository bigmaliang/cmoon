$(document).ready(function()
{
	$("#userlogout").click(function()
	{
		logoutEol();
	});
	$("ul.sf-menu").superfish(
	{
		animation: {height: 'slow'},
		delay: 1200
	});
	sayHi();
	loginCheck();
});
