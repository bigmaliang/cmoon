$(document).ready(function()
{
	$("button[rel=#fileappend]").overlay();
	$("#imageadder").fileUpload(
	{
		uploader: "/js/pub/ref/uploadify/uploader.swf",
		script: "/csc",
		auto: true,
		cancelImg: '/js/pub/ref/uploadify/cancel.png',
		scriptData: {tp: 'imageadd'}
	});
});
