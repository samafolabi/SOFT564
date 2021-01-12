var ws; //WebSocket
var subscribed = false;

//Socket settings
const IP = "192.168.43.21";
const port = "9001";

$("#subscribe").click(function () {
    //If WebSockets are supported, create a new one to buggy
    if ("WebSocket" in window) {
        subscribed = true;
        ws = new WebSocket("ws://"+IP+":"+port);

        //Once the connection is opened, subscribe and disable button
        ws.onopen = function () {
            ws.send("SUBUS_");
            message("Subscribed", false);
            document.getElementById("subscribe").disabled = true;
        }
        
        //When data from the server is received, log it on the text
        ws.onmessage = function (evt) { 
            var msg = evt.data;
            message(msg, true);
        };
        
        //Once the connection is closed, enable the subscribe button
        ws.onclose = function() { 
            message("Unsubscribed", false);
            document.getElementById("subscribe").disabled = false;
        };
    } else {
        subscribed = false;
        alert("WebSocket is not supported by your Browser!");
    }
})

//Ultrasonic and Servo message functions

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

//If it is a message from the server, show in the text box in
//blue text. If from this app, show in green text
function message(msg, server) {
    $("#messages").append("<p class='"+(server?"server":"client")+"'>"+msg+"</p>");
}