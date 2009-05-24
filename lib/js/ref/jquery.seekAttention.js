/**
 * jQuery.seekAttention
 * Copyright (c) 2008 James Padolsey - jp(at)qd9(dot)co.uk | http://enhance.qd-creative.co.uk
 * Dual licensed under MIT and GPL.
 * Date: 17/09/08
 *
 * @projectDescription Focus user's attention towards elements
 * http://enhance.qd-creative.co.uk/demo/seekAttention/
 * Tested with jQuery 1.2.6. On FF 2/3, IE 6/7, Opera 9.5 and Safari 3. on Windows XP.
 *
 * @author James Padolsey
 * @version 1.0
 *
 * @id jQuery.seekAttention
 * @id jQuery.fn.seekAttention
 */
jQuery.fn.seekAttention = function(o){
    /* Default options: */
    var defaultOptions = {
        color: 'black',
        opacity: 0.55,
        fade: true,
        fadeSpeed: 400,
        hideOnClick: true,
        hideOnHover: true,
        pulse: true,
        blur: true,
        pulseSpeed: 400,
        paddingTop: 0,
        paddingRight: 0,
        paddingBottom: 0,
        paddingLeft: 0,
        container: 'body'
    };
    // Checking that correct data type has been passed for each option:
    for (var i in o) {
        if(i!=='container') {
            if(o[i]&&o[i]!=='undefined') {
                if(typeof o[i] !== typeof defaultOptions[i]) {return;}
            }   
        } else {
            if(typeof o[i] !== 'object' && typeof o[i] !== 'string' ) {
                return;
            }
        }
    }
    var options = $.extend(defaultOptions,o);
    /* Make sure there are no remains of the last attention seeker: */
    $('.sa-overlay,.sa-pulse-overlay,div[class*="sa-blur"]').remove();
    function getHorPadding(elem) {
        if(elem===document) {return 0;}
        /* Get computed left and right padding of an element: */
        return parseInt($(elem).css('paddingLeft').replace('px',''),10) + parseInt($(elem).css('paddingRight').replace('px',''),10);
    }
    function getVerPadding(elem) {
        if(elem===document) {return 0;}
        /* Get computed top and bottom padding of an element: */
        return parseInt($(elem).css('paddingTop').replace('px',''),10) + parseInt($(elem).css('paddingBottom').replace('px',''),10);
    }
    function pixelateBorders(elem) {
        if(elem===document) {return 0;}
        /* A function to pixelate all border widths defined in words (thin/medium/thick): */
        var blw = 'borderLeftWidth';
        var brw = 'borderRightWidth';
        var btw = 'borderTopWidth';
        var bbw = 'borderBottomWidth';
        function makeNumerical(prop) {
            if($(elem).css(prop).indexOf('i')>-1) {
                if($(elem).css(prop)==='thin') {$(elem).css(prop,'2px');}
                if($(elem).css(prop)==='medium') {$(elem).css(prop,'4px');}
                if($(elem).css(prop)==='thick') {$(elem).css(prop,'6px');}
            }
        }
        makeNumerical(blw);
        makeNumerical(brw);
        makeNumerical(btw);
        makeNumerical(bbw);
    }
    function getHorBorder(elem) {
        if(elem===document) {return 0;}
        /* Get computed left and right border widths: */
        var horBorder = 0;
        var borderLeftWidth = $(elem).css('borderLeftWidth');
        var borderRightWidth = $(elem).css('borderRightWidth');
        if($(elem).css('borderLeftStyle')!=='none') {    
            horBorder += parseInt(borderLeftWidth.replace('px',''),10);
        }
        if($(elem).css('borderRightStyle')!=='none') {    
            horBorder += parseInt(borderRightWidth.replace('px',''),10);
        }
        return horBorder;
    }
    function getVerBorder(elem) {
        if(elem===document) {return 0;}
        /* Get computed top and bottom border widths: */
        var verBorder = 0;
        var borderTopWidth = $(elem).css('borderTopWidth');
        var borderBottomWidth = $(elem).css('borderBottomWidth');
        if($(elem).css('borderTopStyle')!=='none') {    
            verBorder += parseInt(borderTopWidth.replace('px',''),10);
        }
        if($(elem).css('borderBottomStyle')!=='none') {    
            verBorder += parseInt(borderBottomWidth.replace('px',''),10);
        }
        return verBorder;
    }
    function newBox(t,l,h,w,c) {
        /* Create new div and assign style props and a class then append to body: */
        $('<div/>').css({
            top: t+'px',
            left: l+'px',
            position: 'absolute',
            zIndex: 9999,
            height: h+'px',
            width: w+'px',
            background: options.color,
            opacity: 0,
            display: 'none'
        }).addClass(c||'sa-overlay').appendTo('body').show();
    }
    $(this).each(function(i){
        if(i>0) {return;}
        
        pixelateBorders(this);
        var containerIsDocument = (options.container==='body'||options.container===document) ? true : false;
        
        /* Retrieve all dimensions needed: */
        var container = (containerIsDocument) ? document : $(options.container);
        var containerHeight = $(container).height() + getVerPadding(container) + getVerBorder(container);
        var containerWidth = $(container).width() + getHorPadding(container) + getHorBorder(container);
        var containerTop = (containerIsDocument) ? 0 : $(container).offset().top;
        var containerLeft = (containerIsDocument) ? 0 : $(container).offset().left;
        var width = $(this).width() + getHorPadding(this) + getHorBorder(this) + options.paddingLeft + options.paddingRight;
        var height = $(this).height() + getVerPadding(this) + getVerBorder(this) + options.paddingTop + options.paddingBottom;
        var left = $(this).offset().left - options.paddingLeft;
        var top = $(this).offset().top - options.paddingTop;
        var right = containerWidth - (width+left);
        var bottom = containerHeight - (height+top);
        
        /* If the target element is not positioned within the "container" element then STOP: */
        if(left<containerLeft||top<containerTop||left>containerLeft+containerWidth||top>containerTop+containerHeight) {return;}
        
        /* Just encase the above assignments result in an invalid negative value: */
        right = (right.toString().indexOf('-')>-1) ? 0 : right;
        bottom = (bottom.toString().indexOf('-')>-1) ? 0 : bottom;
        top = (top.toString().indexOf('-')>-1) ? 0 : top;
        left = (left.toString().indexOf('-')>-1) ? 0 : left;
        
        /* We need five new boxes including the pulsing box */
        newBox(containerTop,containerLeft,containerHeight,left-containerLeft);
        newBox(containerTop,containerLeft+(left-containerLeft),top-containerTop,width);
        newBox(containerTop,(left-containerLeft)+containerLeft+width,containerHeight,containerWidth-(left-containerLeft)-width);
        newBox((top-containerTop)+containerTop+height,left,containerHeight-((top-containerTop))-height,width);
        /* The pulse box is needed regardless of whether you've set options.pulse to true/false: */
        newBox(top,left,height,width,'sa-pulse-overlay');
        
        var blur = ($.browser.msie&&$.browser.version<=6) ? false : options.blur;
        if(blur) {
            /* We need 4 shadow/blur levels to make it look nice ;-) */
            /* Obvously there will only be a blur if it is set to true in options */
            $([1,2,3,4]).each(function(i){
                newBox(top+i,left+i,height-(i*2),1,'sa-blur'+i);
                newBox(top+i,left+1+i,1,width-(2+(2*i)),'sa-blur'+i);
                newBox(top+i,left+width-(i+1),height-(2*i),1,'sa-blur'+i);
                newBox(top+height-(i+1),left+1+i,1,width-(2+(2*i)),'sa-blur'+i);
                $('.sa-blur'+i).css({
                    backgroundColor: options.color,
                    opacity: 0
                }).attr('id','sa-blur'+(options.opacity/2-(i/8)).toString());
            });  
        }
        var hasScrolled = false;
        var scrolled = (document.documentElement.scrollTop ? document.documentElement.scrollTop : document.body.scrollTop);
        var viewportHeight = (parseInt($(window).height(),10)>window.innerHeight) ? window.innerHeight : parseInt($(window).height(),10);
        // Tests if target element is visible, if it's not then the page will scroll to it.
        if(top>(scrolled+viewportHeight)||top+height>(scrolled+viewportHeight)||top<scrolled) {
            var where = top-Math.round(viewportHeight/2);
            /* Smoothly scrolls page to position of target element: */
            $('html, body').animate({ scrollTop: where }, 1000);
            hasScrolled = true;
        }
        /* setTimeout - no point in starting the fading too soon */
        setTimeout(function(){
            if(options.fade) {
                /* Fade in the overlay */
                $('.sa-overlay').animate({opacity:options.opacity},options.fadeSpeed);
                $('div[class*="sa-blur"]').each(function(){
                    /* Get required opacity (in ID attribute) and animate to that opacity */
                    $(this).animate({opacity:$(this).attr('id').substr(7)});
                });
                /* Attach the click/hover events once above fading has occured */
                setTimeout(function(){attachEvents();},options.fadeSpeed);
            } else {
                $('.sa-overlay').css({opacity:options.opacity});
                $('div[class*="sa-blur"]').each(function(){
                    /* Get required opacity (in ID attribute) and apply that opacity */
                    $(this).css({opacity:$(this).attr('id').substr(7)});
                });
                /* Wait 100 millisecs and then attach hover/click events (to avoid dblclick fury) */
                setTimeout(function(){attachEvents();},200);
            }
            function pulseElement() {
                $('.sa-pulse-overlay').animate({opacity:options.opacity/2},options.pulseSpeed,function(){
                    $(this).animate({opacity:0},options.pulseSpeed,function(){
                        pulseElement();
                    });
                });
            }
            if(options.pulse) {
                pulseElement();
            }
        },(hasScrolled) ? 1000 : 500);
        function attachEvents() {
            /* Attach the click 'hide' event to the entire overlay and the hover 'hide' event to the element overlay (pulse) */
            if(options.hideOnHover) {
                $('.sa-pulse-overlay').mouseover(function(){
                    $('.sa-pulse-overlay').remove();
                    $('div[class*="sa-blur"],.sa-overlay,').fadeOut(400,function(){
                         $('div[class*="sa-blur"],.sa-overlay,').remove();
                    });
                });
            }
            if(options.hideOnClick) {
                $('.sa-overlay,.sa-pulse-overlay').click(function(){
                   $('.sa-overlay,.sa-pulse-overlay').remove();
                   $('div[class*="sa-blur"]').remove();
                });
            }   
        }
    });
};