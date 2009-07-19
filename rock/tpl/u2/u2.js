$(document).ready(function()
{
	$("button[rel=#fileappend]").overlay();
	$("#imageadder").fileUpload(
	{
		uploader: "/swf/uploader.swf",
		script: ""
	});
});
