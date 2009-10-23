function showStatus(status)
{
	$("#status").html("<strong>"+$("#status").val()+"<br />"+status+"</strong>");
}

function Push (ape, debug) {
	this.initialize = function() {
		ape.onCmd('send', this.cmdSend);
		ape.onRaw('data', this.rawData);
		ape.addEvent('pipeCreate', this.pipeCreate);

        $("#submit").click(function() {
			ape.start($("#uin").val());
        });

        $("#button_send").click(function()
		{
			//this.pipe.send($("#message").val());
			ape.getPipe(ape.getPubid()).send($("#message").val());
			//ape.pushPipe.send($("#message").val());
        });

        $("#button_left").click(function() {
            ape.quit();
        });
	};

	this.cmdSend = function(pipe, sessid, pubid, message) {
		showStatus("cmdSend");
		$("#msg_send").html("<strong>Message of send:</strong>"+message);
	};

	this.rawData = function(raw, pipe) {
		showStatus("rawData");
		//var msg = unescape(raw.datas.pipe.properties.name);
		var msg = unescape(raw.datas.msg);
		$("#msg_receiver").html("<strong>Message of receive:</strong>"+msg);
	};

	this.pipeCreate = function(type, pipe, options) {
		showStatus("Joined Channel: " + pipe.getPubid());
		ape.setPubid(pipe.getPubid());
	};
}
