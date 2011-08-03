/*
    12/3/08
    Form Validator
    Jquery plugin for form validation and quick contact forms
    Copyright (C) 2008 Jeremy Fry

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

jQuery.iFormValidate = {
    build : function()
    {
        return $(this).each(
            function() {
            $inputs = $(this).find(":input").filter(":not(:submit)").filter(":not(:checkbox)");
            //catch the submit
            $inputs.bind("keyup", jQuery.iFormValidate.validate);
            $inputs.filter("select").bind("change", jQuery.iFormValidate.validate);
        });
    },
    validateForm : function()
    {
        $inputs = $(this).find(":input").filter(":not(:submit)").filter(":not(:checkbox)");
        var isValid = true; //benifit of the doubt?
        $inputs.filter(".is_required").each(jQuery.iFormValidate.validate);
        if($inputs.filter(".is_required").hasClass("invalid")){isValid=false;}
        return isValid;
    },

    validate : function(){
        var $val = $(this).val();
        var isValid = true;
        //Regex for DATE
        if($(this).hasClass('vdate')){
            var Regex = /^([\d]|1[0,1,2]|0[1-9])(\-|\/|\.)([0-9]|[0,1,2][0-9]|3[0,1])(\-|\/|\.)\d{4}$/;
            isValid = Regex.test($val);
        //Regex for Email
        }else if($(this).hasClass('vemail')){
            var Regex =/^([a-zA-Z0-9_\.\-\+])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;
            if(!Regex.test($val)){isValid = false;}
        //Regex for Phone
        }else if($(this).hasClass('vphone')){
            var Regex = /^\(?[2-9]\d{2}[ \-\)] ?\d{3}[\- ]?\d{4}$/;
            if(!Regex.test($val)){isValid = false;}
        //Check for U.S. 5 digit zip code
        }else if($(this).hasClass('vzip')){
            var Regex = /^\d{5}$/;
            if(!Regex.test($val)){isValid = false;}
        //Check for state
        }else if($(this).hasClass('vstate')){
            var Regex = /^[a-zA-Z]{2}$/;
            if(!Regex.test($val)){isValid = false;}
        //Check for name
        }else if($(this).hasClass('vname')){
            var Regex = /^[a-zA-Z\ ']*$/;
            if(!Regex.test($val)){isValid = false;}
        //Check for int number
        }else if($(this).hasClass('vint')){
            var Regex = /^\d{1,10}$/;
            if(!Regex.test($val)){isValid = false;}
        //Check for max length
        }else if($(this).attr('maxlen') != undefined){
            if( $val.length > parseInt($(this).attr('maxlen')) ||
                ($(this).hasClass('is_required') && $val.length === 0) ){isValid = false;}
        //Check for min length
        }else if($(this).attr('minlen') != undefined){
            if( $val.length > parseInt($(this).attr('minlen')) ||
                ($(this).hasClass('is_required') && $val.length === 0) ){isValid = false;}
        //Check for not empty empty
        }else if($val.length === 0){
            isValid = false;
        }

        if(isValid){
            $(this).removeClass("invalid");
            $(this).addClass("valid");
        }else{
            $(this).removeClass("valid");
            $(this).addClass("invalid");
        }
    }
};
jQuery.fn.FormValidate = jQuery.iFormValidate.build;
jQuery.fn.FormValidateOnsubmit = jQuery.iFormValidate.validateForm;
