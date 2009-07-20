$(document).ready(function()
{
	$("button[rel=#fileappend]").overlay();
	var uploader = new AjaxUpload("#imageadder",
	{
		action: '/csc',
		name: 'imagename',
		data: {op: 'add', tp: 'imageadd'},
		autoSubmit: true
	});
});
