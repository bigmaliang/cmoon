    // create our new shiney APE client
    var client = new APE.Client;

    // set to false to disable debugging
    var debug = true;

    // load the APE client
    client.load({
        'domain': APE.Config.domain,
        'server': APE.Config.server,
        'identifier': 'jquery',
        'channel': 'test',
        'complete': function(ape){
            // APE has finished loading so now we can load our scripts

            // colorChanger(ape [obj], debug [bool]);
            new colorChanger(ape, debug).initialize();

            if(debug)
                $("#debug").append("<span><strong>APE has finished loading</strong></span><br />");
        },
        'scripts': APE.Config.scripts //Scripts to load for APE JSF
    });







        
function colorChanger(ape, debug){
    // we call this function once APE has finished loading
    this.initialize = function(){
        // once a new user joins (pipe created), call setup()
        ape.addEvent('pipeCreate', this.setup);

        // when a user joins, update the user list
        ape.addEvent('userJoin', this.createUser);

        // when a user leaves, destroy them with mighty thunder!
        ape.addEvent('userLeft', this.deleteUser);

        // when we want to send data
        ape.onCmd('send', this.cmdSend);

        // and when we recieve data
        ape.onRaw('data', this.rawData);

        // start the session with a random name!
        // note: you'll need the chat plugin loaded
        ape.start(String((new Date()).getTime()).replace(/\D/gi,''));
    };

    this.setup = function(type, pipe, options){
        // add an event listener on our selectbox
        $("select[name=selectColor]").change(function(){
            // get the select box value
            color = $("option:selected", this).val();

            // set the background of the document to the color chosen
            $("body").css("background-color", color);

            // send the new color to the APE server
            pipe.send(color);
        });
    };

    this.cmdSend = function(pipe, sessid, pubid, message){
        if(debug)
            $("<span>    " + ape.user.properties.name + " changed the bg color to " + message + "</span><br />").prependTo("#debug");
    };

    this.rawData = function(raw, pipe){
        // data has been received by the APE server so do the following...
        if(debug)
            $("<span>    " + raw.datas.sender.properties.name + " changed the bg color to " + raw.datas.msg + "</span><br />").prependTo("#debug");

        // set the selectboxes value to match other clients
        $("select[name=selectColor]").val(raw.datas.msg);

        // set the background color
        $("body").css("background-color", raw.datas.msg);
    };

    this.createUser = function(user, pipe){
        // a user has joined so prepend them to the debug window
        user.element = $("<span>" + user.properties.name + " has joined bgColor</span><br />").prepend("<img src='bullet_green.png' />").prependTo("#debug");
    };

    this.deleteUser = function(user, pipe){
        // a user has left so update the debug window
        $(user.element).text(user.properties.name + " has left bgColor").css("color", "#666666").prepend("<img src='bullet_red.png' />");
    };
}