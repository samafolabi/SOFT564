function ajax(url, data, callback) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            callback(this.responseText);
        } else if (this.status == 404) {
            alert("ERROR");
        }
    }
    xhttp.open("POST", url, true);
    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
    xhttp.send(data);
}

$("#send").click(function () {
    ajax("http://192.168.43.21:9000","SUBDR_S_",function () {
        alert("done");
    })
})