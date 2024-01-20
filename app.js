import express from 'express';
import {WebSocket, WebSocketServer} from 'ws';
import http from 'http';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

import {toggleLight} from './apiFunctions.mjs'

wss.on('connection', (ws) => {
    console.log('Client connected');
    ws.on('message', (message) => {
        console.log(`Received: ${message}`);

        if(message == "hello") {
          ws.send(`Hi There`);
        } else if(message == "toggle light") {
          toggleLight((err, ledStatus) => {
            if (err) {
              console.error(`Error: ${err}`);
            } else {
              console.log(`Result: ${ledStatus}`);
              ws.send(`ledStatus: ${ledStatus}`);
            }
          });
        }
    });

    ws.on('close', () => {
        console.log('Client disconnected');
    });
});

server.listen(3000, () => {
    console.log('Server listening on port 3000');
});

/*const express = require('express');
const cors = require('cors');
const app = express();
const port = 8080;

const fs = require('fs');

// Use CORS middleware
app.use(cors());

app.get('/expectSuccess', (req, res) => {
  res.status(200).send('This API call was a success (as expected)');
});

app.get('/expectFail', (req, res) => {
  res.status(501).send('This API call was a fail (as expected)');
});

app.post('/toggleLight', (req, res) => {

  const filePath = './apiData.txt';
  let ledStatus;

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
  
    // Split the content into lines
    const lines = data.split('\n');

    let firstLine = lines[0];

    // Split the string by colon
    const parts = firstLine.split(':');

    // Get the text after the colon (index 1)
    const textAfterColon = parts[1].trim();

    if(textAfterColon == "0") {
      ledStatus = "1"
    } else {
      ledStatus = "0";
    }

    console.log(`testing ledStatus: ${ledStatus}`);
  
    // Change the first line (index 0)
    lines[0] = `ledStatus:${ledStatus}`;
  
    // Join the lines back into a string
    const updatedContent = lines.join('\n');
  
    // Write the updated content back to the file
    fs.writeFile(filePath, updatedContent, 'utf8', (err) => {
      if (err) {
        console.error(err);
        res.status(501).send(err);
      }

      res.status(200).send(`Light toggled: ${ledStatus}`);
    });
  });
});

app.get('/getLightVal', (req, res) => {

  const filePath = './apiData.txt';
  let ledStatus;

  fs.readFile(filePath, 'utf8', (err, data) => {
    if (err) {
      console.error(err);
      return;
    }
  
    // Split the content into lines
    const lines = data.split('\n');

    let firstLine = lines[0];

    // Split the string by colon
    const parts = firstLine.split(':');

    // Get the text after the colon (index 1)
    ledStatus = parts[1].trim();
    let returnString = 'ledStatus: ' + ledStatus;
    res.status(200).send(returnString);
  });
});

app.post('/moveUp', (req, res) => { 
    console.log('sending movement: Up'); 
    
    res.status(200).send(`U`);
  });

app.post('/moveDown', (req, res) => {
    console.log('sending movement: Down');
    
    res.status(200).send('D');
  });

app.post('/moveLeft', (req, res) => {
    console.log('sending movement: Left');
    
    res.status(200).send('D');
  });


app.post('/moveRight', (req, res) => {
    console.log('sending movement: Right');
    
    res.status(200).send('D');
  });

app.listen(port, () => {
  console.log(`Server listening at http://localhost:${port}`);
});
*/