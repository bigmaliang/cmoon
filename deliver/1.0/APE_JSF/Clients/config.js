/***
 * APE JSF Setup
 */
APE.Config.baseUrl = 'http://js.hunantv.com/hn/mps'; //APE JSF 
APE.Config.domain = 'hunantv.com'; //Your domain, must be the same than the domain in aped.conf of your server
APE.Config.server = 'push.hunantv.com'; //APE server URL

(function(){
	for (var i = 0; i < arguments.length; i++)
		APE.Config.scripts.push(APE.Config.baseUrl + '/Build/' + arguments[i] + '.js');
})('mpsCore');
