// script.js


// 获取所有带有 'caret' 类的元素
const carets = document.querySelectorAll(".caret");

// 为每个 caret 元素添加点击事件监听器
carets.forEach(caret => {
    caret.addEventListener("click", () => {
        // 切换 'caret-down' 类以改变箭头方向
        caret.classList.toggle("caret-down");

        // 获取当前 caret 元素的下一个兄弟元素（即子节点列表）
        const nested = caret.nextElementSibling;

        // 如果子节点列表存在，则切换其显示状态
        if (nested && nested.classList.contains("nested")) {
            nested.style.display = nested.style.display === "block" ? "none" : "block";
        }
    });
});

// 获取所有输入框
const inputs = document.querySelectorAll(".node-input");

// 为每个输入框添加事件监听器
inputs.forEach(input => {
    input.addEventListener("blur", () => {
        // 当输入框失去焦点时，可以在这里处理输入框中的数据
        console.log(`Input value: ${input.value}`);
    });

    input.addEventListener("keydown", (event) => {
        // 当按下回车键时，模拟失去焦点
        if (event.key === "Enter") {
            input.blur();
        }
    });
});


// 获取提交按钮
// const submitBtn = document.getElementById("submitBtn");

// // 为提交按钮添加点击事件监听器
// submitBtn.addEventListener("click", () => {
//     // 创建一个对象来存储所有输入框的数据
//     const data = [];

//     // 获取所有输入框
//     const inputs = document.querySelectorAll(".node-input");
//     // 遍历所有输入框，收集数据
//     inputs.forEach(input => {
//         // const nodeName = input.getAttribute("data-node"); // 获取节点名称
//         const inputValue = input.value; // 获取输入框中的值
//         data.push(inputValue); // 将节点名称和输入框值存入对象
//         console.log(inputValue);
//     });

//     // 将数据转换为 JSON 格式
//     const jsonData = JSON.stringify(data);

//     // 打印 JSON 数据到控制台（用于调试）
//     // console.log(jsonData);
//     var info= {
//         "configlist":{
//             "video.isp.awbAttr":"video.isp.awbAttr",
//         },
//         "video.isp.awbAttr":{
//             "awb_mode":jsonData,
//         },
//     }
//     console.log(info)
//     $.ajax({
//         type:"POST",

//         url:'../cgi-bin/webTransData.cgi',
//         ContentType:"application/json;charset=utf-8",
//         dataType:"json",
//         data:JSON.stringify(info),
//         success:function(data){
//             alert("Success");
//         },error:function(data){
//             alert("Error");
//         }
//     });
// });