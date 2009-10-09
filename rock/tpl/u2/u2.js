$(document).ready(function()
{
	$("button[rel=#fileappend]").overlay({closeOnClick: false});
	$("#addfile").click(function()
	{
        if (!$(".VAL_APPEND").inputval()) {
            return;
        }
        var data = {
            filedesc: $("#filedesc").val()
        };
        if ($("#fileimg").val().length != 0) {
            data.fileimg = $("#fileimg").val();
        }
        
        $.post("/csc", data, successAdd, "json");
	});

    function successAdd(data) {
        // TODO continue here
        alert("添加成功");
    }
	
	var uploader = new AjaxUpload("#imageadder",
	{
		action: '/csc',
		name: 'imagename',
		data: {op: 'add', tp: 'imageadd'},
		responseType: "json",
		autoSubmit: true,
		onChange: replaceImage
	});

	function replaceImage(file, ext) {
		if (! (ext && /^(jpg|png|jpeg|gif)$/.test(ext))) {
			alert("请选择图片");
			return false;
		}
		return true;
	}
});
