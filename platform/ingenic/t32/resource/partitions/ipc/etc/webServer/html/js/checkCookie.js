function checkCookie(){
	var isLogin = getCookie("isLogin");
	if(isLogin!="1"){
		window.location.href="../index.html";
	}
}
function clrCookie() {
	var keys = document.cookie.match(/[^ =;]+(?=\=)/g);
		if(keys) {
			for(var i = keys.length; i--;)
				document.cookie = keys[i] + '=0;expires=' + new Date(0).toUTCString()
		}
}
function getCookie(name){
	var strcookie = document.cookie;
	var arrcookie = strcookie.split("; ");
	for ( var i = 0; i < arrcookie.length; i++) {
		var arr = arrcookie[i].split("=");
		if (arr[0] == name)
			return arr[1];
	}
	return "";
}
checkCookie();
