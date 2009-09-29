$(document).ready(function()
{
	$("button[rel=#fileappend]").overlay({closeOnClick: false});
	var uploader = new AjaxUpload("#imageadder",
	{
		action: '/csc',
		name: 'imagename',
		data: {op: 'add', tp: 'imageadd'},
        responseType: "json",
		autoSubmit: false,
        onChange: replaceImage
	});

    function replaceImage() {
        alert("change");
    }
});
