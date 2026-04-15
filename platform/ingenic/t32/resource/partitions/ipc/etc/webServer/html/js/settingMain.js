layui.use(['util', 'laydate', 'layer','element'], function(){
		var util = layui.util
		,$ = layui.jquery
		,layer = layui.layer
		,element = layui.element;
		var videoinit_id = 99;
		var videoinit_title = "主视频页面";
		var videoinit_url = "videoMainSetting.html";
		element.tabAdd('tabs',{
			title : videoinit_title,
			content : '<iframe scrolling="no" frameborder="0"  src="'+videoinit_url+'" style="width:100%;height:100%;"></iframe>',
			id: videoinit_id
		});
		element.tabChange('tabs',videoinit_id);
		$(".layui-tab ul").children('li').first().children('.layui-tab-close').css("display",'none');
		$("#hideHeader").hide();
		util.fixbar({
			bar1: '&#x1006'
			,css: {right: 80, bottom: 80}
			,bgcolor: '#393D49'
			,click: function(type){
				if(type === 'bar1'){
					layer.msg('是否退出当前账户？', {
						time: 10000,                    //10s后自动关闭
						area: ['220px', '110px'],       //弹窗大小
						btn: ['是', '否'],              //弹窗按钮
						btnAlign: 'c',                 //按钮居中
						yes: function(index, layero){  //第一个按钮事件
							location.href = '../index.html';       //页面跳转
						}
					});
				}
			}
		});
	});
	$('.site-demo-active').on('click', function(){
		var othis = $(this), type = othis.data('type');
		active[type] ? active[type].call(this, othis) : '';
	});
	layui.use('element', function(){
		var element = layui.element
		element.on('nav(test)',function(elem){
			var options = eval('('+elem.context.dataset.options+')');
			var title = options.title;
			var url = options.url;
			var id = options.id;
			//获取当前id在这个tabs里面的长度
			var li = $("li[lay-id="+id+"]").length;
			var video_id = 99;
			var li_video = $("li[lay-id="+video_id+"]").length;
			//如果大于0证明tabs里面已经有这个标签卡了，切换,否则添加
			if(li>0)
				element.tabChange('tabs',id);
			else{
				element.tabAdd('tabs',{
					title : title,
					content : '<iframe scrolling="no" frameborder="0"  src="'+url+'" style="width:100%;height:960px;"></iframe>',
					id: id
				});
				element.tabChange('tabs',id);
			}
			if(id!=video_id && li_video>0)
				element.tabDelete("tabs",video_id);
		});
	});
	function showTime(){
		nowtime = new Date();
		year = nowtime.getFullYear();
		month = nowtime.getMonth() + 1;
		date = nowtime.getDate();
		document.getElementById("myTime").innerHTML = year + "年" + month + "月" + date + "日" + " " + nowtime.toLocaleTimeString();
	}
	setInterval("showTime()",1000);
