import express from 'express';
import {WebSocket, WebSocketServer} from 'ws';
import http from 'http';

const app = express();
const server = http.createServer(app);
const wss = new WebSocketServer({ server });

import {toggleLight, getLightStatus} from './apiFunctions.mjs'

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
        } else if(message == "get light status") {
          getLightStatus((err, ledStatus) => {
            if (err) {
              console.error(`Error: ${err}`);
            } else {
              console.log(`${ledStatus}`);
              ws.send(ledStatus);
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