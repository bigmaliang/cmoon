/**
 * 社区消息推送类
 */
function Push(ape, debug) {
    this.initialize = function(opts) {
	    ape.uin = opts.uin || 0;
	    ape.friendPipeName = "*friendpipe"+ opts.uin || "";
	    ape.isDesktop = opts.isDesktop;
        ape.friendPipe = null;
        ape.loading = 1;
    
        ape.onRaw("data", this.rawData);
        ape.onRaw("regclass", this.rawClass);
        ape.addEvent("multiPipeCreate", this.pipeCreate);
        
		ape.addEvent('load', this.start);
    };
    
    this.start = function() {
        ape.start({"uin": ape.uin + ""});
    };
    
    //get current pipe
	this.pipeCreate = function(pipe, options) {
	    if (pipe.properties.name.toLowerCase() == ape.friendPipeName) {
            ape.friendPipe = pipe;
            
            //接收哪些消息块
            var classes = "";
            if (ape.isDesktop)
                classes = "1x2x3";
            else
                classes="1x2";
            ape.friendPipe.request.send("REGCLASS", {'apps': classes}, true);
        } else if(pipe.properties.name.toLowerCase().search(/friendpipe/)) {
            if(ape.loading) return;
            
            var userid=pipe.properties.name;
            userid=userid.split('*friendpipe');
            $.getJSON("http://home.hunantv.com/home/home/getUserById",
                      {userid:userid[1]},
                      friendOnline);
        }
    };

    this.rawClass = function(raw) {
        ape.loading = 0; //APE初始化结束
    };

    //show message of receive
	this.rawData = function(raw, pipe) {
        //var msg_JSON = eval('('+msg_str+')'); //string to JSON
        var msg_JSON = raw.data.msg;
        if(msg_JSON.pageclass == 1) {//站内信提示
            var message_num = $("#message_num");
            var msgCount = message_num.html();
            message_num.addClass("new");
            message_num.html(++msgCount);
        } else if(msg_JSON.pageclass == 2) { //通知提示
            var notice_num = $("#notice_num");
            var noticeCount = notice_num.html();
            notice_num.addClass("new");
            notice_num.html(++noticeCount);
        } else if(msg_JSON.pageclass==3 && ape.isDesktop) { //新鲜事提示 
            $.getJSON(
                "http://home.hunantv.com/home/home/getEventById/?r="+Math.random(),
                {id:msg_JSON.content},
                function($data){
                    if (!$data.err){
                        $("#event-box-list").prepend($data.msg);
                        var new_event = $("#event-box-list div:first");
                        new_event.hide();
                        new_event.show(1000);
                    }
	            }
            );
        } else if(msg_JSON.pageclass!=3) { //错误 
            alert("class error:"+msg_JSON.pageclass);    
        }
	};

    this.feedSend = function(t, m) {
        var d = "{pageclass:3, type:"+t+", content:\""+m+"\"}";
        ape.friendPipe.send(d);
    };

    this.noticeSend = function(c, m, u) {
        var d = escape("{pageclass:"+c+", content:\""+m+"\"}");
        ape.friendPipe.request.send("SENDUNIQ", {dstuin: u, msg: d});
    };

    this.getUserList = function() {
        ape.friendPipe.request.send("USERLIST", null, true);
    };

    this.getFriendList = function() {
        ape.friendPipe.request.send("FRIENDLIST", null, true);
    };
    
    function friendOnline($data) {
		if (!$data.err){
		    hnLoad(['http://js.hunantv.com/hn/dialogbox.source.1.0.js'],
            function() {
	            var img_avatar = '<img height="48" width="48" ' +
                'src="http://avatar.hunantv.com/' +
                parseInt(userid[1])%255+'/'+userid[1]+'_48_48.jpg?r=' +
                Math.random()+'"/>';
                
	            $.dialogBox(
                '<a href="http://home.hunantv.com/home/profile/index/' +
                userid[1]+'">'+img_avatar +
                '</a> <a href="http://home.hunantv.com/home/profile/index/' +
                userid[1]+'">'+$data.msg+'</a><a href="javascript:"> 上线啦！</a>',
                {wait:10000});
		    });
		} else {
		    alert($data.msg);
		}
    }
}
