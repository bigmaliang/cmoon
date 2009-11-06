/***
 * APE JSF Setup
 */
APE.Config.baseUrl = 'http://js.hunantv.com/hn/mps'; //APE JSF 
APE.Config.domain = 'hunantv.com'; //Your domain, must be the same than the domain in aped.conf of your server
APE.Config.server = 'push.hunantv.com'; //APE server URL

(function(){
	for (var i = 0; i < arguments.length; i++)
		APE.Config.scripts.push(APE.Config.baseUrl + '/Source/' + arguments[i] + '.js');
})('mootools-core', 'Core/APE', 'Core/Events', 'Core/Core', 'Pipe/Pipe', 'Pipe/PipeProxy', 'Pipe/PipeMulti', 'Pipe/PipeSingle', 'Request/Request','Request/Request.Stack', 'Request/Request.CycledStack', 'Transport/Transport.longPolling','Transport/Transport.SSE', 'Transport/Transport.XHRStreaming', 'Transport/Transport.JSONP', 'Core/Utility', 'Core/Session');
