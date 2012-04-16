/*
 * jQuery Blink
 * Author: Jordan Thomas
 * https://github.com/jordanthomas/jquery-blink
 */
;jQuery.fn.mnblink = function(o) {
    var d = { speed: 500, blinks: 3, callback: null };
    var o = jQuery.extend(d, o);

    return this.each( function() {
        var calls = 0;
        for (i=1;i<=o.blinks;i++) {
            $(this).animate({
                opacity: 0
            }, o.speed).animate({
                opacity: 1
            }, o.speed, function() {
                calls++;
                if (calls == o.blinks && jQuery.isFunction(o.callback)) { o.callback(); }
            });
        }
    });
};
