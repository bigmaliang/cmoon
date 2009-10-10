$(document).ready(function()
{
	var overlay_append = $("button[rel=#appendoverlay]").overlay(
    {
        api: true,
        closeOnClick: false
    });
    
	var uploader = new AjaxUpload("#imageadder",
	{
		action: '<?cs var: Output.tpl.uri ?>',
		name: 'imagename',
		data: {op: 'add', tp: 'imageadd'},
		responseType: "json",
		autoSubmit: true,
		onChange: replaceImage,
        onComplete: function(file, resp) {
            if (jsonCbkSuc(resp)) {
                $("#previewimg").attr("src", resp.imageurl);
                $("#fileimg").val(resp.imagename);
            }
        }
	});

	function replaceImage(file, ext) {
		if (! (ext && /^(jpg|png|jpeg|gif)$/.test(ext))) {
			alert("请选择图片");
			return false;
		}
		return true;
	}

	$("#addfile").click(function()
	{
        if (!$(".VAL_APPEND").inputval()) {
            return;
        }
        
        $.post("<?cs var: Output.tpl.uri ?>",
               {filedesc: $("#filedesc").val(), fileimg: $("#fileimg").val()},
               function(data) {
                   if (jsonCbksuc(data, {rurl: "<?cs var: Output.tpl.uri ?>"}))
                       overlay_append.close();
               }, "json");
	});
});
