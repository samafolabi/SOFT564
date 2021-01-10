var ws;
var subscribed = false;
const IP = "192.168.43.21";
const port = "9001";

$("#subscribe").click(function () {
    if ("WebSocket" in window) {
        subscribed = true;
        ws = new WebSocket("ws://"+IP+":"+port);
        ws.onopen = function () {
            ws.send("SUBUS_");
            message("Subscribed", false);
            document.getElementById("subscribe").disabled = true;
        }
        
        ws.onmessage = function (evt) { 
            var msg = evt.data;
            message(msg, true);
        };
        
        ws.onclose = function() { 
            message("Unsubscribed", false);
            document.getElementById("subscribe").disabled = false;
        };
    } else {
        subscribed = false;
        alert("WebSocket is not supported by your Browser!");
    }
})

$("#ultra").click(function () {
    if (subscribed) {
        message("ULT", false);
        ws.send("ULT");
    }
})

$("#servo").click(function () {
    if (subscribed) {
        message("SRV", false);
        ws.send("SRV"+$("#angle").val()+"_");
    }
})

function message(msg, server) {
    $("#messages").append("<p class='"+(server?"server":"client")+"'>"+msg+"</p>");
}