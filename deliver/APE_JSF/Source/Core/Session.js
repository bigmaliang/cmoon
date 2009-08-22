var APE_Core = new Class({

	Extends: APE_Core,

	initialize: function(options){
		if (this.getInstance(options.identifier).instance) options.restore = true;

		this.parent(options);

		//Init and save cookies
		if (options.restore) this.init();

		this.addEvent('pipeCreate',this.savePipeUni);
		this.addEvent('pipeDelete',this.pipeDelete);
		this.session = {
			uniPipe: new $H
		};
	},
	
	saveSessionPipe:function(){
		this.setSession('uniPipe',JSON.encode(this.session.uniPipe.getValues()));
	},

	savePipeUni: function(type, pipe, options) {
		if(type=='uni'){
			this.session.uniPipe.set(pipe.getPubid(), options);
			this.saveSessionPipe();
		}
	},

	pipeDelete: function(type, pipe) {
		this.session.uniPipe.erase(pipe.getPubid());
		this.saveSessionPipe();
	},

	restoreUniPipe: function(resp){
		if(resp.raw=='SESSIONS'){
			var pipes = JSON.decode(unescape(resp.datas.sessions.uniPipe));
			if (pipes) {
				for (var i = 0; i < pipes.length; i++){
					this.newPipe('uni',pipes[i]);
				}
			}
		}
		this.fireEvent('restoreEnd');
		this.restoring = false;
	},

	init: function(){
		this.initCookie();
		this.createCookie();//Create cookie if needed
		this.saveCookie();//Save cookie
	},

	callbackCheck: function(resp){
		if (resp.raw!='ERR' && this.status == 0) { 
			this.fireEvent('init');
			this.status = 1;
			this.getSession('uniPipe',this.restoreUniPipe.bind(this));
		} else {//Check failed, stop the pooler
			this.stopPooler();
		}
	},

	connect:function(options){
		var cookie = this.initCookie();
		if(!cookie){//No cookie defined start a new connection
			this.parent(options);
			this.addEvent('init',this.init);
		}else{//Cookie or instance exist
			this.restoring = true;
			this.fireEvent('restoreStart');
			this.startPooler();
			this.check(this.callbackCheck.bind(this));//Send a check raw (this ask ape for an updated session)
		}
	},

	/***
	 * Read the cookie APE_Cookie and try to find the application identifier
	 * @param	String	identifier, can be used to force the identifier to find ortherwhise identifier defined in the options will be used
	 * @return 	Boolean	false if application identifier isn't found or an object with the instance and the cookie
	 */
	getInstance: function(identifier){
		var	tmp = Cookie.read('APE_Cookie');
		identifier = identifier || this.options.identifier;
		if(tmp){
			tmp = JSON.decode(tmp);
			//Get the instance of ape in cookie
			for(var i = 0; i < tmp.instance.length; i++){
				if(tmp.instance[i].identifier == identifier){
					return {'instance': tmp.instance[i], 'cookie': tmp};
				}
			}
			//No instance found, just return the cookie
			return {'cookie':tmp};
		}
		//no instance found, no cookie found 
		return false;
	},

	/***
	 * Initialize cookie and some application variable is instance is found
	 * set this.cookie variable
	 * @return 	boolean	true if instance is found, else false
	 */
	initCookie: function(){
		var tmp = this.getInstance();
		if(tmp && tmp.instance){ //Cookie exist, application instance exist
			this.setSessid(tmp.instance.sessid);
			this.setPubid(tmp.instance.pubid);
			tmp.cookie.frequency = tmp.cookie.frequency.toInt() + 1;
			this.cookie = tmp.cookie;
			return true;
		} else if (tmp.cookie) { //Cookie exist, no application instance
			this.createInstance(tmp.cookie);
			tmp.cookie.frequency = tmp.cookie.frequency.toInt() + 1;
			this.cookie = tmp.cookie;
			return false;
		} else { //No cookie
			this.cookie = null;
			return false;
		}
	},

	/***
	 * Create a cookie instance (add to the instance array of the cookie the current application)
	 * @param	object	APE_Cookie
	 */
	createInstance: function(cookie) {
		cookie.instance.push({'identifier': this.options.identifier, 'pubid': this.getPubid(), 'sessid': this.getSessid()});
	},

	/***
	 * Create ape cookie if needed (but do not write it)
	 */
	createCookie: function(){
		if(!this.cookie){
			//No Cookie or no ape instance in cookie, lets create the cookie
			tmp = {
				frequency: 1,
				instance: []
			};
			this.createInstance(tmp);
			this.cookie = tmp;
		}
	},

	saveCookie: function(){
		Cookie.write('APE_Cookie', JSON.encode(this.cookie), {domain:this.options.domain});
	},

	clearSession: function(){
		this.parent();
		this.removeCookie();
	},

	removeCookie: function(){
		Cookie.dispose('APE_Cookie', {domain:this.options.domain});
	}
});
