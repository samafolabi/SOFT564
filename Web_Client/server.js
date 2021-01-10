const fs = require('fs');
const express = require('express');
const bodyParser = require('body-parser');
const multer = require('multer');
const upload = multer();

const app = express();

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: true }));
app.use(upload.array()); 
app.use(express.static('public'));

function outputFile(file, res) {
    fs.readFile(file, function (err, data) {
        if (err) {
            res.writeHead(404, {'Content-Type': "text/html"});
        } else {
            res.writeHead(200, {'Content-Type': "text/html"});
            res.write(data.toString());
        }
    })
    res.end()
}

function errFunc(err, res) {
    res.writeHead(404, {'Content-Type': 'text/html'});
    res.end();
    throw err;
}

var port = 8000;
            //process.env.PORT || 443;
app.listen(port, function () {
    console.log("Server running at port "+port);
});
